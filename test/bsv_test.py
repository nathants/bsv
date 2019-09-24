import shell
import struct
import os
import string
import random
from hypothesis import given, settings
from hypothesis.strategies import text, lists, composite, integers, sampled_from, randoms
from test_util import run, rm_whitespace, rm_whitespace, compile_buffer_sizes

if os.environ.get('TEST_FACTOR'):
    buffers = list(sorted(set([5, 8, 11, 17, 64, 256, 1024] + [random.randint(8, 1024) for _ in range(10)])))
else:
    buffers = [5, 8, 11, 17, 64]

def setup_module():
    with shell.climb_git_root():
        shell.run('make clean', stream=True)
        compile_buffer_sizes('csv', buffers)
        compile_buffer_sizes('bsv', buffers)
        shell.run('make csv')
        shell.run('make bsv')

def teardown_module():
    with shell.climb_git_root():
        shell.run('make clean', stream=True)

def partition(r, n, x):
    res = []
    ks = list(sorted({r.randint(1, max(1, len(x))) for _ in range(n)}))
    ks = [0] + ks
    ks[-1] = len(x)
    for a, b in zip(ks, ks[1:]):
        res.append(x[a:b])
    return res

@composite
def inputs(draw):
    buffer = draw(sampled_from(buffers))
    r = draw(randoms())
    bytes_available = buffer - 2
    max_columns = bytes_available // 2
    num_columns = draw(integers(min_value=1, max_value=min(64, max_columns)))
    bytes_available -= num_columns * 2
    line = text(string.ascii_lowercase + ' ', min_size=0, max_size=bytes_available)
    line = line.map(lambda x: ','.join(partition(r, num_columns, x)))
    lines = draw(lists(line))
    lines = '\n'.join([l for l in lines if l]).strip() + '\n'
    return buffer, lines

@given(inputs())
@settings(max_examples=100 * int(os.environ.get('TEST_FACTOR', 1)), deadline=os.environ.get("TEST_DEADLINE", 1000 * 60))
def test_props(arg):
    buffer, csv = arg
    assert csv == run(csv, f'bin/bsv.{buffer} | bin/csv.{buffer}')

def test_example1():
    buffer, csv = 11, ',\n'
    val = run(csv, 'bin/bsv')
    val = bytes(val, 'utf-8')
    bsv = b''.join([
        struct.pack('i', 6), # uint32 num bytes in this chunk,
        struct.pack('H', 1), # uint16 max, see load.h
        struct.pack('H', 0), # uint16 sizes, see load.h
        struct.pack('H', 0), # uint16 sizes, see load.h
    ])
    assert bsv == val
    assert csv == run(csv, f'bin/bsv.{buffer} | bin/csv.{buffer}')

def test_max_bytes():
    stdin = 'a' * (2**16 - 1)
    assert len(stdin.strip()) == len(run(stdin, 'bin/bsv | bin/csv').strip())
    stdin = 'a' * (2**16)
    with shell.climb_git_root():
        res = shell.run('bin/bsv', stdin=stdin, warn=True)
    assert 'fatal: cannot have columns with more than 2**16 bytes, column: 0, size: 65536, content: aaaaaaaaaa...' == res['stderr']
    assert res['exitcode'] == 1

def test_encoding():
    stdin = """
    a
    """
    val = run(rm_whitespace(stdin), 'bin/bsv')
    val = bytes(val, 'utf-8')
    bsv = b''.join([
        # chunk header
        struct.pack('i', 5), # uint32 num bytes in this chunk,
        # chunk body
        struct.pack('H', 0), # uint16 max, see load.h
        struct.pack('H', 1), # uint16 sizes, see load.h
        b'a',
    ])
    assert bsv == val
    assert rm_whitespace(stdin) + '\n' == run(rm_whitespace(stdin), 'bin/bsv | bin/csv')

    stdin = """
    a,bb,ccc
    """
    val = run(rm_whitespace(stdin), 'bin/bsv')
    val = bytes(val, 'utf-8')
    bsv = b''.join([
        # chunk header
        struct.pack('i', 14), # uint32 num bytes in this chunk,
        # chunk body
        struct.pack('H', 2), # uint16 max, see load.h
        struct.pack('H', 1), # uint16 sizes, see load.h
        struct.pack('H', 2), # uint16 sizes, see load.h
        struct.pack('H', 3), # uint16 sizes, see load.h
        b'abbccc',
    ])
    assert bsv == val
    assert rm_whitespace(stdin) + '\n' == run(rm_whitespace(stdin), 'bin/bsv | bin/csv')

    stdin = """
    a
    """
    val = run(rm_whitespace(stdin), 'bin/bsv')
    val = bytes(val, 'utf-8')
    bsv = b''.join([
        # chunk header
        struct.pack('i', 5), # uint32 num bytes in this chunk,
        # chunk body
        struct.pack('H', 0), # uint16 max, see load.h
        struct.pack('H', 1), # uint16 sizes, see load.h
        b'a',
    ])
    assert bsv == val
    assert rm_whitespace(stdin) + '\n' == run(rm_whitespace(stdin), 'bin/bsv | bin/csv')

    stdin = '\n'
    assert rm_whitespace(stdin) + '\n' == run(rm_whitespace(stdin), 'bin/bsv | bin/csv')
