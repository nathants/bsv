import os
import string
import shell
import collections
from hypothesis.database import ExampleDatabase
from hypothesis import given, settings
from hypothesis.strategies import text, lists, composite, integers, tuples
from test_util import unindent, rm_whitespace, clone_source

def setup_module(m):
    m.tempdir = clone_source()
    m.orig = os.getcwd()
    m.path = os.environ['PATH']
    os.chdir(m.tempdir)
    os.environ['PATH'] = f'{os.getcwd()}/bin:/usr/bin:/usr/local/bin:/sbin:/usr/sbin:/bin'
    shell.run('make clean && make bsv csv bschema bcat bpartition', stream=True)

def teardown_module(m):
    os.chdir(m.orig)
    os.environ['PATH'] = m.path
    assert m.tempdir.startswith('/tmp/') or m.tempdir.startswith('/private/var/folders/')
    shell.run('rm -rf', m.tempdir)

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
            val += f'prefix{k}:{line}\n'
    return val.strip()

@given(inputs())
@settings(database=ExampleDatabase(':memory:'), max_examples=100 * int(os.environ.get('TEST_FACTOR', 1)), deadline=os.environ.get("TEST_DEADLINE", 1000 * 60)) # type: ignore
def test_props(args):
    num_buckets, csv = args
    result = expected(num_buckets, csv)
    with shell.tempdir():
        stdout = '\n'.join(sorted({l.split(':')[0] for l in result.splitlines()}))
        assert stdout == shell.run(f'bsv | bschema a:u64,... | bpartition prefix {num_buckets}', stdin=csv, echo=True)
        assert result == shell.run('bcat --prefix prefix*')

def test_fail1():
    args = (254, '0,a\n')
    num_buckets, csv = args
    result = expected(num_buckets, csv)
    with shell.tempdir():
        stdout = '\n'.join(sorted({l.split(':')[0] for l in result.splitlines()}))
        assert stdout == shell.run(f'bsv | bschema a:u64,... | bpartition prefix {num_buckets}', stdin=csv, echo=True)
        assert result == shell.run('bcat --prefix prefix*')

def test_basic():
    with shell.tempdir():
        stdin = """
        0,b,c,d
        1,e,f,g
        2,h,i,j
        """
        stdout = """
        prefix00
        prefix01
        prefix02
        """
        assert rm_whitespace(unindent(stdout)) == shell.run('bsv | bschema a:u64,... | bpartition prefix 10', stdin=unindent(stdin))
        stdout = """
        prefix00:b,c,d
        prefix01:e,f,g
        prefix02:h,i,j
        """
        assert unindent(stdout).strip() == shell.run('bcat --prefix prefix*')
        stdout = """
        prefix00
        prefix01
        prefix02
        """
        assert unindent(stdout).strip() == shell.run('ls prefix*')

def test_appends():
    with shell.tempdir():
        stdin = """
        0,b,c,d
        1,e,f,g
        2,h,i,j
        """
        stdout = """
        prefix00
        prefix01
        prefix02
        """
        assert rm_whitespace(unindent(stdout)) == shell.run('bsv | bschema a:u64,... | bpartition prefix 10', stdin=unindent(stdin))
        assert rm_whitespace(unindent(stdout)) == shell.run('bsv | bschema a:u64,... | bpartition prefix 10', stdin=unindent(stdin))
        stdout = """
        prefix00:b,c,d
        prefix00:b,c,d
        prefix01:e,f,g
        prefix01:e,f,g
        prefix02:h,i,j
        prefix02:h,i,j
        """
        assert unindent(stdout).strip() == shell.run('bcat --prefix prefix*')
        stdout = """
        prefix00
        prefix01
        prefix02
        """
        assert unindent(stdout).strip() == shell.run('ls prefix*')
