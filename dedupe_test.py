import pytest
import string
import os
import string
from hypothesis import given, settings
from hypothesis.strategies import text, lists, composite, integers
import shell
import hashlib

tmp = os.environ.get('TMP_DIR', '/tmp').rstrip('/')
stdinpath = '%s/%s.stdin' % (tmp, hashlib.md5(__file__.encode('ascii')).hexdigest())
stdoutpath = '%s/%s.stdout' % (tmp, hashlib.md5(__file__.encode('ascii')).hexdigest())

def rm_whitespace(x):
    return '\n'.join([y.strip().replace(' ', '')
                      for y in x.splitlines()
                      if y.strip()])

def unindent(text):
    return '\n'.join([x.lstrip() for x in text.splitlines()]) + '\n'

def run(*args):
    try:
        shell.run(*args, echo=True)
    except:
        raise AssertionError from None

shell.run('make clean && make dedupe', stream=True)

@composite
def inputs(draw):
    line = text(string.ascii_letters, max_size=50)
    lines = lists(line)
    files = lists(lines, min_size=2)
    return {str(i): sorted(x) for i, x in enumerate(draw(files))}

def dedupe(xs):
    seen = set()
    for x in xs:
        if x not in seen:
            seen.add(x)
            yield x

def expected(inputs):
    uniqs = {i: set(x) for i, x in inputs.items()}
    res = {i: [y for y in dedupe(x) if y not in others]
           for i, x in inputs.items()
           for others in [set.union(*[z
                                      for j, z in uniqs.items()
                                      if j != i])]}

    return '\n'.join(['%(path)s.suffix:%(line)s' % locals()
                      for path in sorted(res)
                      for line in res[path]
                      if line])

@given(inputs())
@settings(max_examples=100 * int(os.environ.get('TEST_FACTOR', 1)))
def test_props(inputs):
    cmd = os.path.abspath('dedupe')
    with shell.tempdir():
        result = expected(inputs)
        for path, lines in inputs.items():
            with open(path, 'w') as f:
                f.write('\n'.join(lines) + '\n')
        run(cmd, 'suffix', *inputs)
        assert result == shell.run('(grep --with-filename ".*" *.suffix || true) | LC_ALL=C sort')

def test_basic():
    cmd = os.path.abspath('dedupe')
    with shell.tempdir():
        with open('a', 'w') as f:
            f.write(rm_whitespace("""
                1
                2
                3
            """))
        with open('b', 'w') as f:
            f.write(rm_whitespace("""
                3
                4
                5
                6
            """))
        with open('c', 'w') as f:
            f.write(rm_whitespace("""
                5
                6
                7
                8
                9
            """))
        run(cmd, 'out a b c')
        stdout = """
        a.out:1
        a.out:2
        b.out:4
        c.out:7
        c.out:8
        c.out:9
        """
        assert rm_whitespace(stdout) == shell.run('grep ".*" a.out b.out c.out')

def test_basic2():
    cmd = os.path.abspath('dedupe')
    with shell.tempdir():
        with open('a', 'w') as f:
            f.write(rm_whitespace("""
                1
            """))
        with open('b', 'w') as f:
            f.write(rm_whitespace("""
                2
            """))
        with open('c', 'w') as f:
            f.write(rm_whitespace("""
                1
                2
                3
            """))
        run(cmd, 'out a b c')
        stdout = """
        c.out:3
        """
        assert rm_whitespace(stdout) == shell.run('grep ".*" a.out b.out c.out')
