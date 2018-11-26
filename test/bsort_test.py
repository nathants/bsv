import pytest
import os
import string
import shell
import hypothesis
from hypothesis import given, settings
from hypothesis.strategies import text, lists, composite, integers
from test_util import run, rm_whitespace

def setup_module():
    with shell.climb_git_root():
        shell.run('make clean && make bsv csv bsort', stream=True)

@composite
def inputs(draw):
    num_columns = draw(integers(min_value=1, max_value=12))
    column = text(string.ascii_lowercase, min_size=1)
    line = lists(column, min_size=num_columns, max_size=num_columns)
    lines = draw(lists(line))
    csv = '\n'.join([','.join(x) for x in lines]) + '\n'
    return csv

def expected(csv):
    return '\n'.join(sorted(csv.splitlines(), key=lambda x: x.replace(',', ''))) + '\n'

@given(inputs())
@settings(max_examples=100 * int(os.environ.get('TEST_FACTOR', 1)), timeout=hypothesis.unlimited, suppress_health_check=[hypothesis.HealthCheck.hung_test])
def test_props(csv):
    result = expected(csv)
    if result:
        assert result == run(csv, f'bin/bsv | bin/bsort | bin/csv')
    else:
        with pytest.raises(AssertionError):
            run(csv, f'bin/bsv | bin/bsort | bin/csv')

def test_compatability():
    stdin = """
    c
    b
    a
    """
    stdout = """
    a
    b
    c
    """
    assert rm_whitespace(stdout) + '\n' == run(rm_whitespace(stdin), 'bin/bsv | bin/bsort | bin/csv')
