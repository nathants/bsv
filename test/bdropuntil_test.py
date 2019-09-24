import os
import string
import shell
from hypothesis import given, settings
from hypothesis.strategies import text, lists, composite, integers, randoms, sampled_from
import random
from test_util import run, compile_buffer_sizes

if os.environ.get('TEST_FACTOR'):
    buffers = list(sorted(set([5, 8, 11, 17, 64, 256, 1024] + [random.randint(8, 1024) for _ in range(10)])))
else:
    buffers = [5, 8, 11, 17, 64]

def setup_module():
    with shell.climb_git_root():
        shell.run('make clean', stream=True)
        compile_buffer_sizes('csv', buffers)
        compile_buffer_sizes('bsv', buffers)
        compile_buffer_sizes('bdropuntil', buffers)
        shell.run('make bsv csv bdropuntil', stream=True)

def teardown_module():
    with shell.climb_git_root():
        shell.run('make clean', stream=True)

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
    buffer = draw(sampled_from(buffers))
    r = draw(randoms())
    bytes_available = buffer - 2
    max_columns = bytes_available // 2
    num_columns = draw(integers(min_value=1, max_value=min(64, max_columns)))
    bytes_available -= num_columns * 2
    line = text(string.ascii_lowercase, min_size=0, max_size=bytes_available)
    line = line.filter(lambda x: len(x) > 0)
    line = line.map(lambda x: partition(r, num_columns, x))
    lines = draw(lists(line, min_size=1))
    first_column_values = [line[0] for line in lines]
    for line in lines:
        if line and random.random() > 0.8:
            line[0] = random.choice(first_column_values)
    lines = sorted(lines)
    lines = '\n'.join([','.join(l)[:bytes_available + num_columns - 1] # swapping out line[0] sometimes causes oversized rows, so trim to max row size
                       for l in lines if l]).strip() + '\n'
    value = random.choice(first_column_values)
    return buffer, value, lines

def expected(value, csv):
    res = []
    found = False
    for line in csv.splitlines():
        if found:
            res.append(line)
        else:
            columns = line.split(',')
            if columns and columns[0] >= value:
                res.append(line)
                found = True
    return '\n'.join(res) + '\n'

@given(inputs())
@settings(max_examples=100 * int(os.environ.get('TEST_FACTOR', 1)), deadline=os.environ.get("TEST_DEADLINE", 1000 * 60))
def test_props(args):
    buffer, value, csv = args
    result = expected(value, csv)
    assert result == run(csv, f'bin/bsv.{buffer} | bin/bdropuntil.{buffer} "{value}" | bin/csv.{buffer}')

def test_example1():
    buffer, value, csv = 11, 'g', 'a\nb\nc\nd\ne\nf\ng\nh\n'
    result = expected(value, csv)
    assert result == run(csv, f'bin/bsv.{buffer} 2>/dev/null | bin/bdropuntil.{buffer} "{value}" | bin/csv.{buffer} 2>/dev/null')

def test_example2():
    buffer, value, csv = 11, 'a', 'a\n'
    result = expected(value, csv)
    assert result == run(csv, f'bin/bsv.{buffer} 2>/dev/null | bin/bdropuntil.{buffer} "{value}" | bin/csv.{buffer} 2>/dev/null')

def test_example3():
    buffer, value, csv = 11, 'ga', 'a\nb\nc\nddd\neee\nf\nga\n'
    result = expected(value, csv)
    assert result == run(csv, f'bin/bsv.{buffer} 2>/dev/null | bin/bdropuntil.{buffer} "{value}" | bin/csv.{buffer} 2>/dev/null')

def test_example4():
    buffer, value, csv = 11, 'b', 'a\na\na\nb\n'
    result = expected(value, csv)
    assert result == run(csv, f'bin/bsv.{buffer} 2>/dev/null | bin/bdropuntil.{buffer} "{value}" | bin/csv.{buffer} 2>/dev/null')
