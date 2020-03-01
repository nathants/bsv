import pytest
import os
import string
import shell
from hypothesis.database import ExampleDatabase
from hypothesis import given, settings
from hypothesis.strategies import text, lists, composite, integers, randoms
from test_util import run, rm_whitespace, clone_source

def setup_module(m):
    m.tempdir = clone_source()
    m.orig = os.getcwd()
    m.path = os.environ['PATH']
    os.chdir(m.tempdir)
    os.environ['PATH'] = f'{os.getcwd()}/bin:/usr/bin:/usr/local/bin:/sbin:/usr/sbin:/bin'
    shell.run('make clean && make bsv csv bcut bsort', stream=True)

def teardown_module(m):
    os.chdir(m.orig)
    os.environ['PATH'] = m.path
    assert m.tempdir.startswith('/tmp/') or m.tempdir.startswith('/private/var/folders/')
    shell.run('rm -rf', m.tempdir)

@composite
def inputs(draw):
    num_columns = draw(integers(min_value=1, max_value=64))
    column = text(string.ascii_lowercase + ':/|', min_size=1, max_size=64)
    line = lists(column, min_size=num_columns, max_size=num_columns)
    lines = draw(lists(line))
    csv = '\n'.join([','.join(x) for x in lines]) + '\n'
    return csv

def expected(csv):
    xs = csv.splitlines()
    xs = [x.split(',')[0] for x in xs]
    xs = sorted(xs)
    return '\n'.join(xs) + '\n'

@given(inputs())
@settings(database=ExampleDatabase(':memory:'), max_examples=100 * int(os.environ.get('TEST_FACTOR', 1)), deadline=os.environ.get("TEST_DEADLINE", 1000 * 60))
def test_props(csv):
    result = expected(csv)
    if result:
        assert result == run(csv, f'bsv | bsort | bcut 1 | bin/csv')
    else:
        with pytest.raises(AssertionError):
            run(csv, f'bsv | bsort | bin/csv')

@given(inputs())
@settings(database=ExampleDatabase(':memory:'), max_examples=100 * int(os.environ.get('TEST_FACTOR', 1)), deadline=os.environ.get("TEST_DEADLINE", 1000 * 60))
def test_props_compatability(csv):
    assert run(csv, f'LC_ALL=C sort -k1,1 | cut -d, -f1') == run(csv, f'bsv | bsort | bcut 1 | bin/csv')

def test_basic2():
    stdin = """
    a,b
    aa,a
    """
    stdout = """
    a,b
    aa,a
    """
    assert rm_whitespace(stdout) + '\n' == run(rm_whitespace(stdin), 'bsv | bsort | bin/csv')

def test_basic():
    stdin = """
    aa
    a
    """
    stdout = """
    a
    aa
    """
    assert rm_whitespace(stdout) + '\n' == run(rm_whitespace(stdin), 'bsv | bsort | bin/csv')

def test_compatability():
    stdin = """
    c
    b
    a
    """
    stdout = """
    a
    b
    c
    """
    assert rm_whitespace(stdout) + '\n' == run(rm_whitespace(stdin), 'bsv | bsort | bin/csv')

def test_compatability2():
    stdin = """
    c,c
    b,b
    a,a
    """
    stdout = """
    a,a
    b,b
    c,c
    """
    assert rm_whitespace(stdout) + '\n' == run(rm_whitespace(stdin), 'bsv | bsort | bin/csv')
