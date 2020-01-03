import os
import string
import shell
from hypothesis.database import ExampleDatabase
from hypothesis import given, settings
from hypothesis.strategies import text, lists, composite, integers, randoms, floats
from test_util import run, clone_source

def setup_module(m):
    m.tempdir = clone_source()
    m.orig = os.getcwd()
    m.path = os.environ['PATH']
    os.chdir(m.tempdir)
    os.environ['PATH'] = f'{os.getcwd()}/bin:/usr/bin:/usr/local/bin'
    shell.run('make clean && make bsv csv bsort bdropuntil', stream=True)

def teardown_module(m):
    os.chdir(m.orig)
    os.environ['PATH'] = m.path
    assert m.tempdir.startswith('/tmp/')
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
    num_text_columns = draw(integers(min_value=1, max_value=4))
    text_column = text(string.ascii_lowercase, min_size=1, max_size=8)
    text_line = lists(text_column, min_size=num_text_columns, max_size=num_text_columns)
    text_lines = draw(lists(text_line, min_size=1))
    num_digit_columns = draw(integers(min_value=1, max_value=4))
    digit_column = text(string.digits, min_size=1, max_size=8)
    digit_line = lists(digit_column, min_size=num_digit_columns, max_size=num_digit_columns)
    digit_lines = draw(lists(digit_line, min_size=len(text_lines), max_size=len(text_lines)))
    if r.random() > 0.5:
        lines = zip(text_lines, digit_lines)
    else:
        lines = zip(digit_lines, text_lines)
    lines = [x + y for x, y in lines]
    first_column_values = [line[0] for line in lines]
    threshold = draw(floats(min_value=0, max_value=1))
    for line in lines:
        if line and r.random() > threshold:
            line[0] = r.choice(first_column_values)
    csv = '\n'.join([','.join(l) for l in lines if l]).strip() + '\n'
    value = r.choice(first_column_values)
    return value, csv

def parse(value):
    if value.isdigit():
        value = int(value)
    return value

def expected(value, csv):
    res = []
    found = False
    value = parse(value)
    lines = csv.splitlines()
    lines = [l.split(',') for l in lines]
    lines = [[parse(c) for c in cols] for cols in lines]
    lines = sorted(lines)
    for cols in lines:
        line = ','.join(str(c) for c in cols)
        if found:
            res.append(line)
        else:
            if cols:
                if {type(value), type(cols[0])} == {str, int}:
                    if type(value) == int: # numeric is always less than string
                        res.append(line)
                        found = True
                elif cols[0] >= value:
                    res.append(line)
                    found = True
    return '\n'.join(res) + '\n'

@given(inputs())
@settings(database=ExampleDatabase(':memory:'), max_examples=100 * int(os.environ.get('TEST_FACTOR', 1)), deadline=os.environ.get("TEST_DEADLINE", 1000 * 60))
def test_props(args):
    value, csv = args
    result = expected(value, csv)
    assert result == run(csv, f'bsv | bsort | bdropuntil "{value}" | bin/csv')

def test_example1():
    value, csv = 'g', 'a\nb\nc\nd\ne\nf\ng\nh\n'
    result = expected(value, csv)
    assert result == run(csv, f'bsv 2>/dev/null | bsort | bdropuntil "{value}" | bin/csv 2>/dev/null')

def test_example2():
    value, csv = 'a', 'a\n'
    result = expected(value, csv)
    assert result == run(csv, f'bsv 2>/dev/null | bsort | bdropuntil "{value}" | bin/csv 2>/dev/null')

def test_example3():
    value, csv = 'ga', 'a\nb\nc\nddd\neee\nf\nga\n'
    result = expected(value, csv)
    assert result == run(csv, f'bsv 2>/dev/null | bsort | bdropuntil "{value}" | bin/csv 2>/dev/null')

def test_example4():
    value, csv = 'b', 'a\na\na\nb\n'
    result = expected(value, csv)
    assert result == run(csv, f'bsv 2>/dev/null | bsort | bdropuntil "{value}" | bin/csv 2>/dev/null')

def test_example5():
    value, csv = '3', '5\n4\n3\n2\n1\n'
    result = expected(value, csv)
    assert result == run(csv, f'bsv 2>/dev/null | bsort | bdropuntil "{value}" | bin/csv 2>/dev/null')

def test_example6():
    value, csv = '10000', '20,a\n10000,a\n'
    result = expected(value, csv)
    assert result == run(csv, f'bsv 2>/dev/null | bsort | bdropuntil "{value}" | bin/csv 2>/dev/null')

def test_example7():
    value, csv = '1', '0,a\n1,a\n'
    result = expected(value, csv)
    assert result == run(csv, f'bsv 2>/dev/null | bsort | bdropuntil "{value}" | bin/csv 2>/dev/null')
