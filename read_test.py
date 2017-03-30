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

shell.run('mv read.c read.c.bak')
shell.run('make clean')
for i in buffers:
    shell.run('cat read.c.bak | sed -r "s/#define READ_BUFFER_SIZE.*/#define READ_BUFFER_SIZE %s/" > read.c' % i)
    print(shell.run('cat read.c|grep define'))
    shell.run('make read')
    shell.run('mv read read.%s' % i, stream=True)
shell.run('mv -f read.c.bak read.c')

@composite
def inputs(draw):
    buffer = draw(sampled_from(buffers))
    line = text(string.ascii_lowercase + ' ', min_size=1, max_size=min(buffer - 1, 64))
    lines = lists(line)
    cmd = './read.%s' % buffer
    return cmd, '\n'.join(draw(lines)) + '\n'

@given(inputs())
@settings(max_examples=100 * int(os.environ.get('TEST_FACTOR', 1)))
def test_props(arg):
    cmd, txt = arg
    assert txt == run(txt, cmd)

def test_cycling1():
    stdin = """
    a
    b
    c
    d
    e
    f
    """
    assert rm_whitespace(stdin) == run(rm_whitespace(stdin), './read.8').strip()

def test_cycling2():
    stdin = """
    a
    b
    c
    d
    eee
    """
    assert rm_whitespace(stdin) == run(rm_whitespace(stdin), './read.8').strip()

def test_cycling3():
    stdin = """
    a
    b
    c
    d
    e
    fff
    """
    assert rm_whitespace(stdin) == run(rm_whitespace(stdin), './read.8').strip()

def test_cycling4():
    stdin = """
    a
    b
    c
    de
    """
    assert rm_whitespace(stdin) == run(rm_whitespace(stdin), './read.8').strip()

def test_cycling4():
    stdin = """
    a
    b
    c
    de
    """
    assert rm_whitespace(stdin) == run(rm_whitespace(stdin), './read.8').strip()

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
    assert stdout == run(stdin, './read.8')

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
    assert stdout == run(stdin, './read.8')

def test_whitespace3():
    stdin = ('\n'
             '\n'
             '\n'
             '\n')
    stdout = ('\n'
              '\n'
              '\n'
              '\n')
    assert stdout == run(stdin, './read.8')

def test_oversized_line():
    stdin = 'a' * 8
    res = shell.run('./read.8 2>&1', stdin=stdin, warn=True)
    assert res['exitcode'] == 1
    assert 'error: line longer than READ_BUFFER_SIZE' == res['output']
