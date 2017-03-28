import pytest
import os
import collections
import random
import string
from hypothesis import given, settings
from hypothesis.strategies import text, lists, composite, integers, tuples
import shell
import hashlib

tmp = os.environ.get('TMP_DIR', '/tmp').rstrip('/')
assert tmp != '/'
stdinpath = '%s/%s.stdin' % (tmp, hashlib.md5(__file__.encode('ascii')).hexdigest())
stdoutpath = '%s/%s.stdout' % (tmp, hashlib.md5(__file__.encode('ascii')).hexdigest())

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

shell.run('make clean && make partition', stream=True)

MAX_LINE_BYTES = 8192

prefix = '%s/tmp_partition.' % tmp

@composite
def inputs(draw):
    num_buckets = draw(integers(min_value=1, max_value=1000))
    num_columns = draw(integers(min_value=1, max_value=12))
    column = text(string.ascii_lowercase, min_size=1)
    columns = lists(column, min_size=1, max_size=num_columns)
    line = tuples(integers(min_value=0, max_value=num_buckets - 1), columns)
    lines = draw(lists(line, min_size=1))
    csv = '\n'.join([str(bucket) + ',' + ','.join(line) for bucket, line in lines]) + '\n'
    return (num_buckets, csv)

def expected(num_buckets, csv):
    res = collections.defaultdict(list)
    size = len(str(num_buckets))
    for line in csv.splitlines():
        bucket, line = line.split(',', 1)
        res[str(bucket).zfill(size)].append(line)
    val = ''
    for k in sorted(res):
        for line in res[k]:
            val += '%s%s:%s\n' % (prefix, k, line)
    return val.strip()

@given(inputs())
@settings(max_examples=100 * int(os.environ.get('TEST_FACTOR', 1)))
def test_props(args):
    num_buckets, csv = args
    result = expected(num_buckets, csv)
    print(result)
    shell.run('rm -f %s*' % prefix)
    try:
        run(csv, './partition', num_buckets, prefix)
        assert result == shell.run('grep --with-filename ".*" %s*' % prefix)
    finally:
        shell.run('rm -f %s*' % prefix)

def test_basic():
    shell.run('rm -f %s*' % prefix)
    try:
        stdin = """
        0,b,c,d
        1,e,f,g
        2,h,i,j
        """
        assert '' == run(stdin, './partition 10', prefix)
        stdout = """
        %(prefix)s00:b,c,d
        %(prefix)s01:e,f,g
        %(prefix)s02:h,i,j
        """ % {'prefix': prefix}
        assert unindent(stdout).strip() == shell.run('grep --with-filename ".*" %s*' % prefix)
        stdout = """
        %(prefix)s00
        %(prefix)s01
        %(prefix)s02
        """ % {'prefix': prefix}
        assert unindent(stdout).strip() == shell.run('ls %s*' % prefix)
    finally:
        shell.run('rm -f %s*' % prefix)

def test_appends():
    shell.run('rm -f %s*' % prefix)
    try:
        stdin = """
        0,b,c,d
        1,e,f,g
        2,h,i,j
        """
        assert '' == run(stdin, './partition 10', prefix)
        assert '' == run(stdin, './partition 10', prefix)
        stdout = """
        %(prefix)s00:b,c,d
        %(prefix)s00:b,c,d
        %(prefix)s01:e,f,g
        %(prefix)s01:e,f,g
        %(prefix)s02:h,i,j
        %(prefix)s02:h,i,j
        """ % {'prefix': prefix}
        assert unindent(stdout).strip() == shell.run('grep --with-filename ".*" %s*' % prefix)
        stdout = """
        %(prefix)s00
        %(prefix)s01
        %(prefix)s02
        """ % {'prefix': prefix}
        assert unindent(stdout).strip() == shell.run('ls %s*' % prefix)
    finally:
        shell.run('rm -f %s*' % prefix)
