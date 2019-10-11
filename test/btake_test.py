import os
import shell
from hypothesis import given, settings
from hypothesis.strategies import lists, composite, integers, randoms, sampled_from, floats
from test_util import run

def setup_module():
    with shell.climb_git_root():
        shell.run('make clean', stream=True)
        shell.run('make bsv csv btake', stream=True)

def teardown_module():
    with shell.climb_git_root():
        shell.run('make clean', stream=True)

with open('/usr/share/dict/words') as f:
    words = f.read().splitlines()

@composite
def inputs(draw):
    r = draw(randoms())
    word = sampled_from(words)
    num_columns = draw(integers(min_value=1, max_value=64))
    line = lists(word, min_size=1, max_size=num_columns)
    lines = draw(lists(line, min_size=1))
    first_column_values = [line[0] for line in lines]
    threshold = draw(floats(min_value=0, max_value=1))
    for line in lines:
        if line and r.random() > threshold:
            line[0] = r.choice(first_column_values)
    lines = sorted(lines)
    csv = '\n'.join([','.join(l) for l in lines if l]).strip() + '\n'
    value = r.choice(first_column_values + [r.choice(words) for _ in range(len(lines))]) # sometimes look for values that arent in the dataset
    return value, csv

def expected(value, csv):
    res = []
    for line in csv.splitlines():
        columns = line.split(',')
        if columns and columns[0] != value:
            break
        res.append(line)
    return '\n'.join(res) + '\n'

@given(inputs())
@settings(max_examples=100 * int(os.environ.get('TEST_FACTOR', 1)), deadline=os.environ.get("TEST_DEADLINE", 1000 * 60))
def test_props(args):
    value, csv = args
    result = expected(value, csv)
    assert result == run(csv, f'bin/bsv | bin/btake "{value}" | bin/csv')
