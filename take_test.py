import os
import string
from hypothesis import given, settings
from hypothesis.strategies import text, lists, composite, integers, randoms
import shell # https://github.com/nathants/py-shell
import hashlib

tmp = os.environ.get('TMP_DIR', '/tmp').rstrip('/')
stdinpath = '%s/%s.stdin' % (tmp, hashlib.md5(__file__.encode('ascii')).hexdigest())
stdoutpath = '%s/%s.stdout' % (tmp, hashlib.md5(__file__.encode('ascii')).hexdigest())

def rm_whitespace(x):
    return '\n'.join([y.strip().replace(' ', '')
                      for y in x.splitlines()
                      if y.strip()]) + '\n'

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

shell.run('make clean && make take', stream=True)

@composite
def inputs(draw):
    r = draw(randoms())
    num_columns = draw(integers(min_value=1, max_value=12))
    column = text(string.ascii_lowercase, min_size=1)
    line = lists(column, min_size=num_columns, max_size=num_columns)
    lines = draw(lists(line, min_size=3))
    token = r.choice(lines)[0]
    csv = '\n'.join(sorted([','.join(x) for x in lines])) + '\n'
    return token, csv

def expected(token, csv):
    return '\n'.join([x
                      for x in csv.splitlines()
                      if x.split(',')[0] == token]).rstrip() + '\n'

@given(inputs())
@settings(max_examples=100 * int(os.environ.get('TEST_FACTOR', 1)))
def test_props(args):
    token, csv = args
    result = expected(token, csv)
    assert result == run(csv, './take', token)

def test_basic():
    stdin = """
    a,1
    a,2
    b,1
    b,2
    c,1
    c,2
    """
    stdout = """
    b,1
    b,2
    """
    assert rm_whitespace(stdout) == run(stdin, './take b')
    assert '' == run(stdin, './take asdf')
