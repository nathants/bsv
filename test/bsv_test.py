import bsv
import io
import shell
import struct
import os
import string
import random
from hypothesis.database import ExampleDatabase
from hypothesis import given, settings
from hypothesis.strategies import text, lists, composite, integers, sampled_from, randoms
from test_util import runb, run, rm_whitespace, rm_whitespace, compile_buffer_sizes, clone_source

if os.environ.get('TEST_FACTOR'):
    buffers = list(sorted(set([12, 17, 64, 256, 1024] + [random.randint(8, 1024) for _ in range(10)])))
else:
    buffers = [12, 17, 64]

def setup_module(m):
    m.tempdir = clone_source()
    m.orig = os.getcwd()
    m.path = os.environ['PATH']
    os.chdir(m.tempdir)
    os.environ['PATH'] = f'{os.getcwd()}/bin:/usr/bin:/usr/local/bin:/sbin:/usr/sbin:/bin'
    shell.run('make clean', stream=True)
    compile_buffer_sizes('csv', buffers)
    compile_buffer_sizes('bsv', buffers)
    shell.run('make csv')
    shell.run('make bsv')

def teardown_module(m):
    os.chdir(m.orig)
    os.environ['PATH'] = m.path
    assert m.tempdir.startswith('/tmp/') or m.tempdir.startswith('/private/var/folders/')
    shell.run('rm -rf', m.tempdir)

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
    bytes = buffer - 5
    cols = bytes // (2 + 1 + 4)
    r = draw(randoms())
    num_columns = draw(integers(min_value=1, max_value=64))
    line = text(string.ascii_lowercase, min_size=0, max_size=1024)
    line = line.map(lambda x: ','.join(partition(r, num_columns, x))[:cols])
    lines = draw(lists(line))
    lines = '\n'.join([l for l in lines if l]).strip() + '\n'
    return buffer, lines

def expected(text):
    return '\n'.join(','.join('%d' % int(y) if y.isdigit() else y for y in x.split(',')) for x in text.splitlines())

@given(inputs())
@settings(database=ExampleDatabase(':memory:'), max_examples=100 * int(os.environ.get('TEST_FACTOR', 1)), deadline=os.environ.get("TEST_DEADLINE", 1000 * 60))
def test_props(arg):
    buffers, csv = arg
    assert expected(csv) + '\n' == run(csv, f'bsv.{buffers} | csv.{buffers}')

@given(inputs())
@settings(database=ExampleDatabase(':memory:'), max_examples=100 * int(os.environ.get('TEST_FACTOR', 1)), deadline=os.environ.get("TEST_DEADLINE", 1000 * 60))
def test_props_python_write(arg):
    buffers, csv = arg
    csv[:1024 * 1024 * 5] # slice to buffer size which is max python write supports
    bytes_io = io.BytesIO()
    data = [row.split(b',') for row in csv.encode('utf-8').split(b'\n')]
    bsv.dump(bytes_io, data)
    assert expected(csv) == runb(bytes_io.getvalue(), 'csv').decode('utf-8').rstrip()

@given(inputs())
@settings(database=ExampleDatabase(':memory:'), max_examples=100 * int(os.environ.get('TEST_FACTOR', 1)), deadline=os.environ.get("TEST_DEADLINE", 1000 * 60))
def test_props_python_read(arg):
    buffers, csv = arg
    assert expected(csv) + '\n' == '\n'.join(','.join(v.decode('utf-8')
                                                      if isinstance(v, bytes)
                                                      else str(v)
                                                      for v in row)
                                             for row in bsv.load(io.BytesIO(runb(csv, f'bsv.{buffers}')))) + '\n'

def test_example1():
    csv = ',\n'
    val = runb(csv, 'bsv')
    bsv = b''.join([
        struct.pack('i', 8), # uint32 num bytes in this chunk,
        struct.pack('H', 1), # uint16 max, see load.h
        struct.pack('B', 0), # uint8 types, see load.h
        struct.pack('B', 0), # uint8 types, see load.h
        struct.pack('H', 0), # uint16 sizes, see load.h
        struct.pack('H', 0), # uint16 sizes, see load.h
    ])
    assert bsv == val
    assert csv == run(csv, f'bsv | csv')

