import pytest
import os
import string
import shell
from hypothesis.database import ExampleDatabase
from hypothesis import given, settings
from hypothesis.strategies import text, lists, composite, integers, randoms
from test_util import run, rm_whitespace, clone_source
import os
import shell
from test_util import unindent, rm_whitespace, clone_source

def setup_module(m):
    m.tempdir = clone_source()
    m.orig = os.getcwd()
    m.path = os.environ['PATH']
    os.chdir(m.tempdir)
    os.environ['PATH'] = f'{os.getcwd()}/bin:/usr/bin:/usr/local/bin:/sbin:/usr/sbin:/bin'
    shell.run('make clean && make bsv csv bsort bcut bmerge', stream=True)

def teardown_module(m):
    os.chdir(m.orig)
    os.environ['PATH'] = m.path
    assert m.tempdir.startswith('/tmp/') or m.tempdir.startswith('/private/var/folders/')
    shell.run('rm -rf', m.tempdir)

def test_basic():
    with shell.tempdir():
        shell.run('echo -e "a,a\nc,c\ne,e\n" | bsv > a.bsv')
        shell.run('echo -e "b,b\nd,d\nf,f\n" | bsv > b.bsv')
        stdout = """
        a,a
        b,b
        c,c
        d,d
        e,e
        f,f
        """
        assert rm_whitespace(unindent(stdout)) == shell.run(f'bmerge a.bsv b.bsv | csv', stream=True)

@composite
def inputs(draw):
    num_inputs = draw(integers(min_value=1, max_value=8))
    csvs = []
    for _ in range(num_inputs):
        num_columns = draw(integers(min_value=1, max_value=2))
        column = text(string.ascii_lowercase, min_size=1, max_size=4)
        line = lists(column, min_size=num_columns, max_size=num_columns)
        lines = draw(lists(line))
        csv = '\n'.join(sorted([','.join(x) for x in lines])) + '\n'
        csvs.append(csv)
    return csvs

def expected(csvs):
    xs = []
    for csv in csvs:
        xs += csv.splitlines()
    xs = sorted([x.split(',')[0] for x in xs])
    return '\n'.join(xs) + '\n'

@given(inputs())
@settings(database=ExampleDatabase(':memory:'), max_examples=100 * int(os.environ.get('TEST_FACTOR', 1)), deadline=os.environ.get("TEST_DEADLINE", 1000 * 60))
def test_props(csvs):
    result = expected(csvs)
    if result.strip():
        with shell.tempdir():
            paths = []
            for i, csv in enumerate(csvs):
                path = f'file{i}.bsv'
                shell.run(f'bsv > {path}', stdin=csv)
                paths.append(path)
            assert result.strip() == shell.run(f'bmerge', *paths, '| bcut 1 | csv', echo=True)
            assert shell.run('cat', *paths, '| bsort | bcut 1 | csv') == shell.run(f'bmerge', *paths, '| bcut 1 | csv')

@given(inputs())
@settings(database=ExampleDatabase(':memory:'), max_examples=100 * int(os.environ.get('TEST_FACTOR', 1)), deadline=os.environ.get("TEST_DEADLINE", 1000 * 60))
def test_props_compatability(csvs):
    result = expected(csvs)
    if result.strip():
        with shell.tempdir():
            bsv_paths = []
            for i, csv in enumerate(csvs):
                path = f'file{i}.bsv'
                shell.run(f'bsv > {path}', stdin=csv)
                bsv_paths.append(path)
            csv_paths = []
            for i, csv in enumerate(csvs):
                path = f'file{i}.csv'
                shell.run(f'cat - > {path}', stdin=csv)
                csv_paths.append(path)
            assert shell.run(f'LC_ALL=C sort -m -k1,1', *csv_paths, ' | cut -d, -f1') == shell.run(f'bmerge', *bsv_paths, ' | bcut 1 | csv', echo=True)
