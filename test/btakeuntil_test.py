import os
import shell
from hypothesis import given, settings
import string
from hypothesis.strategies import lists, composite, integers, randoms, sampled_from, floats, text
from test_util import run

def setup_module():
    with shell.climb_git_root():
        shell.run('make clean', stream=True)
        shell.run('make bsv csv bsort btakeuntil', stream=True)

def teardown_module():
    with shell.climb_git_root():
        shell.run('make clean', stream=True)

with open('/usr/share/dict/words') as f:
    words = f.read().splitlines()

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
@settings(max_examples=100 * int(os.environ.get('TEST_FACTOR', 1)), deadline=os.environ.get("TEST_DEADLINE", 1000 * 60))
def test_props(args):
    value, csv = args
    result = expected(value, csv)
    assert result == run(csv, f'bin/bsv | bin/bsort | bin/btakeuntil "{value}" | bin/csv')
