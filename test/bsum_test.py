import os
import string
import shell
from hypothesis.database import ExampleDatabase
from hypothesis import given, settings
from hypothesis.strategies import text, lists, composite, integers
from test_util import run, clone_source

def setup_module(m):
    m.tempdir = clone_source()
    m.orig = os.getcwd()
    m.path = os.environ['PATH']
    os.chdir(m.tempdir)
    os.environ['PATH'] = f'{os.getcwd()}/bin:/usr/bin:/usr/local/bin:/sbin:/usr/sbin:/bin'
    shell.run('make clean && make bsv csv bschema bsum bcut', stream=True)

def teardown_module(m):
    os.chdir(m.orig)
    os.environ['PATH'] = m.path
    assert m.tempdir.startswith('/tmp/') or m.tempdir.startswith('/private/var/folders/')
    shell.run('rm -rf', m.tempdir)

@composite
def inputs(draw):
    num_columns = draw(integers(min_value=1, max_value=64))
    column = text(string.digits, min_size=1, max_size=16)
    line = lists(column, min_size=num_columns, max_size=num_columns)
    lines = draw(lists(line))
    csv = '\n'.join([','.join(x) for x in lines]) + '\n'
    return csv

def expected(csv):
    val = 0
    for line in csv.splitlines():
        col = line.split(',')[0]
        if col:
            val += int(col)
    return val

@given(inputs())
@settings(database=ExampleDatabase(':memory:'), max_examples=100 * int(os.environ.get('TEST_FACTOR', 1)), deadline=os.environ.get("TEST_DEADLINE", 1000 * 60)) # type: ignore
def test_props(args):
    csv = args
    result = expected(csv)
    assert result == int(run(csv, 'bsv | bschema a:i64,... | bsum i64 | bcut 1 | bschema i64:a | csv'))

def test1():
    stdin = """
    1
    1
    1
    """
    assert '3' == shell.run('bsv | bschema a:i64 | bsum i64 | bschema i64:a | csv', stdin=stdin)
