import os
import string
import shell
import random
from hypothesis.database import ExampleDatabase
from hypothesis import given, settings
from hypothesis.strategies import text, lists, composite, integers, randoms, floats, sampled_from
from test_util import run, clone_source, compile_buffer_sizes

if os.environ.get('TEST_FACTOR'):
    buffers = list(sorted(set([12, 17, 64, 256, 1024, 1024 * 1024 * 5] + [random.randint(8, 1024) for _ in range(10)])))
else:
    buffers = [12, 17, 64, 1024 * 1024 * 5]

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
    compile_buffer_sizes('bdropuntil', buffers)
    shell.run('make bsv csv bsort bdropuntil', stream=True)

def teardown_module(m):
    os.chdir(m.orig)
    os.environ['PATH'] = m.path
    assert m.tempdir.startswith('/tmp/') or m.tempdir.startswith('/private/var/folders/')
    shell.run('rm -rf', m.tempdir)

def partition(r, n, x):
    res = []
    ks = list(sorted({r.randint(1, max(1, len(x))) for _ in range(n)}))
    ks = [0] + ks
    ks[-1] = len(x)
    for a, b in zip(ks, ks[1:]):
        res.append(x[a:b])
    return res

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
    found = False
    lines = csv.splitlines()
    lines = [l.split(',') for l in lines]
    lines = sorted(lines, key=lambda x: x[0])
    for cols in lines:
        line = ','.join(str(c) for c in cols)
        if found:
            res.append(line)
        else:
            if cols:
                if cols[0] >= value:
                    res.append(line)
                    found = True
    return '\n'.join(res) + '\n'

@given(inputs())
@settings(database=ExampleDatabase(':memory:'), max_examples=100 * int(os.environ.get('TEST_FACTOR', 1)), deadline=os.environ.get("TEST_DEADLINE", 1000 * 60)) # type: ignore
def test_props(args):
    value, csv, buffer = args
    result = expected(value, csv)
    assert set(result.splitlines()) == set(run(csv, f'bsv.{buffer} | bsort.{buffer} | bdropuntil.{buffer} "{value}" | csv.{buffer}').splitlines()) # set because sort is not stable and is only for first column values

def test_example1():
    value, csv = 'g', 'a\nb\nc\nd\ne\nf\ng\nh\n'
    result = expected(value, csv)
    assert result == run(csv, f'bsv 2>/dev/null | bsort | bdropuntil "{value}" | csv 2>/dev/null')

def test_example2():
    value, csv = 'a', 'a\n'
    result = expected(value, csv)
    assert result == run(csv, f'bsv 2>/dev/null | bsort | bdropuntil "{value}" | csv 2>/dev/null')

def test_example3():
    value, csv = 'ga', 'a\nb\nc\nddd\neee\nf\nga\n'
    result = expected(value, csv)
    assert result == run(csv, f'bsv 2>/dev/null | bsort | bdropuntil "{value}" | csv 2>/dev/null')

def test_example4():
    value, csv = 'b', 'a\na\na\nb\n'
    result = expected(value, csv)
    assert result == run(csv, f'bsv 2>/dev/null | bsort | bdropuntil "{value}" | csv 2>/dev/null')
