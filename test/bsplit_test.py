import random
import os
import shell
from hypothesis.database import ExampleDatabase
from hypothesis import given, settings
from hypothesis.strategies import composite, integers, sampled_from
from test_util import compile_buffer_sizes, clone_source

if os.environ.get('TEST_FACTOR'):
    buffers = list(sorted(set([64, 256, 1024] + [random.randint(64, 1024) for _ in range(10)])))
else:
    buffers = [64, 256, 1024]

def setup_module(m):
    m.tempdir = clone_source()
    m.orig = os.getcwd()
    m.path = os.environ['PATH']
    os.chdir(m.tempdir)
    os.environ['PATH'] = f'{os.getcwd()}/bin:/usr/bin:/usr/local/bin'
    shell.run('make clean', stream=True)
    compile_buffer_sizes('bsv', buffers)
    compile_buffer_sizes('csv', buffers)
    compile_buffer_sizes('bsplit', buffers)
    shell.run('make bsv csv bsplit xxh3 _gen_csv')

def teardown_module(m):
    os.chdir(m.orig)
    os.environ['PATH'] = m.path
    assert m.tempdir.startswith('/tmp/')
    shell.run('rm -rf', m.tempdir)

@composite
def inputs(draw):
    buffer = draw(sampled_from(buffers))
    bytes = draw(integers(min_value=buffer, max_value=1024 * 1024))
    return buffer, bytes

@given(inputs())
@settings(database=ExampleDatabase(':memory:'), max_examples=100 * int(os.environ.get('TEST_FACTOR', 1)), deadline=os.environ.get("TEST_DEADLINE", 1000 * 60))
def test_props(args):
    buffer, bytes = args
    with shell.tempdir():
        shell.run(f'_gen_csv 1 100000000 | head -c {bytes} | bsv.{buffer} > data.bsv', echo=True)
        shell.run(f'cat data.bsv | bsplit.{buffer} > filenames')
        assert shell.run(f'cat data.bsv | csv.{buffer} | xxh3') == shell.run(f'cat $(cat filenames) | csv.{buffer} | xxh3')

def test1():
    with shell.tempdir():
        shell.run('_gen_csv 8 1000000000 | head -c 17MB | bsv > data.bsv')
        shell.run('cat data.bsv | bsplit > filenames')
        assert '4' == shell.run('cat filenames | wc -l')
        assert shell.run('cat data.bsv | csv | xxh3') == shell.run('cat $(cat filenames) | csv | xxh3')
