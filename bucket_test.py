import pytest
import murmur3 # https://github.com/nathants/murmur3/tree/master/python
import string
from hypothesis import given, settings
from hypothesis.strategies import text, lists, composite, integers
import shell # https://github.com/nathants/py-shell
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

shell.run('make clean && make bucket', stream=True)

MAX_LINE_BYTES = 8192

@composite
def inputs(draw):
    num_columns = draw(integers(min_value=1, max_value=12))
    column = text(string.ascii_lowercase, min_size=1)
    line = lists(column, min_size=num_columns, max_size=num_columns)
    lines = draw(lists(line, min_size=3))
    csv = '\n'.join([','.join(x) for x in lines]) + '\n'
    buckets = draw(integers(min_value=1, max_value=1e8 - 1))
    return (buckets, csv)

def expected(buckets, csv):
    xs = ["%d,%s" % (murmur3.hash(x.split(',')[0]) % buckets, x)
          if x.strip()
          else x
          for x in csv.splitlines()]

    return '\n'.join(xs) + '\n'

@given(inputs())
@settings(max_examples=100)
def test_props(args):
    buckets, csv = args
    result = expected(buckets, csv)
    assert result == run(csv, './bucket ,', buckets)

def test_single_column():
    stdin = """
    a
    1
    x
    """
    stdout = """
    2,a
    3,1
    3,x
    """
    assert unindent(stdout) == run(stdin, './bucket , 4')

def test_basic():
    stdin = """
    a,b,c,d
    1,2,3
    x,y
    """
    stdout = """
    2,a,b,c,d
    3,1,2,3
    3,x,y
    """
    assert unindent(stdout) == run(stdin, './bucket , 4')

def test_fails_when_non_positive_buckets():
    stdin = 'a'
    res = shell.run('./bucket , 0 2>&1', stdin=stdin, warn=True)
    assert res['exitcode'] == 1
    assert 'NUM_BUCKETS must be positive, got: 0' == res['output']

def test_fails_when_too_many_buckets():
    stdin = 'a'
    res = shell.run('./bucket ,', int(1e8), '2>&1', stdin=stdin, warn=True)
    assert res['exitcode'] == 1
    assert 'NUM_BUCKETS must be less than 1e8, got: 100000000' == res['output']

def test_fails_when_lines_too_long():
    stdin = 'a' * MAX_LINE_BYTES
    res = shell.run('./bucket , 4 2>&1', stdin=stdin, warn=True)
    assert res['exitcode'] == 1
    assert 'error: encountered a line longer than the max of 8192 chars' == res['output']
