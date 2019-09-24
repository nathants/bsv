import pytest
import os
import string
import shell
from hypothesis import given, settings
from hypothesis.strategies import text, lists, composite, integers
from test_util import run, rm_whitespace, rm_whitespace, max_columns

def setup_module():
    with shell.climb_git_root():
        shell.run('make clean && make bsv csv bcut', stream=True)

@composite
def inputs(draw):
    num_columns = draw(integers(min_value=1, max_value=12))
    column = text(string.ascii_lowercase, min_size=1)
    line = lists(column, min_size=num_columns, max_size=num_columns)
    lines = draw(lists(line))
    csv = '\n'.join([','.join(x) for x in lines]) + '\n'
    field = integers(min_value=1, max_value=num_columns)
    fields = draw(lists(field, min_size=1, max_size=num_columns))
    fields = ','.join(map(str, fields))
    return fields, csv

@composite
def inputs_ascending_unique_fields(draw):
    fields, csv = draw(inputs())
    fields = ','.join(sorted(set(fields.split(',')), key=int))
    return (fields, csv)

def expected(fields, csv):
    fields = [int(x) - 1 for x in fields.split(',')]
    result = []
    for line in csv.splitlines():
        if not line.strip():
            result.append('')
        else:
            columns = line.split(',')
            if len(columns) > max_columns:
                return
            res = []
            for field in fields:
                if field > max_columns:
                    return
                try:
                    res.append(columns[field])
                except IndexError:
                    pass
            result.append(','.join(res))
    return '\n'.join(result) + '\n'

@given(inputs())
@settings(max_examples=100 * int(os.environ.get('TEST_FACTOR', 1)))
def test_props(args):
    fields, csv = args
    result = expected(fields, csv)
    if result:
        assert result == run(csv, f'bin/bsv | bin/bcut {fields} | bin/csv')
    else:
        with pytest.raises(AssertionError):
            run(csv, f'bin/bsv | bin/bcut {fields} | bin/csv')

@given(inputs_ascending_unique_fields())
@settings(max_examples=100 * int(os.environ.get('TEST_FACTOR', 1)))
def test_props_compatability(args):
    fields, csv = args
    result = expected(fields, csv)
    if result:
        assert result == run(csv, 'cut -d, -f' + fields)
        assert result == run(csv, f'bin/bsv | bin/bcut {fields} | bin/csv')
    else:
        with pytest.raises(AssertionError):
            run(csv, f'bin/bsv | bin/bcut {fields} | bin/csv')

def test_compatability():
    stdin = """
    a,b,c,d
    1,2,3
    x,y
    """
    stdout = """
    a,b
    1,2
    x,y
    """
    assert rm_whitespace(stdout) + '\n' == run(rm_whitespace(stdin), 'bin/bsv | bin/bcut 1,2 | bin/csv')
    assert rm_whitespace(stdout) + '\n' == run(rm_whitespace(stdin), 'cut -d, -f1,2')

def test_single_char():
    stdin = "1\n2\n10\n20\n"
    stdout = "1\n2\n10\n20\n"
    assert stdout == run(rm_whitespace(stdin), 'bin/bsv | bin/bcut 1 | bin/csv')

def test_double_digits():
    stdin = "1,2,3,4,5,6,7,8,9,10\n"
    stdout = "10\n"
    assert stdout == run(rm_whitespace(stdin), 'bin/bsv | bin/bcut 10 | bin/csv')

def test_holes():
    stdin = """
    a,b,
    1,,3
    x,y,z
    """
    stdout = """
    b,
    ,3
    y,z
    """
    assert rm_whitespace(stdout) + '\n' == run(rm_whitespace(stdin), 'bin/bsv | bin/bcut 2,3 | bin/csv')

def test_repeats():
    stdin = """
    x,y,z
    1,2,3
    a,b,c,d
    """
    stdout = """
    x,z,x,x
    1,3,1,1
    a,c,a,a
    """
    assert rm_whitespace(stdout) + '\n' == run(rm_whitespace(stdin), 'bin/bsv | bin/bcut 1,3,1,1 | bin/csv')

def test_single_column():
    stdin = """
    x,y
    1,2,3
    a,b,c,d
    """
    stdout = """
    x
    1
    a
    """
    assert rm_whitespace(stdout) + '\n' == run(rm_whitespace(stdin), 'bin/bsv | bin/bcut 1 | bin/csv')
    stdin = """
    a,b,c,d
    1,2,3
    x,y
    """
    stdout = """
    a
    1
    x
    """
    assert rm_whitespace(stdout) + '\n' == run(rm_whitespace(stdin), 'bin/bsv | bin/bcut 1 | bin/csv')
    stdin = """
    a,b,c,d
    1,2,3
    x,y
    """
    stdout = """
    b
    2
    y
    """
    assert rm_whitespace(stdout) + '\n' == run(rm_whitespace(stdin), 'bin/bsv | bin/bcut 2 | bin/csv')

def test_forward():
    stdin = """
    a,b,c,d
    1,2,3
    x,y
    """
    stdout = """
    a,b
    1,2
    x,y
    """
    assert rm_whitespace(stdout) + '\n' == run(rm_whitespace(stdin), 'bin/bsv |  bin/bcut 1,2 | bin/csv')
    stdin = """
    a,b,c,d
    1,2,3
    x,y,z
    """
    stdout = """
    a,c
    1,3
    x,z
    """
    assert rm_whitespace(stdout) + '\n' == run(rm_whitespace(stdin), 'bin/bsv | bin/bcut 1,3 | bin/csv')
    stdin = """
    x,y,z
    1,2,3
    a,b,c,d
    """
    stdout = """
    x,z
    1,3
    a,c
    """
    assert rm_whitespace(stdout) + '\n' == run(rm_whitespace(stdin), 'bin/bsv | bin/bcut 1,3 | bin/csv')

def test_reverse():
    stdin = """
    a,b,c,d
    1,2,3
    x,y
    """
    stdout = """
    b,a
    2,1
    y,x
    """
    assert rm_whitespace(stdout) + '\n' == run(rm_whitespace(stdin), 'bin/bsv | bin/bcut 2,1 | bin/csv')
    stdin = """
    a,b,c,d
    1,2,3
    x,y,z
    """
    stdout = """
    c,a
    3,1
    z,x
    """
    assert rm_whitespace(stdout) + '\n' == run(rm_whitespace(stdin), 'bin/bsv | bin/bcut 3,1 | bin/csv')
    stdin = """
    x,y,z
    1,2,3
    a,b,c,d
    """
    stdout = """
    z,x
    3,1
    c,a
    """
    assert rm_whitespace(stdout) + '\n' == run(rm_whitespace(stdin), 'bin/bsv | bin/bcut 3,1 | bin/csv')

def test_fails_when_not_enough_columns():
    with shell.climb_git_root():
        stdin = 'a,b,c'
        res = shell.run('bin/bsv | bin/bcut 4', stdin=stdin, warn=True)
        assert 'fatal: line without 4 columns: a,b,c' == res['stderr']
        assert res['exitcode'] == 1

def test_fails_when_non_positive_fields():
    with shell.climb_git_root():
        stdin = 'a,b,c'
        res = shell.run('bin/bsv | bin/bcut 0', stdin=stdin, warn=True)
        assert 'fatal: fields must be positive, got: 0' == res['stderr']
        assert res['exitcode'] == 1
