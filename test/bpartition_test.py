import os
import string
import shell
import collections
from hypothesis import given, settings
from hypothesis.strategies import text, lists, composite, integers, tuples
from test_util import run, unindent, rm_whitespace

def setup_module():
    with shell.climb_git_root():
        shell.run('make clean && make bsv csv bpartition', stream=True)

def teardown_module():
    with shell.climb_git_root():
        shell.run('make clean', stream=True)

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
            val += f'prefix{k}.csv:{line}\n'
    return val.strip()

csv = os.path.abspath('./bin/bsv')
bsv = os.path.abspath('./bin/csv')
partition = os.path.abspath('./bin/bpartition')

@given(inputs())
@settings(max_examples=100 * int(os.environ.get('TEST_FACTOR', 1)), deadline=os.environ.get("TEST_DEADLINE", 1000 * 60))
def test_props(args):
    num_buckets, csv = args
    result = expected(num_buckets, csv)
    with shell.tempdir():
        stdout = '\n'.join(sorted({l.split('.csv')[0] for l in result.splitlines()}))
        assert stdout == shell.run(f'bsv | {partition} {num_buckets} prefix', stdin=csv, echo=True)
        shell.run('for path in prefix*; do csv < $path > $path.csv; done')
        assert result == shell.run('grep --with-filename ".*" prefix*.csv')

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
        assert rm_whitespace(unindent(stdout)) == shell.run(f'bsv | {partition} 10 prefix', stdin=unindent(stdin))
        stdout = """
        prefix00.csv:b,c,d
        prefix01.csv:e,f,g
        prefix02.csv:h,i,j
        """
        shell.run('for path in prefix*; do csv < $path > $path.csv; done')
        assert unindent(stdout).strip() == shell.run('grep --with-filename ".*" prefix*.csv')
        stdout = """
        prefix00.csv
        prefix01.csv
        prefix02.csv
        """
        assert unindent(stdout).strip() == shell.run('ls prefix*.csv')

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
        assert rm_whitespace(unindent(stdout)) == shell.run(f'bsv | {partition} 10 prefix', stdin=unindent(stdin))
        assert rm_whitespace(unindent(stdout)) == shell.run(f'bsv | {partition} 10 prefix', stdin=unindent(stdin))
        stdout = """
        prefix00.csv:b,c,d
        prefix00.csv:b,c,d
        prefix01.csv:e,f,g
        prefix01.csv:e,f,g
        prefix02.csv:h,i,j
        prefix02.csv:h,i,j
        """
        shell.run('for path in prefix*; do csv < $path > $path.csv; done')
        assert unindent(stdout).strip() == shell.run('grep --with-filename ".*" prefix*.csv')
        stdout = """
        prefix00.csv
        prefix01.csv
        prefix02.csv
        """
        assert unindent(stdout).strip() == shell.run('ls prefix*.csv')
