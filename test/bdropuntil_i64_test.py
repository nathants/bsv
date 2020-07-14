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
    os.environ['PATH'] = f'{os.getcwd()}/bin:/usr/bin:/usr/local/bin:/sbin:/usr/sbin:/bin'
    shell.run('make clean && make bsv csv bschema bsort bdropuntil', stream=True)

def teardown_module(m):
    os.chdir(m.orig)
    os.environ['PATH'] = m.path
    assert m.tempdir.startswith('/tmp/') or m.tempdir.startswith('/private/var/folders/')
    shell.run('rm -rf', m.tempdir)

@composite
def inputs(draw):
    r = draw(randoms())
    num_columns = draw(integers(min_value=1, max_value=4))
    column = integers(min_value=-9223372036854775807, max_value=9223372036854775807)
    line = lists(column, min_size=num_columns, max_size=num_columns)
    lines = draw(lists(line, min_size=1))
    lines = [[str(x) for x in line] for line in lines]
    first_column_values = [line[0] for line in lines]
    threshold = draw(floats(min_value=0, max_value=1))
    for line in lines:
        if line and r.random() > threshold:
            line[0] = r.choice(first_column_values)
    csv = '\n'.join([','.join(l) for l in lines if l]).strip() + '\n'
    value = r.choice(first_column_values)
    return value, csv

def expected(value, csv):
    value = int(value)
    res = []
    found = False
    lines = csv.splitlines()
    lines = [[int(x) for x in l.split(',')] for l in lines]
    lines = sorted(lines)
    for cols in lines:
        if found:
            res.append(cols[0])
        else:
            if cols:
                if cols[0] >= value:
                    res.append(cols[0])
                    found = True
    return '\n'.join(map(str, res)) + '\n'

@given(inputs())
@settings(database=ExampleDatabase(':memory:'), max_examples=100 * int(os.environ.get('TEST_FACTOR', 1)), deadline=os.environ.get("TEST_DEADLINE", 1000 * 60)) # type: ignore
def test_props(args):
    value, csv = args
    result = expected(value, csv)
    assert result.splitlines() == run(csv, f'bsv | bschema a:i64,... | bsort i64 | bdropuntil "{value}" i64 | bschema i64:a | csv').splitlines()
