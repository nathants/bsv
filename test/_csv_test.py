import os
import string
import shell
import hypothesis
from hypothesis import given, settings
from hypothesis.strategies import text, lists, composite, integers, sampled_from
from test_util import compile_buffer_sizes, run, rm_whitespace

os.chdir(os.path.dirname(os.path.abspath(__file__)))

buffers = [2, 3, 5, 8, 11, 17, 64, 256, 1024]

def setup_module():
    with shell.climb_git_root():
        shell.run('make clean', stream=True)
        compile_buffer_sizes('_csv', buffers)
        shell.run('make _csv')

def teardown_module():
    with shell.climb_git_root():
        shell.run('rm -f bin/_csv.*')

@composite
def inputs(draw):
    buffer = draw(sampled_from(buffers))
    num_columns = draw(integers(min_value=1, max_value=64))
    column = text(string.ascii_lowercase + ' ', min_size=1, max_size=64)
    line = lists(column, min_size=1, max_size=num_columns)
    lines = lists(line)
    csv = '\n'.join([','.join(x)[:buffer - 1] for x in draw(lines)]) + '\n'
    cmd = 'bin/_csv.%s' % buffer
    return cmd, csv

def expected(csv):
    res = []
    for line in csv.splitlines():
        for col in line.split(','):
            res.append(col)
    return '\n'.join(res) + '\n'

@given(inputs())
@settings(max_examples=100 * int(os.environ.get('TEST_FACTOR', 1)), timeout=hypothesis.unlimited, suppress_health_check=[hypothesis.HealthCheck.hung_test])
def test_props(arg):
    cmd, csv = arg
    result = expected(csv)
    assert result == run(csv, cmd)

def test_escapes():
    stdin = """
    a,b,c\,d\,e\n
    f,g\,h\\n\,i\n
    """
    stdout = """
    a
    b
    c\,d\,e
    f
    g\,h\\n\,i
    """
    assert rm_whitespace(stdout) == run(rm_whitespace(stdin), 'bin/_csv.1024').strip()

def test_cycling1():
    stdin = """
    a
    b
    c
    d
    e
    f
    """
    assert rm_whitespace(stdin) == run(rm_whitespace(stdin), 'bin/_csv.8').strip()

def test_cycling2():
    stdin = """
    a
    b
    c
    d
    eee
    """
    assert rm_whitespace(stdin) == run(rm_whitespace(stdin), 'bin/_csv.8').strip()

def test_cycling3():
    stdin = """
    a
    b
    c
    d
    e
    fff
    """
    assert rm_whitespace(stdin) == run(rm_whitespace(stdin), 'bin/_csv.8').strip()

def test_cycling4():
    stdin = """
    a
    b
    c
    de
    """
    assert rm_whitespace(stdin) == run(rm_whitespace(stdin), 'bin/_csv.8').strip()

def test_cycling4():
    stdin = """
    a
    b
    c
    de
    """
    assert rm_whitespace(stdin) == run(rm_whitespace(stdin), 'bin/_csv.8').strip()

def test_holes():
    stdin = 'a,,c\n'
    stdout = 'a\n\nc\n'
    assert stdout == run(stdin, 'bin/_csv.8')

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
    assert rm_whitespace(stdout) + '\n' == run(rm_whitespace(stdin), 'bin/_csv.8')

def test_empties():
    stdin = 'a,,,b,c'
    stdout = 'a\n\n\nb\nc\n'
    assert stdout == run(stdin, 'bin/_csv.8')

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
    assert stdout == run(stdin, 'bin/_csv.8')


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
    assert stdout == run(stdin, 'bin/_csv.8')

def test_whitespace3():
    stdin = ('\n'
             '\n'
             '\n'
             '\n')
    stdout = ('\n'
              '\n'
              '\n'
              '\n')
    assert stdout == run(stdin, 'bin/_csv.8')

def test_fails_when_too_many_columns():
    with shell.climb_git_root():
        stdin = 'a,' * (2**16 - 1)
        with shell.tempdir(cleanup=False):
            with open('input', 'w') as f:
                f.write(stdin)
            path = os.path.abspath('input')
        try:
            res = shell.run('set -o pipefail; cat', path, '| bin/_csv >/dev/null', warn=True)
        finally:
            shell.run('rm', path)
        assert res['exitcode'] == 1
        assert 'error: line with more than 65535 columns' == res['stderr']

def test_oversized_line():
    with shell.climb_git_root():
        stdin = 'a' * 8
        res = shell.run('bin/_csv.8', stdin=stdin, warn=True)
        assert res['exitcode'] == 1
        assert 'error: line longer than BUFFER_SIZE' == res['stderr']
