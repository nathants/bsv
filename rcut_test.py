import pytest
import os
import string
from hypothesis import given, settings
from hypothesis.strategies import text, lists, composite, integers
import shell
import hashlib

tmp = os.environ.get('TMP_DIR', '/tmp').rstrip('/')
stdinpath = '%s/%s.stdin' % (tmp, hashlib.md5(__file__.encode('ascii')).hexdigest())
stdoutpath = '%s/%s.stdout' % (tmp, hashlib.md5(__file__.encode('ascii')).hexdigest())

def unindent(text):
    return '\n'.join([x.lstrip() for x in text.splitlines()]) + '\n'

def run(stdin, *args):
    with open(stdinpath, 'w') as f:
        f.write(unindent(stdin))
    try:
        shell.run(*(('cat', stdinpath, '|') + args + ('>', stdoutpath)), stream=True)
    except:
        raise AssertionError from None
    with open(stdoutpath) as f:
        return f.read()

shell.run('make clean && make rcut', stream=True)

MAX_COLUMNS = 64
MAX_LINE_BYTES = 8192

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
    return (fields, csv)

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
            if len(columns) > 64:
                return
            res = []
            for field in fields:
                if field > 64:
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
        assert result == run(csv, './rcut', fields)
    else:
        with pytest.raises(AssertionError):
            run(csv, './rcut', fields)

@given(inputs_ascending_unique_fields())
@settings(max_examples=100 * int(os.environ.get('TEST_FACTOR', 1)))
def test_props_compatability(args):
    fields, csv = args
    result = expected(fields, csv)
    if result:
        assert result == run(csv, './rcut', fields)
        assert result == run(csv, 'cut -d, -f' + fields)
    else:
        with pytest.raises(AssertionError):
            run(csv, './rcut', fields)

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
    assert unindent(stdout) == run(stdin, './rcut 1,2')
    assert unindent(stdout) == run(stdin, 'cut -d, -f1,2')

def test_double_digits():
    stdin = "1,2,3,4,5,6,7,8,9,10\n"
    stdout = "10\n"
    assert stdout == run(stdin, './rcut 10')

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
    assert unindent(stdout) == run(stdin, './rcut 2,3')

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
    assert unindent(stdout) == run(stdin, './rcut 1,3,1,1')

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
    assert unindent(stdout) == run(stdin, './rcut 1')
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
    assert unindent(stdout) == run(stdin, './rcut 1')
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
    assert unindent(stdout) == run(stdin, './rcut 2')

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
    assert unindent(stdout) == run(stdin, './rcut 1,2')
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
    assert unindent(stdout) == run(stdin, './rcut 1,3')
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
    assert unindent(stdout) == run(stdin, './rcut 1,3')

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
    assert unindent(stdout) == run(stdin, './rcut 2,1')
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
    assert unindent(stdout) == run(stdin, './rcut 3,1')
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
    assert unindent(stdout) == run(stdin, './rcut 3,1')

def test_fails_when_not_enough_columns():
    stdin = 'a,b,c'
    res = shell.run('./rcut 4 2>&1', stdin=stdin, warn=True)
    assert res['exitcode'] == 1
    assert 'error: line without 4 columns: a,b,c' == res['stdout']

def test_fails_when_non_positive_fields():
    stdin = 'a,b,c'
    res = shell.run('./rcut 0 2>&1', stdin=stdin, warn=True)
    assert res['exitcode'] == 1
    assert 'error: fields must be positive, got: 0' == res['stdout']

def test_fails_when_too_many_fields():
    stdin = 'a,b,c'
    print(list(range(1, MAX_COLUMNS + 1)))
    res = shell.run('./rcut', ','.join('1' for _ in range(MAX_COLUMNS + 1)), '2>&1', stdin=stdin, warn=True)
    assert res['exitcode'] == 1
    assert 'error: cannot select more than 64 fields' == res['stdout']

def test_fails_when_too_many_fields():
    stdin = 'a,b,c'
    print(list(range(1, MAX_COLUMNS + 1)))
    res = shell.run('./rcut', ','.join('1' for _ in range(MAX_COLUMNS + 1)), '2>&1', stdin=stdin, warn=True)
    assert res['exitcode'] == 1
    assert 'error: cannot select more than 64 fields' == res['stdout']
