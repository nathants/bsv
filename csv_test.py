import pytest
import os
import string
from hypothesis import given, settings
from hypothesis.strategies import text, characters, lists, composite, integers, sampled_from
import shell
import hashlib

tmp = os.environ.get('TMP_DIR', '/tmp').rstrip('/')
stdinpath = '%s/%s.stdin' % (tmp, hashlib.md5(__file__.encode('ascii')).hexdigest())
stdoutpath = '%s/%s.stdout' % (tmp, hashlib.md5(__file__.encode('ascii')).hexdigest())

MAX_COLUMNS = 64

def rm_whitespace(x):
    return '\n'.join([y.strip().replace(' ', '')
                      for y in x.splitlines()
                      if y.strip()])

def run(stdin, *args):
    with open(stdinpath, 'w') as f:
        f.write(stdin)
    try:
        shell.run(*(('cat', stdinpath, '|') + args + ('>', stdoutpath, '2>/dev/null')))
    except:
        raise AssertionError from None
    with open(stdoutpath) as f:
        return f.read()

buffers = [8, 11, 17, 64, 256, 1024]

shell.run('mv csv.c csv.c.bak')
shell.run('make clean')
for i in buffers:
    shell.run('cat csv.c.bak | sed -r "s/#define CSV_BUFFER_SIZE.*/#define CSV_BUFFER_SIZE %s/" > csv.c' % i)
    print(shell.run('cat csv.c|grep define'))
    shell.run('make csv')
    shell.run('mv csv csv.%s' % i, stream=True)
shell.run('mv -f csv.c.bak csv.c')

@composite
def inputs(draw):
    buffer = draw(sampled_from(buffers))
    num_columns = draw(integers(min_value=1, max_value=64))
    column = text(string.ascii_lowercase + ' ', min_size=1, max_size=64, average_size=16)
    line = lists(column, min_size=1, max_size=num_columns)
    lines = lists(line)
    csv = '\n'.join([','.join(x)[:buffer - 1] for x in draw(lines)]) + '\n'
    cmd = './csv.%s' % buffer
    return cmd, csv

def expected(csv):
    res = []
    for line in csv.splitlines():
        for col in line.split(','):
            res.append(col)
    return '\n'.join(res) + '\n'

@given(inputs())
@settings(max_examples=100 * int(os.environ.get('TEST_FACTOR', 1)))
def test_props(arg):
    cmd, csv = arg
    result = expected(csv)
    assert result == run(csv, cmd)

@pytest.mark.only
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
    assert rm_whitespace(stdout) == run(rm_whitespace(stdin), './csv.1024').strip()

def test_cycling1():
    stdin = """
    a
    b
    c
    d
    e
    f
    """
    assert rm_whitespace(stdin) == run(rm_whitespace(stdin), './csv.8').strip()

def test_cycling2():
    stdin = """
    a
    b
    c
    d
    eee
    """
    assert rm_whitespace(stdin) == run(rm_whitespace(stdin), './csv.8').strip()

def test_cycling3():
    stdin = """
    a
    b
    c
    d
    e
    fff
    """
    assert rm_whitespace(stdin) == run(rm_whitespace(stdin), './csv.8').strip()

def test_cycling4():
    stdin = """
    a
    b
    c
    de
    """
    assert rm_whitespace(stdin) == run(rm_whitespace(stdin), './csv.8').strip()

def test_cycling4():
    stdin = """
    a
    b
    c
    de
    """
    assert rm_whitespace(stdin) == run(rm_whitespace(stdin), './csv.8').strip()

def test_holes():
    stdin = 'a,,c\n'
    stdout = 'a\n\nc\n'
    assert stdout == run(stdin, './csv.8')

def test1():
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
    assert rm_whitespace(stdout) == run(rm_whitespace(stdin), './csv.8').strip()

def test_empties():
    stdin = 'a,,,b,c'
    stdout = 'a\n\n\nb\nc\n'
    assert stdout == run(stdin, './csv.8')

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
    assert stdout == run(stdin, './csv.8')


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
    assert stdout == run(stdin, './csv.8')

def test_whitespace3():
    stdin = ('\n'
             '\n'
             '\n'
             '\n')
    stdout = ('\n'
              '\n'
              '\n'
              '\n')
    assert stdout == run(stdin, './csv.8')

def test_fails_when_too_many_columns():
    stdin = 'a,' * MAX_COLUMNS
    res = shell.run('./csv.1024 2>&1', stdin=stdin, warn=True, stream=True)
    assert res['exitcode'] == 1
    assert 'error: line with more than 64 columns' == res['output']

def test_oversized_line():
    stdin = 'a' * 8
    res = shell.run('./csv.8 2>&1', stdin=stdin, warn=True)
    assert res['exitcode'] == 1
    assert 'error: line longer than CSV_BUFFER_SIZE' == res['output']
