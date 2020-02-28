import os
import shell
import string
from hypothesis.database import ExampleDatabase
from hypothesis import given, settings
from hypothesis.strategies import lists, composite, integers, randoms, floats, text
from test_util import run, clone_source

def setup_module(m):
    m.tempdir = clone_source()
    m.orig = os.getcwd()
    m.path = os.environ['PATH']
    os.chdir(m.tempdir)
    os.environ['PATH'] = f'{os.getcwd()}/bin:/usr/bin:/usr/local/bin'
    shell.run('make clean && make bsv csv bsort btakeuntil', stream=True)

def teardown_module(m):
    os.chdir(m.orig)
    os.environ['PATH'] = m.path
    assert m.tempdir.startswith('/tmp/')
    shell.run('rm -rf', m.tempdir)

with open('/usr/share/dict/words') as f:
    words = f.read().splitlines()

@composite
def inputs(draw):
    r = draw(randoms())
    num_text_columns = draw(integers(min_value=1, max_value=4))
    text_column = text(string.ascii_lowercase, min_size=1, max_size=8)
    text_line = lists(text_column, min_size=num_text_columns, max_size=num_text_columns)
    lines = draw(lists(text_line, min_size=1))
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
    value = parse(value)
    res = []
    lines = csv.splitlines()
    lines = [[parse(c) for c in line.split(',')] for line in lines]
    lines = sorted(lines)
    for cols in lines:
        if cols:
            if {type(value), type(cols[0])} == {str, int}:
                if type(value) == int:
                    break
            elif cols[0] >= value:
                break
            res.append(','.join(str(c) for c in cols))
    return '\n'.join(res) + '\n'

@given(inputs())
@settings(database=ExampleDatabase(':memory:'), max_examples=100 * int(os.environ.get('TEST_FACTOR', 1)), deadline=os.environ.get("TEST_DEADLINE", 1000 * 60))
def test_props(args):
    value, csv = args
    result = expected(value, csv)
    assert set(result.splitlines()) == set(run(csv, f'bsv | bsort | btakeuntil "{value}" | bin/csv').splitlines()) # set because sort is not stable and is only for first column values
