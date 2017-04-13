import pytest
import os
import murmur3 # https://github.com/nathants/murmur3/tree/master/python
import string
from hypothesis import given, settings
from hypothesis.strategies import text, lists, composite, integers
import shell # https://github.com/nathants/py-shell
import hashlib

tmp = os.environ.get('TMP_DIR', '/tmp').rstrip('/')
stdinpath = '%s/%s.stdin' % (tmp, hashlib.md5(__file__.encode('ascii')).hexdigest())
stdoutpath = '%s/%s.stdout' % (tmp, hashlib.md5(__file__.encode('ascii')).hexdigest())

def rm_whitespace(x):
    return '\n'.join([y.strip().replace(' ', '')
                      for y in x.splitlines()
                      if y.strip()]) + '\n'

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

shell.run('make clean && make take', stream=True)

MAX_LINE_BYTES = 8192

# @composite
# def inputs(draw):
#     num_columns = draw(integers(min_value=1, max_value=12))
#     column = text(string.ascii_lowercase, min_size=1)
#     line = lists(column, min_size=num_columns, max_size=num_columns)
#     lines = draw(lists(line, min_size=3))
#     csv = '\n'.join([','.join(x) for x in lines]) + '\n'
#     buckets = draw(integers(min_value=1, max_value=1e5))
#     return (buckets, csv)

# def expected(buckets, csv):
#     xs = ["%d,%s" % (murmur3.hash(x.split(',')[0]) % buckets, x)
#           if x.strip()
#           else x
#           for x in csv.splitlines()]

#     return '\n'.join(xs) + '\n'

# @given(inputs())
# @settings(max_examples=100 * int(os.environ.get('TEST_FACTOR', 1)))
# def test_props(args):
#     buckets, csv = args
#     result = expected(buckets, csv)
#     assert result == run(csv, './bucket', buckets)

def test_basic():
    stdin = """
    a,1
    a,2
    b,1
    b,2
    c,1
    c,2
    """
    stdout = """
    b,1
    b,2
    """
    assert rm_whitespace(stdout) == run(stdin, './take b')
