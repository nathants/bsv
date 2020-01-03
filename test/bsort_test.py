import pytest
import os
import string
import shell
from hypothesis.database import ExampleDatabase
from hypothesis import given, settings
from hypothesis.strategies import text, lists, composite, integers, randoms
from test_util import run, rm_whitespace, clone_source

def setup_module(m):
    m.tempdir = clone_source()
    m.orig = os.getcwd()
    m.path = os.environ['PATH']
    os.chdir(m.tempdir)
    os.environ['PATH'] = f'{os.getcwd()}/bin:/usr/bin:/usr/local/bin'
    shell.run('make clean && make bsv csv bsort', stream=True)

def teardown_module(m):
    os.chdir(m.orig)
    os.environ['PATH'] = m.path
    assert m.tempdir.startswith('/tmp/')
    shell.run('rm -rf', m.tempdir)

@composite
def inputs(draw):
    num_columns = draw(integers(min_value=1, max_value=64))
    column = text(string.ascii_lowercase, min_size=1, max_size=64)
    line = lists(column, min_size=num_columns, max_size=num_columns)
    lines = draw(lists(line))
    csv = '\n'.join([','.join(x) for x in lines]) + '\n'
    return csv

def expected(csv):
    xs = csv.splitlines()
    xs = sorted(xs)
    return '\n'.join(xs) + '\n'

@given(inputs())
@settings(database=ExampleDatabase(':memory:'), max_examples=100 * int(os.environ.get('TEST_FACTOR', 1)), deadline=os.environ.get("TEST_DEADLINE", 1000 * 60))
def test_props(csv):
    result = expected(csv)
    if result:
        assert result == run(csv, f'bsv | bsort | bin/csv')
    else:
        with pytest.raises(AssertionError):
            run(csv, f'bsv | bsort | bin/csv')

@given(inputs())
@settings(database=ExampleDatabase(':memory:'), max_examples=100 * int(os.environ.get('TEST_FACTOR', 1)), deadline=os.environ.get("TEST_DEADLINE", 1000 * 60))
def test_props_compatability(csv):
    assert run(csv, f'LC_ALL=C sort') == run(csv, f'bsv | bsort | bin/csv')

def expected_numeric(csv):
    xs = csv.splitlines()
    xs = [tuple(int(y) for y in x.split(',') if y) for x in xs]
    xs = sorted(xs)
    xs = [','.join(str(y) for y in x) for x in xs]
    return '\n'.join(xs) + '\n'

@composite
def inputs_numeric(draw):
    num_columns = draw(integers(min_value=1, max_value=4))
    column = text(string.digits, min_size=1, max_size=8)
    line = lists(column, min_size=num_columns, max_size=num_columns)
    lines = draw(lists(line))
    csv = '\n'.join([','.join(y for y in x) for x in lines]) + '\n'
    return csv

@given(inputs_numeric())
@settings(database=ExampleDatabase(':memory:'), max_examples=100 * int(os.environ.get('TEST_FACTOR', 1)), deadline=os.environ.get("TEST_DEADLINE", 1000 * 60))
def test_props_numeric(csv):
    result = expected_numeric(csv)
    if result:
        assert result == run(csv, f'bsv | bsort | bin/csv')
    else:
        with pytest.raises(AssertionError):
            run(csv, f'bsv | bsort | bin/csv')

def expected_mixed(csv):
    xs = csv.splitlines()
    xs = [tuple(int(y) if y.isdigit() else y for y in x.split(',') if y) for x in xs]
    xs = sorted(xs)
    xs = [','.join(str(y) for y in x) for x in xs]
    return '\n'.join(xs) + '\n'

@composite
def inputs_mixed(draw):
    r = draw(randoms())
    num_text_columns = draw(integers(min_value=1, max_value=4))
    text_column = text(string.ascii_lowercase, min_size=1, max_size=8)
    text_line = lists(text_column, min_size=num_text_columns, max_size=num_text_columns)
    text_lines = draw(lists(text_line))
    num_digit_columns = draw(integers(min_value=1, max_value=4))
    digit_column = text(string.digits, min_size=1, max_size=8)
    digit_line = lists(digit_column, min_size=num_digit_columns, max_size=num_digit_columns)
    digit_lines = draw(lists(digit_line, min_size=len(text_lines), max_size=len(text_lines)))
    if r.random() > 0.5:
        lines = zip(text_lines, digit_lines)
    else:
        lines = zip(digit_lines, text_lines)
    return '\n'.join(','.join(x + y) for x, y in lines)

@given(inputs_mixed())
@settings(database=ExampleDatabase(':memory:'), max_examples=100 * int(os.environ.get('TEST_FACTOR', 1)), deadline=os.environ.get("TEST_DEADLINE", 1000 * 60))
def test_props_mixed(csv):
    result = expected_mixed(csv)
    if result:
        assert result == run(csv, f'bsv | bsort | bin/csv')
    else:
        with pytest.raises(AssertionError):
            run(csv, f'bsv | bsort | bin/csv')

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
    assert rm_whitespace(stdout) + '\n' == run(rm_whitespace(stdin), 'bsv | bsort | bin/csv')
