import random
import os
import shell
from hypothesis.database import ExampleDatabase
from hypothesis import given, settings
from hypothesis.strategies import composite, integers, sampled_from
from test_util import compile_buffer_sizes, clone_source

if os.environ.get('TEST_FACTOR'):
    buffers = list(sorted(set([64, 256, 1024, 1024 * 1024 * 5] + [random.randint(64, 1024) for _ in range(10)])))
else:
    buffers = [64, 256, 1024, 1024 * 1024 * 5]

def setup_module(m):
    m.tempdir = clone_source()
    m.orig = os.getcwd()
    m.path = os.environ['PATH']
    os.chdir(m.tempdir)
    os.environ['PATH'] = f'{os.getcwd()}/bin:/usr/bin:/usr/local/bin:/sbin:/usr/sbin:/bin'
    shell.run('make clean', stream=True)
    compile_buffer_sizes('bsv', buffers)
    compile_buffer_sizes('csv', buffers)
    compile_buffer_sizes('bsplit', buffers)
    shell.run('make bsv csv bsplit xxh3 _gen_csv')

def teardown_module(m):
    os.chdir(m.orig)
    os.environ['PATH'] = m.path
    assert m.tempdir.startswith('/tmp/') or m.tempdir.startswith('/private/var/folders/')
    shell.run('rm -rf', m.tempdir)

@composite
def inputs(draw):
    buffer = draw(sampled_from(buffers))
    lines = draw(integers(max_value=1024))
    chunks_per_file = draw(integers(min_value=0, max_value=64))
    return buffer, lines, chunks_per_file

try:
    import bsv
except ImportError:
    pass # not working on mac via tox
else:
    @given(inputs())
    @settings(database=ExampleDatabase(':memory:'), max_examples=100 * int(os.environ.get('TEST_FACTOR', 1)), deadline=os.environ.get("TEST_DEADLINE", 1000 * 60))
    def test_props(args):
        buffer, lines, chunks_per_file = args
        if not chunks_per_file:
            chunks_per_file = ''
        with shell.tempdir():
            shell.run(f'_gen_csv 2 {lines} | bsv.{buffer} > data.bsv', echo=True)
            shell.run(f'cat data.bsv | bsplit.{buffer} {chunks_per_file} > filenames')
            assert shell.run(f'cat data.bsv | csv.{buffer} | xxh3') == shell.run(f'cat filenames | while read path; do cat $path; done | csv.{buffer} | xxh3')
