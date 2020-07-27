import random
import os
import string
import shell
from hypothesis.database import ExampleDatabase
from hypothesis import given, settings
from hypothesis.strategies import text, lists, composite, integers, integers, sampled_from
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
    compile_buffer_sizes('bcut', buffers)
    compile_buffer_sizes('btopn', buffers)
    compile_buffer_sizes('bschema', buffers)
    shell.run('make bsv csv bcut btopn bschema', stream=True)

def teardown_module(m):
    os.chdir(m.orig)
    os.environ['PATH'] = m.path
    assert m.tempdir.startswith('/tmp/') or m.tempdir.startswith('/private/var/folders/')
    shell.run('rm -rf', m.tempdir)

@composite
def inputs(draw):
    buffer = draw(sampled_from(buffers))
    n = draw(integers(min_value=1, max_value=16))
    num_columns = draw(integers(min_value=1, max_value=3))
    column = text(string.ascii_lowercase, min_size=1, max_size=20)
    line = lists(column, min_size=num_columns, max_size=num_columns)
    lines = draw(lists(line))
    lines = [','.join(map(str, line)) for line in lines]
    return buffer, n, '\n'.join(lines) + '\n'

def expected(n, csv):
    xs = csv.splitlines()
    xs = [x.split(',')[0] for x in xs if x]
    xs = sorted(xs, reverse=True)[:n]
    return '\n'.join(xs) + '\n'

@given(inputs())
@settings(database=ExampleDatabase(':memory:'), max_examples=100 * int(os.environ.get('TEST_FACTOR', 1)), deadline=os.environ.get("TEST_DEADLINE", 1000 * 60)) # type: ignore
def test_props(args):
    buffer, n, csv = args
    result = expected(n, csv)
    assert result == run(csv, f'bsv.{buffer} | btopn.{buffer} {n} | bcut.{buffer} 1 | csv.{buffer}')
