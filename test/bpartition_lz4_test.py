import os
import string
import shell
import collections
import xxh3
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
    shell.run('make clean && make bsv csv blz4d bcat bpartition', stream=True)

def teardown_module(m):
    os.chdir(m.orig)
    os.environ['PATH'] = m.path
    assert m.tempdir.startswith('/tmp/') or m.tempdir.startswith('/private/var/folders/')
    shell.run('rm -rf', m.tempdir)

@composite
def inputs(draw):
    num_buckets = draw(integers(min_value=1, max_value=128))
    num_columns = draw(integers(min_value=1, max_value=12))
    column = text(string.ascii_lowercase, min_size=1)
    columns = lists(column, min_size=1, max_size=num_columns)
    lines = draw(lists(columns, min_size=1))
    csv = '\n'.join([','.join(line) for line in lines]) + '\n'
    return num_buckets, csv

def expected(num_buckets, csv):
    res = collections.defaultdict(list)
    size = len(str(num_buckets))
    for line in csv.splitlines():
        col0 = line.split(',', 1)[0]
        bucket = xxh3.oneshot_int(col0.encode()) % num_buckets
        res[str(bucket).zfill(size)].append(line)
    val = ''
    for k in sorted(res):
        for line in res[k]:
            val += f'prefix_{k}:{line}\n'
    return val.strip()

@given(inputs())
@settings(database=ExampleDatabase(':memory:'), max_examples=100 * int(os.environ.get('TEST_FACTOR', 1)), deadline=os.environ.get("TEST_DEADLINE", 1000 * 60)) # type: ignore
def test_props(args):
    num_buckets, csv = args
    result = expected(num_buckets, csv)
    with shell.tempdir():
        stdout = '\n'.join(sorted({l.split(':')[0] for l in result.splitlines()}))
        assert stdout == shell.run(f'bsv | bpartition -l {num_buckets} prefix', stdin=csv, echo=True)
        assert result == shell.run('bcat -l -p prefix*')

def test_without_prefix():
    with shell.tempdir():
        stdin = """
        b,c,d
        e,f,g
        h,i,j
        """
        stdout = """
        02
        04
        05
        """
        assert rm_whitespace(unindent(stdout)) == shell.run('bsv | bpartition -l 10', stdin=unindent(stdin))

def test_basic():
    with shell.tempdir():
        stdin = """
        b,c,d
        e,f,g
        h,i,j
        """
        stdout = """
        prefix_02
        prefix_04
        prefix_05
        """
        assert rm_whitespace(unindent(stdout)) == shell.run('bsv | bpartition -l 10 prefix', stdin=unindent(stdin))
        stdout = """
        prefix_02:h,i,j
        prefix_04:e,f,g
        prefix_05:b,c,d
        """
        assert unindent(stdout).strip() == shell.run('bcat -l -p prefix*')
        stdout = """
        prefix_02
        prefix_04
        prefix_05
        """
        assert unindent(stdout).strip() == shell.run('ls prefix*')

def test_appends():
    with shell.tempdir():
        stdin = """
        b,c,d
        e,f,g
        h,i,j
        """
        stdout = """
        prefix_02
        prefix_04
        prefix_05
        """
        assert rm_whitespace(unindent(stdout)) == shell.run('bsv | bpartition -l 10 prefix', stdin=unindent(stdin))
        assert rm_whitespace(unindent(stdout)) == shell.run('bsv | bpartition -l 10 prefix', stdin=unindent(stdin))
        stdout = """
        prefix_02:h,i,j
        prefix_02:h,i,j
        prefix_04:e,f,g
        prefix_04:e,f,g
        prefix_05:b,c,d
        prefix_05:b,c,d
        """
        assert unindent(stdout).strip() == shell.run('bcat -l -p prefix*')
        stdout = """
        prefix_02
        prefix_04
        prefix_05
        """
        assert unindent(stdout).strip() == shell.run('ls prefix*')
