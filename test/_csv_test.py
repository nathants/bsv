import os
import random
import string
import shell
from hypothesis import given, settings
from hypothesis.strategies import text, lists, composite, integers, sampled_from
from test_util import compile_buffer_sizes, run, rm_whitespace, clone_source

if os.environ.get('TEST_FACTOR'):
    buffers = list(sorted(set([5, 8, 11, 17, 64, 128, 256, 1024, 1024 * 1024 * 5] + [random.randint(8, 1024) for _ in range(10)])))
else:
    buffers = [8, 128]

def setup_module(m):
    m.tempdir = clone_source()
    m.orig = os.getcwd()
    m.path = os.environ['PATH']
    os.chdir(m.tempdir)
    os.environ['PATH'] = f'{os.getcwd()}/bin:/usr/bin:/usr/local/bin:/sbin:/usr/sbin:/bin'
    shell.run('make clean', stream=True)
    compile_buffer_sizes('_csv', buffers)
    shell.run('make _csv')

def teardown_module():
    with shell.climb_git_root():
        shell.run('make clean', stream=True)

@composite
def inputs(draw):
    buffer = draw(sampled_from(buffers))
    num_columns = draw(integers(min_value=1, max_value=64))
    column = text(string.ascii_lowercase + ' ', min_size=1, max_size=64)
    line = lists(column, min_size=1, max_size=num_columns)
    line = line.filter(lambda x: len(' '.join(x)) + len(x) * 2 + 1 < buffer)
    lines = lists(line)
    csv = '\n'.join([','.join(x) for x in draw(lines)]) + '\n'
    cmd = '_csv.%s' % buffer
    return cmd, csv

def expected(csv):
    res = []
    for line in csv.splitlines():
        for col in line.split(','):
            res.append(col)
    return '\n'.join(res) + '\n'

@given(inputs())
@settings(max_examples=100 * int(os.environ.get('TEST_FACTOR', 1)), deadline=os.environ.get("TEST_DEADLINE", 1000 * 60))
def test_props(arg):
    cmd, csv = arg
    result = expected(csv)
    assert result == run(csv, cmd)

def test_cycling1():
    stdin = """
    a
    b
    c
    d
    e
    f
    """
    assert rm_whitespace(stdin) == run(rm_whitespace(stdin), '_csv.8').strip()

def test_cycling2():
    stdin = """
    a
    b
    c
    d
    eee
    """
    assert rm_whitespace(stdin) == run(rm_whitespace(stdin), '_csv.8').strip()

def test_cycling3():
    stdin = """
    a
    b
    c
    d
    e
    fff
    """
    assert rm_whitespace(stdin) == run(rm_whitespace(stdin), '_csv.8').strip()

def test_cycling4():
    stdin = """
    a
    b
    c
    de
    """
    assert rm_whitespace(stdin) == run(rm_whitespace(stdin), '_csv.8').strip()

def test_holes():
    stdin = 'a,,c\n'
    stdout = 'a\n\nc\n'
    assert stdout == run(stdin, '_csv.8')

def test_basic():
    stdin = """
    a,b
    cd,e
    """
    stdout = """
    a
    b
    cd
    e
    """
    assert rm_whitespace(stdout) + '\n' == run(rm_whitespace(stdin), '_csv.8')

def test_empties():
    stdin = 'a,,,b,c'
    stdout = 'a\n\n\nb\nc\n'
    assert stdout == run(stdin, '_csv.8')

def test_whitespace1():
    stdin = ('a\n'
             '\n'
             'b\n'
             '\n'
             'c\n')
    stdout = ('a\n'
              '\n'
              'b\n'
              '\n'
              'c\n')
    assert stdout == run(stdin, '_csv.8')


def test_whitespace2():
    stdin = ('\n'
             'a\n'
             '\n'
             'b\n'
             '\n'
             'c\n'
             '\n')
    stdout = ('\n'
              'a\n'
              '\n'
              'b\n'
              '\n'
              'c\n'
              '\n')
    assert stdout == run(stdin, '_csv.8')

def test_whitespace3():
    stdin = ('\n'
             '\n'
             '\n'
             '\n')
    stdout = ('\n'
              '\n'
              '\n'
              '\n')
    assert stdout == run(stdin, '_csv.8')

def test_fails_when_too_many_columns():
    with shell.climb_git_root():
        stdin = 'a,' * (2**16 - 1)
        with shell.tempdir(cleanup=False):
            with open('input', 'w') as f:
                f.write(stdin)
            path = os.path.abspath('input')
        try:
            res = shell.run('cat', path, '| _csv >/dev/null', warn=True)
        finally:
            shell.run('rm', path)
        assert res['exitcode'] == 1
        assert 'fatal: line with more than 65535 columns' == res['stderr']

def test_oversized_line():
    with shell.climb_git_root():
        stdin = 'a' * 8
        res = shell.run('_csv.8', stdin=stdin, warn=True)
        assert res['exitcode'] == 1
        assert 'fatal: line longer than BUFFER_SIZE' == res['stderr']