def test_numeric_first_column_is_illegal():
    res = shell.run('echo 1 | bsv', warn=True)
    assert 'warn: first column value is numeric, which will sort incorrectly. first column is the sort key, and is interpreted as bytes' == res['stderr']
    assert res['exitcode'] == 0

def test_max_bytes():
    stdin = 'a' * (2**16 - 1)
    assert len(stdin.strip()) == len(run(stdin, 'bsv | csv').strip())
    stdin = 'a' * (2**16)
    with shell.climb_git_root():
        res = shell.run('bsv', stdin=stdin, warn=True)
    assert 'fatal: cannot have columns with more than 2**16 bytes, column: 0, size: 65536, content: aaaaaaaaaa...' == res['stderr']
    assert res['exitcode'] == 1

def test_encoding():
    stdin = """
    a
    """
    val = runb(rm_whitespace(stdin), 'bsv')
    bsv = b''.join([
        # chunk header
        struct.pack('i', 6), # uint32 num bytes in this chunk,
        # chunk body
        struct.pack('H', 0), # uint16 max, see load.h
        struct.pack('B', 0), # uint8 types, see load.h
        struct.pack('H', 1), # uint16 sizes, see load.h
        b'a',
    ])
    assert bsv == val
    assert rm_whitespace(stdin) + '\n' == run(rm_whitespace(stdin), 'bsv | csv')

    stdin = """
    a,bb,ccc
    """
    val = runb(rm_whitespace(stdin), 'bsv')
    bsv = b''.join([
        # chunk header
        struct.pack('i', 17), # uint32 num bytes in this chunk,
        # chunk body
        struct.pack('H', 2), # uint16 max, see load.h
        struct.pack('B', 0), # uint8 types, see load.h
        struct.pack('B', 0), # uint8 types, see load.h
        struct.pack('B', 0), # uint8 types, see load.h
        struct.pack('H', 1), # uint16 sizes, see load.h
        struct.pack('H', 2), # uint16 sizes, see load.h
        struct.pack('H', 3), # uint16 sizes, see load.h
        b'abbccc',
    ])
    assert bsv == val
    assert rm_whitespace(stdin) + '\n' == run(rm_whitespace(stdin), 'bsv | csv')

    stdin = """
    a,12,1.500000
    """
    val = runb(rm_whitespace(stdin), 'bsv')
    bsv = b''.join([
        # chunk header
        struct.pack('i', 28), # uint32 num bytes in this chunk,
        # chunk body
        struct.pack('H', 2), # uint16 max, see load.h
        struct.pack('B', 0), # uint8 types, see load.h
        struct.pack('B', 1), # uint8 types, see load.h
        struct.pack('B', 2), # uint8 types, see load.h
        struct.pack('H', 1), # uint16 sizes, see load.h
        struct.pack('H', 8), # uint16 sizes, see load.h
        struct.pack('H', 8), # uint16 sizes, see load.h
        b'a', struct.pack('q', 12), struct.pack('d', 1.5),
    ])
    assert bsv == val
    assert rm_whitespace(stdin) + '\n' == run(rm_whitespace(stdin), 'bsv | csv')

    stdin = """
    a
    """
    val = run(rm_whitespace(stdin), 'bsv')
    val = bytes(val, 'utf-8')
    bsv = b''.join([
        # chunk header
        struct.pack('i', 6), # uint32 num bytes in this chunk,
        # chunk body
        struct.pack('H', 0), # uint16 max, see load.h
        struct.pack('B', 0), # uint8 types, see load.h
        struct.pack('H', 1), # uint16 sizes, see load.h
        b'a',
    ])
    assert bsv == val
    assert rm_whitespace(stdin) + '\n' == run(rm_whitespace(stdin), 'bsv | csv')

    stdin = '\n'
    assert rm_whitespace(stdin) + '\n' == run(rm_whitespace(stdin), 'bsv | csv')
