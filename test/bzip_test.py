import pytest
import uuid
import os
import string
import shell
from hypothesis.database import ExampleDatabase
from hypothesis import given, settings
from hypothesis.strategies import lists, composite, integers, text, randoms
from test_util import run, clone_source

def setup_module(m):
    m.tempdir = clone_source()
    m.orig = os.getcwd()
    m.path = os.environ['PATH']
    os.chdir(m.tempdir)
    os.environ['PATH'] = f'{os.getcwd()}/bin:/usr/bin:/usr/local/bin:/sbin:/usr/sbin:/bin'
    shell.run('make clean && make bsv csv bzip bunzip', stream=True)

def teardown_module(m):
    os.chdir(m.orig)
    os.environ['PATH'] = m.path
    assert m.tempdir.startswith('/tmp/') or m.tempdir.startswith('/private/var/folders/')
    shell.run('rm -rf', m.tempdir)

@composite
def inputs(draw):
    rand = draw(randoms())
    num_columns = draw(integers(min_value=1, max_value=12))
    zipcol = integers(min_value=0, max_value=num_columns - 1)
    zipcols = draw(lists(zipcol, min_size=1, max_size=16))
    zipcols = list(set(zipcols))
    rand.shuffle(zipcols)
    column = text(string.ascii_lowercase, min_size=1)
    columns = lists(column, min_size=num_columns, max_size=num_columns)
    lines = draw(lists(columns, min_size=1))
    csv = '\n'.join([','.join(line) for line in lines]) + '\n'
    return zipcols, csv

def expected(zipcols, csv):
    res = []
    for line in csv.splitlines():
        columns = line.split(',')
        res.append(','.join(columns[zipcol] for zipcol in zipcols))
    return '\n'.join(res) + '\n'

@given(inputs())
@settings(database=ExampleDatabase(':memory:'), max_examples=100 * int(os.environ.get('TEST_FACTOR', 1)), deadline=os.environ.get("TEST_DEADLINE", 1000 * 60)) # type: ignore
def test_props(args):
    zipcols, csv = args
    result = expected(zipcols, csv)
    cols = ','.join(str(i + 1) for i in zipcols)
    prefix = str(uuid.uuid4())
    assert result == run(csv, f'bsv | bunzip {prefix} >/dev/null && ls {prefix}_* | bzip {cols} | csv')

def test_selection():
    shell.run('echo -e "a\nb\n" | bsv > a')
    shell.run('echo -e "1\n2\n" | bsv > b')
    assert '1,a\n2,b' == shell.run('echo a b | bzip 2,1 | csv')
    assert 'a,1\nb,2' == shell.run('echo a b | bzip 1,2 | csv')
    assert 'a\nb' == shell.run('echo a b | bzip 1 | csv')
    assert '1\n2' == shell.run('echo a b | bzip 2 | csv')
    with pytest.raises(Exception):
        assert '1\n2' == shell.run('echo a b | bzip 0 | csv')
    with pytest.raises(Exception):
        assert '1\n2' == shell.run('echo a b | bzip 3 | csv')
    with pytest.raises(Exception):
        assert '1\n2' == shell.run('echo a b | bzip 1,1 | csv')

def test_different_lengths():
    shell.run('echo -e "a\nb\nc\n" | bsv > a')
    shell.run('echo -e "a\nb\n" | bsv > b')
    with pytest.raises(Exception):
        shell.run('echo a b | bzip')

def test_more_than_1_column():
    shell.run('echo -e "a\nb\nc\n" | bsv > a')
    shell.run('echo -e "a\nb\nc,c\n" | bsv > b')
    with pytest.raises(Exception):
        shell.run('echo a b | bzip')
