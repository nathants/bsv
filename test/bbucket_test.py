import os
import string
import shell
from hypothesis.database import ExampleDatabase
from hypothesis import given, settings
from hypothesis.strategies import text, lists, composite, integers
from test_util import run, rm_whitespace, rm_whitespace, rm_whitespace, clone_source

def setup_module(m):
    m.tempdir = clone_source()
    m.orig = os.getcwd()
    m.path = os.environ['PATH']
    os.chdir(m.tempdir)
    os.environ['PATH'] = f'{os.getcwd()}/bin:/usr/bin:/usr/local/bin'
    shell.run('make clean && make bsv csv bbucket xxh3', stream=True)

def teardown_module(m):
    os.chdir(m.orig)
    os.environ['PATH'] = m.path
    assert m.tempdir.startswith('/tmp/')
    shell.run('rm -rf', m.tempdir)

@composite
def inputs(draw):
    num_columns = draw(integers(min_value=1, max_value=64))
    column = text(string.ascii_lowercase, min_size=1, max_size=64)
    line = lists(column, min_size=num_columns, max_size=num_columns)
    lines = draw(lists(line, min_size=3))
    csv = '\n'.join([','.join(x) for x in lines]) + '\n'
    buckets = draw(integers(min_value=1, max_value=1e5))
    return buckets, csv

def xxh3_hash(x):
    return int(shell.run('xxh3 --int', stdin=x))

def expected(buckets, csv):
    xs = ["%d,%s" % (xxh3_hash(x.split(',')[0]) % buckets, x)
          if x.strip()
          else x
          for x in csv.splitlines()]
    return '\n'.join(xs) + '\n'

@given(inputs())
@settings(database=ExampleDatabase(':memory:'), max_examples=100 * int(os.environ.get('TEST_FACTOR', 1)), deadline=os.environ.get("TEST_DEADLINE", 1000 * 60))
def test_props(args):
    buckets, csv = args
    result = expected(buckets, csv)
    assert result == run(csv, 'bsv | bbucket', buckets, '| bin/csv')

def test_single_column():
    stdin = """
    a
    y
    x
    """
    stdout = """
    3,a
    3,y
    2,x
    """
    assert rm_whitespace(stdout) + '\n' == run(rm_whitespace(stdin), 'bsv | bbucket 4 | bin/csv')

def test_basic():
    stdin = """
    a,b,c,d
    e,f,g
    x,y
    """
    stdout = """
    3,a,b,c,d
    3,e,f,g
    2,x,y
    """
    assert rm_whitespace(stdout) + '\n' == run(rm_whitespace(stdin), 'bsv | bbucket 4 | bin/csv')

def test_fails_when_non_positive_buckets():
    with shell.climb_git_root():
        stdin = 'a'
        print(shell.run('pwd'))
        res = shell.run('bsv | bbucket 0', stdin=stdin, warn=True)
        assert 'NUM_BUCKETS must be positive, got: 0' == res['stderr']
        assert res['exitcode'] == 1

def test_fails_when_too_many_buckets():
    with shell.climb_git_root():
        stdin = 'a'
        res = shell.run('bsv | bbucket', int(1e8), stdin=stdin, warn=True)
        assert res['exitcode'] == 1
        assert 'NUM_BUCKETS must be less than 1e8, got: 100000000' == res['stderr']
