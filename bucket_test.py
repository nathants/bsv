import pytest
import string
from hypothesis import given, settings
from hypothesis.strategies import text, lists, composite, integers
import shell
import hashlib

stdinpath = '/tmp/%s.stdin' % hashlib.md5(__file__.encode('ascii')).hexdigest()
stdoutpath = '/tmp/%s.stdout' % hashlib.md5(__file__.encode('ascii')).hexdigest()

def unindent(text):
    return '\n'.join([x.lstrip() for x in text.splitlines()]) + '\n'

def run(stdin, *args):
    with open(stdinpath, 'w') as f:
        f.write(unindent(stdin))
    try:
        shell.run(*(('cat', stdinpath, '|') + args + ('>', stdoutpath)), stream=True)
    except:
        raise AssertionError from None
    with open(stdoutpath) as f:
        return f.read()

shell.run('make bucket', stream=True)

MAX_COLUMNS = 64
MAX_LINE_BYTES = 8192

# def test_basic():
#     stdin = """
#     a,b,c,d
#     1,2,3
#     x,y
#     """
#     stdout = """
#     2,a,b,c,d
#     3,1,2,3
#     3,x,y
#     """
#     assert unindent(stdout) == run(stdin, './bucket , 4')

def test_basic2():
    stdin = """
    a,b,c,d
    1
    x,y
    """
    stdout = """
    """
    assert unindent(stdout) == run(stdin, './bucket , 4')
