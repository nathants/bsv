import random
import os
import string
import shell
from hypothesis.database import ExampleDatabase
from hypothesis import given, settings
from hypothesis.strategies import lists, composite, integers, text, sampled_from
from test_util import run, clone_source, compile_buffer_sizes

if os.environ.get('TEST_FACTOR'):
    buffers = list(sorted(set([128, 256, 1024, 1024 * 1024 * 5] + [random.randint(128, 1024) for _ in range(10)])))
else:
    buffers = [128]

def setup_module(m):
    m.tempdir = clone_source()
    m.orig = os.getcwd()
    m.path = os.environ['PATH']
    os.chdir(m.tempdir)
    os.environ['PATH'] = f'{os.getcwd()}/bin:/usr/bin:/usr/local/bin:/sbin:/usr/sbin:/bin'
    shell.run('make clean', stream=True)
    compile_buffer_sizes('csv', buffers)
    compile_buffer_sizes('bsv', buffers)
    compile_buffer_sizes('bunzip', buffers)
    compile_buffer_sizes('bcat', buffers)
    shell.run('make bsv csv bcat bunzip', stream=True)

def teardown_module(m):
    os.chdir(m.orig)
    os.environ['PATH'] = m.path
    assert m.tempdir.startswith('/tmp/') or m.tempdir.startswith('/private/var/folders/')
    shell.run('rm -rf', m.tempdir)

@composite
def inputs(draw):
    buffer = draw(sampled_from(buffers))
    num_columns = draw(integers(min_value=1, max_value=12))
    zipcol = integers(min_value=0, max_value=num_columns - 1)
    zipcols = draw(lists(zipcol, min_size=1, max_size=16))
    column = text(string.ascii_lowercase, min_size=1, max_size=5)
    columns = lists(column, min_size=num_columns, max_size=num_columns)
    lines = draw(lists(columns, min_size=1))
    csv = '\n'.join([','.join(line) for line in lines]) + '\n'
    return buffer, zipcols, csv

@given(inputs())
@settings(database=ExampleDatabase(':memory:'), max_examples=100 * int(os.environ.get('TEST_FACTOR', 1)), deadline=os.environ.get("TEST_DEADLINE", 1000 * 60)) # type: ignore
def test_props(args):
    buffer, zipcols, csv = args
    just = max(len(str(zipcol)) for zipcol in zipcols)
    zipcols = [str(i).rjust(just, '0') for i in zipcols]
    for i, column in enumerate(run(csv, f'bsv.{buffer} | bunzip.{buffer} prefix').splitlines()):
        result = '\n'.join(row.split(',')[i] for row in csv.splitlines())
        assert result == shell.run('bcat', column)
