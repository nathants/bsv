import os
import collections
import string
import shell
from hypothesis.database import ExampleDatabase
from hypothesis import given, settings, HealthCheck
from hypothesis.strategies import text, lists, composite, integers, randoms
from test_util import run, rm_whitespace, rm_whitespace, clone_source

def setup_module(m):
    m.tempdir = clone_source()
    m.orig = os.getcwd()
    m.path = os.environ['PATH']
    os.chdir(m.tempdir)
    os.environ['PATH'] = f'{os.getcwd()}/bin:/usr/bin:/usr/local/bin:/sbin:/usr/sbin:/bin'
    shell.run('make clean && make bsv csv bsort bschema bcounteach-hash', stream=True)

def teardown_module(m):
    os.chdir(m.orig)
    os.environ['PATH'] = m.path
    assert m.tempdir.startswith('/tmp/') or m.tempdir.startswith('/private/var/folders/')
    shell.run('rm -rf', m.tempdir)

@composite
def inputs(draw):
    random = draw(randoms())
    num_columns = draw(integers(min_value=1, max_value=4))
    max_repeats = draw(integers(min_value=1, max_value=3))
    column = text(string.ascii_lowercase, min_size=1, max_size=64)
    line = lists(column, min_size=num_columns, max_size=num_columns)
    lines = draw(lists(line))
    lines = [','.join(x) for x in lines]
    lines = [l
             for line in lines
             for l in [line] * (1 if random.random() > .5 else random.randint(1, max_repeats))]
    return '\n'.join(lines) + '\n'

def expected(csv):
    lines = csv.splitlines()
    lines = [x.split(',')[0] for x in lines]
    counts = collections.Counter(lines)
    return '\n'.join(f'{k},{counts[k]}' for k in sorted(counts) if k) + '\n'

@given(inputs())
@settings(database=ExampleDatabase(':memory:'), max_examples=100 * int(os.environ.get('TEST_FACTOR', 1)), suppress_health_check=HealthCheck.all()) # type: ignore
def test_props(args):
    csv = args
    result = expected(csv)
    assert result == run(csv, 'bsv | bcounteach-hash | bschema *,i64:a | bsort | csv')

def test_basic():
    stdin = """
    a
    a
    a
    b
    b
    a
    """
    stdout = """
    a,4
    b,2
    """
    assert rm_whitespace(stdout) + '\n' == run(rm_whitespace(stdin), 'bsv | bcounteach-hash | bschema *,i64:a | bsort | csv')
