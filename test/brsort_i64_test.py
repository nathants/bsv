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
    shell.run('make clean && make bsv csv bcut brsort-i64 bschema', stream=True)

def teardown_module(m):
    os.chdir(m.orig)
    os.environ['PATH'] = m.path
    assert m.tempdir.startswith('/tmp/') or m.tempdir.startswith('/private/var/folders/')
    shell.run('rm -rf', m.tempdir)

@composite
def inputs(draw):
    num_columns = draw(integers(min_value=1, max_value=3))
    column = integers(min_value=-9223372036854775807, max_value=9223372036854775807)
    line = lists(column, min_size=num_columns, max_size=num_columns)
    lines = draw(lists(line))
    lines = [','.join(map(str, line)) for line in lines]
    return '\n'.join(lines) + '\n'

def expected(csv):
    xs = csv.splitlines()
    xs = [int(x.split(',')[0]) for x in xs if x]
    xs = sorted(xs, reverse=True)
    return '\n'.join(map(str, xs)) + '\n'

import pytest
@pytest.mark.only
@given(inputs())
@settings(database=ExampleDatabase(':memory:'), max_examples=100 * int(os.environ.get('TEST_FACTOR', 1)), deadline=os.environ.get("TEST_DEADLINE", 1000 * 60)) # type: ignore
def test_props(csv):
    result = expected(csv)
    assert result == run(csv, 'bsv | bschema a:i64,... | brsort-i64 | bcut 1 | bschema i64:a | csv')
