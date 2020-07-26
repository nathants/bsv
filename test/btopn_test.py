import pytest
import os
import string
import shell
from hypothesis.database import ExampleDatabase
from hypothesis import given, settings
from hypothesis.strategies import text, lists, composite, integers, integers
from test_util import run, rm_whitespace, clone_source

def setup_module(m):
    m.tempdir = clone_source()
    m.orig = os.getcwd()
    m.path = os.environ['PATH']
    os.chdir(m.tempdir)
    os.environ['PATH'] = f'{os.getcwd()}/bin:/usr/bin:/usr/local/bin:/sbin:/usr/sbin:/bin'
    shell.run('make clean && make bsv csv bcut btopn bschema', stream=True)

def teardown_module(m):
    os.chdir(m.orig)
    os.environ['PATH'] = m.path
    assert m.tempdir.startswith('/tmp/') or m.tempdir.startswith('/private/var/folders/')
    shell.run('rm -rf', m.tempdir)

@composite
def inputs(draw):
    n = draw(integers(min_value=1, max_value=16))
    num_columns = draw(integers(min_value=1, max_value=3))
    column = text(string.ascii_lowercase, min_size=1, max_size=64)
    line = lists(column, min_size=num_columns, max_size=num_columns)
    lines = draw(lists(line))
    lines = [','.join(map(str, line)) for line in lines]
    return n, '\n'.join(lines) + '\n'

def expected(n, csv):
    xs = csv.splitlines()
    xs = [x.split(',')[0] for x in xs if x]
    xs = sorted(xs, reverse=True)[:n]
    return '\n'.join(xs) + '\n'

@given(inputs())
@settings(database=ExampleDatabase(':memory:'), max_examples=100 * int(os.environ.get('TEST_FACTOR', 1)), deadline=os.environ.get("TEST_DEADLINE", 1000 * 60)) # type: ignore
def test_props(args):
    n, csv = args
    result = expected(n, csv)
    assert result == run(csv, f'bsv | btopn {n} | bcut 1 | csv ')
