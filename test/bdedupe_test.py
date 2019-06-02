import os
import string
import shell
import hypothesis
from hypothesis import given, settings
from hypothesis.strategies import text, lists, composite, integers, randoms
from test_util import run, rm_whitespace, rm_whitespace

def setup_module():
    with shell.climb_git_root():
        shell.run('make clean && make bsv csv bdedupe', stream=True)

@composite
def inputs(draw):
    random = draw(randoms())
    num_columns = draw(integers(min_value=1, max_value=12))
    max_repeats = draw(integers(min_value=1, max_value=3))
    column = text(string.ascii_lowercase, min_size=1, max_size=3)
    line = lists(column, min_size=num_columns, max_size=num_columns)
    lines = draw(lists(line))
    lines = [','.join(x) for x in lines]
    lines = [l
             for line in lines
             for l in [line] * (1 if random.random() > .5 else random.randint(1, max_repeats))]
    return '\n'.join(lines) + '\n'

def expected(csv):
    lines = csv.splitlines()
    result = []
    for line in lines:
        if not result or result[-1] != line:
            result.append(line)
    return '\n'.join(result) + '\n'

@given(inputs())
@settings(max_examples=100 * int(os.environ.get('TEST_FACTOR', 1)))
def test_props(args):
    csv = args
    result = expected(csv)
    assert result == run(csv, f'bin/bsv | bin/bdedupe | bin/csv')

def test_basic():
    stdin = """
    a
    a
    a
    b
    b
    a
    a
    """
    stdout = """
    a
    b
    a
    """
    assert rm_whitespace(stdout) + '\n' == run(rm_whitespace(stdin), 'bin/bsv | bin/bdedupe | bin/csv')
