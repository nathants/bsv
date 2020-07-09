import pytest
import uuid
import os
import string
import shell
import random
from hypothesis.database import ExampleDatabase
from hypothesis import given, settings
from hypothesis.strategies import lists, composite, integers, text, randoms, sampled_from
from test_util import run, clone_source, compile_buffer_sizes

if os.environ.get('TEST_FACTOR'):
    buffers = list(sorted(set([128, 256, 1024, 1024 * 1024 * 5] + [random.randint(128, 1024) for _ in range(10)])))
else:
    buffers = [128, 1024 * 1024 * 5]

def setup_module(m):
    m.tempdir = clone_source()
    m.orig = os.getcwd()
    m.path = os.environ['PATH']
    os.chdir(m.tempdir)
    os.environ['PATH'] = f'{os.getcwd()}/bin:/usr/bin:/usr/local/bin:/sbin:/usr/sbin:/bin'
    shell.run('make clean', stream=True)
    compile_buffer_sizes('csv', buffers)
    compile_buffer_sizes('bsv', buffers)
    compile_buffer_sizes('blz4', buffers)
    compile_buffer_sizes('bzip-lz4', buffers)
    compile_buffer_sizes('bunzip-lz4', buffers)
    shell.run('make bsv csv blz4 bzip-lz4 bunzip-lz4', stream=True)

def teardown_module(m):
    os.chdir(m.orig)
    os.environ['PATH'] = m.path
    assert m.tempdir.startswith('/tmp/') or m.tempdir.startswith('/private/var/folders/')
    shell.run('rm -rf', m.tempdir)

@composite
def inputs(draw):
    buffer = draw(sampled_from(buffers))
    rand = draw(randoms())
    num_columns = draw(integers(min_value=1, max_value=12))
    zipcol = integers(min_value=0, max_value=num_columns - 1)
    zipcols = draw(lists(zipcol, min_size=1, max_size=16))
    zipcols = list(set(zipcols))
    rand.shuffle(zipcols)
    column = text(string.ascii_lowercase, min_size=1, max_size=5)
    columns = lists(column, min_size=num_columns, max_size=num_columns)
    lines = draw(lists(columns, min_size=1))
    csv = '\n'.join([','.join(line) for line in lines]) + '\n'
    return buffer, zipcols, csv

def expected(zipcols, csv):
    res = []
    for line in csv.splitlines():
        columns = line.split(',')
        res.append(','.join(columns[zipcol] for zipcol in zipcols))
    return '\n'.join(res) + '\n'

@given(inputs())
@settings(database=ExampleDatabase(':memory:'), max_examples=100 * int(os.environ.get('TEST_FACTOR', 1)), deadline=os.environ.get("TEST_DEADLINE", 1000 * 60)) # type: ignore
def test_props(args):
    buffer, zipcols, csv = args
    result = expected(zipcols, csv)
    cols = ','.join(str(i + 1) for i in zipcols)
    prefix = str(uuid.uuid4())
    assert result == run(csv, f'bsv.{buffer} | bunzip-lz4.{buffer} {prefix} >/dev/null && ls {prefix}_* | bzip-lz4.{buffer} {cols} | csv.{buffer}')

def test_selection():
    shell.run('echo -e "a\nb\n" | bsv | blz4 > a')
    shell.run('echo -e "1\n2\n" | bsv | blz4 > b')
    assert '1,a\n2,b' == shell.run('echo a b | bzip-lz4 2,1 | csv')
    assert 'a,1\nb,2' == shell.run('echo a b | bzip-lz4 1,2 | csv')
    assert 'a\nb' == shell.run('echo a b | bzip-lz4 1 | csv')
    assert '1\n2' == shell.run('echo a b | bzip-lz4 2 | csv')
    with pytest.raises(Exception):
        assert '1\n2' == shell.run('echo a b | bzip-lz4 0 | csv')
    with pytest.raises(Exception):
        assert '1\n2' == shell.run('echo a b | bzip-lz4 3 | csv')
    with pytest.raises(Exception):
        assert '1\n2' == shell.run('echo a b | bzip-lz4 1,1 | csv')

def test_different_lengths():
    shell.run('echo -e "a\nb\nc\n" | bsv | blz4 > a')
    shell.run('echo -e "a\nb\n" | bsv | blz4 > b')
    with pytest.raises(Exception):
        shell.run('echo a b | bzip-lz4')

def test_more_than_1_column():
    shell.run('echo -e "a\nb\nc\n" | bsv | blz4 > a')
    shell.run('echo -e "a\nb\nc,c\n" | bsv | blz4 > b')
    with pytest.raises(Exception):
        shell.run('echo a b | bzip-lz4')
