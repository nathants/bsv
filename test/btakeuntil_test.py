import os
import random
import shell
import string
from hypothesis.database import ExampleDatabase
from hypothesis import given, settings
from hypothesis.strategies import lists, composite, integers, randoms, floats, text, sampled_from
from test_util import run, clone_source, compile_buffer_sizes

if os.environ.get('TEST_FACTOR'):
    buffers = list(sorted(set([12, 17, 64, 128, 256, 1024, 1024 * 1024 * 5] + [random.randint(8, 1024) for _ in range(10)])))
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
    compile_buffer_sizes('bsort', buffers)
    compile_buffer_sizes('btakeuntil', buffers)
    shell.run('make bsv csv bsort btakeuntil', stream=True)

def teardown_module(m):
    os.chdir(m.orig)
    os.environ['PATH'] = m.path
    assert m.tempdir.startswith('/tmp/') or m.tempdir.startswith('/private/var/folders/')
    shell.run('rm -rf', m.tempdir)

with open('/usr/share/dict/words') as f:
    words = f.read().splitlines()

@composite
def inputs(draw):
    r = draw(randoms())
    buffer = draw(sampled_from(buffers))
    num_text_columns = draw(integers(min_value=1, max_value=4))
    text_column = text(string.ascii_lowercase, min_size=1, max_size=8)
    text_line = lists(text_column, min_size=num_text_columns, max_size=num_text_columns)
    lines = draw(lists(text_line, min_size=1))
    first_column_values = [line[0] for line in lines]
    threshold = draw(floats(min_value=0, max_value=1))
    for line in lines:
        if line and r.random() > threshold:
            line[0] = r.choice(first_column_values)
    csv = '\n'.join([','.join(l)[:buffer // 4] for l in lines if l]).strip() + '\n'
    value = r.choice(first_column_values)
    return value, csv, buffer

def expected(value, csv):
    res = []
    lines = csv.splitlines()
    lines = [line.split(',') for line in lines]
    lines = sorted(lines)
    for cols in lines:
        if cols:
            if cols[0] >= value:
                break
            res.append(','.join(str(c) for c in cols))
    return '\n'.join(res) + '\n'

@given(inputs())
@settings(database=ExampleDatabase(':memory:'), max_examples=100 * int(os.environ.get('TEST_FACTOR', 1)), deadline=os.environ.get("TEST_DEADLINE", 1000 * 60)) # type: ignore
def test_props(args):
    value, csv, buffer = args
    result = expected(value, csv)
    assert set(result.splitlines()) == set(run(csv, f'bsv.{buffer} | bsort.{buffer} | btakeuntil.{buffer} "{value}" | csv.{buffer}').splitlines()) # set because sort is not stable and is only for first column values
