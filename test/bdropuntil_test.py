import os
import string
import shell
from hypothesis import given, settings
from hypothesis.strategies import text, lists, composite, integers, randoms
from test_util import run

def setup_module():
    with shell.climb_git_root():
        shell.run('make clean && make bsv csv bdropuntil', stream=True)

@composite
def inputs(draw):
    random = draw(randoms())
    num_columns = draw(integers(min_value=1, max_value=12))
    column = text(string.ascii_lowercase, min_size=1)
    line = lists(column, min_size=num_columns, max_size=num_columns)
    lines = draw(lists(line, min_size=1))
    first_column_values = [line[0] for line in lines if line]
    for line in lines:
        if line and random.random() > 0.8:
            line[0] = random.choice(first_column_values)
    lines = sorted(lines)
    csv = '\n'.join([','.join(line) for line in lines]) + '\n'
    value = random.choice(first_column_values)
    return value, csv

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
@settings(max_examples=100 * int(os.environ.get('TEST_FACTOR', 1)))
def test_props(args):
    value, csv = args
    result = expected(value, csv)
    assert result == run(csv, f'bin/bsv | bin/bdropuntil {value} | bin/csv')
