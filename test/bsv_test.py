import shell
import random
import os
import hypothesis
import string
from hypothesis import given, settings
from hypothesis.strategies import text, lists, composite, integers, sampled_from
from test_util import run, rm_whitespace, rm_whitespace, compile_buffer_sizes

buffers = list(sorted(set([2, 3, 5, 8, 11, 17, 64, 256, 1024] + [random.randint(1, 1024) for _ in range(10)])))

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

@composite
def inputs(draw):
    buffer = draw(sampled_from(buffers))
    num_columns = draw(integers(min_value=1, max_value=64))
    column = text(string.ascii_lowercase + ' ', min_size=1, max_size=64)
    line = lists(column, min_size=1, max_size=num_columns)
    lines = lists(line)
    lines = [','.join(x)[:buffer - 2] for x in draw(lines)]
    lines = [l for l in lines if l.strip()]
    csv = '\n'.join(lines) + '\n'
    return buffer, csv

@given(inputs())
@settings(max_examples=100 * int(os.environ.get('TEST_FACTOR', 1)))
def test_props(arg):
    buffer, csv = arg
    assert csv == run(csv, f'bin/bsv.{buffer} | bin/csv.{buffer}')

def test_max_bytes():
    stdin = 'a' * (2**16 - 1)
    assert len(stdin.strip()) == len(run(stdin, 'bin/bsv | bin/csv').strip())
    stdin = 'a' * (2**16)
    with shell.climb_git_root():
        res = shell.run('bin/bsv', stdin=stdin, warn=True)
    assert 'error: cannot have columns with more than 2**16 bytes, column: 0,size: 65536, content: aaaaaaaaaa...' == res['stderr']
    assert res['exitcode'] == 1

def test_encoding():
    stdin = """
    aa,bbb
    xyz
    """
    assert rm_whitespace(stdin) + '\n' == run(rm_whitespace(stdin), 'bin/bsv | bin/csv')
    val = run(rm_whitespace(stdin), 'bin/bsv')
    val = bytes(val, 'utf-8')
    bsv = b''                                                           # ushort:max-index; ushort:size1, ...; bytes:chars
    bsv += b'\x01\x00' + b'\x02\x00' + b'\x03\x00' + b'aabbb'  # 1;2,3;aabbb
    bsv += b'\x00\x00' + b'\x03\x00'               + b'xyz'    # 0;3;xyz
    assert bsv == val
