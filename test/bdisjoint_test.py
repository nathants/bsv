import os
import string
import shell
from hypothesis.database import ExampleDatabase
from hypothesis import given, settings
from hypothesis.strategies import text, lists, composite
from test_util import rm_whitespace, rm_whitespace, clone_source

def setup_module(m):
    m.tempdir = clone_source()
    m.orig = os.getcwd()
    m.path = os.environ['PATH']
    os.chdir(m.tempdir)
    os.environ['PATH'] = f'{os.getcwd()}/bin:/usr/bin:/usr/local/bin'
    shell.run('make clean && make bsv csv bdisjoint', stream=True)

def teardown_module(m):
    os.chdir(m.orig)
    os.environ['PATH'] = m.path
    assert m.tempdir.startswith('/tmp/')
    shell.run('rm -rf', m.tempdir)

@composite
def inputs(draw):
    line = text(string.ascii_letters, max_size=50)
    lines = lists(line)
    files = lists(lines, min_size=2, max_size=5)
    return {str(i): sorted(x) for i, x in enumerate(draw(files))}

def disjoint(xs):
    seen = set()
    for x in xs:
        if x not in seen:
            seen.add(x)
            yield x

def expected(inputs):
    uniqs = {i: set(x) for i, x in inputs.items()}
    res = {i: [y for y in disjoint(x) if y not in others]
           for i, x in inputs.items()
           for others in [set.union(*[z
                                      for j, z in uniqs.items()
                                      if j != i])]}
    return '\n'.join([f'{path}.suffix:{line}'
                      for path in sorted(res)
                      for line in res.get(path)
                      if line])

@given(inputs())
@settings(database=ExampleDatabase(':memory:'), max_examples=50 * int(os.environ.get('TEST_FACTOR', 1)), deadline=os.environ.get("TEST_DEADLINE", 1000 * 60))
def test_props(inputs):
    with shell.tempdir():
        result = expected(inputs)
        for path, lines in inputs.items():
            with open(f'{path}.csv', 'w') as f:
                f.write('\n'.join(lines) + '\n')
            shell.run(f'cat {path}.csv | bsv > {path}')
        shell.run('bdisjoint suffix.csv', *inputs, echo=True)
        for path in shell.run('ls').splitlines():
            if path.endswith('.suffix.csv'):
                shell.run(f'cat {path} | csv > {path.split(".csv")[0]}')
        assert result == shell.run('(grep --with-filename ".*" *.suffix || true) | LC_ALL=C sort')

def test_basic():
    with shell.tempdir(cleanup=False):
        with open('a.csv', 'w') as f:
            f.write(rm_whitespace("""
                1
                2
                3
            """))
        shell.run('cat a.csv | bsv > a')
        with open('b.csv', 'w') as f:
            f.write(rm_whitespace("""
                3
                4
                5
                6
            """))
        shell.run('cat b.csv | bsv > b')
        with open('c.csv', 'w') as f:
            f.write(rm_whitespace("""
                5
                6
                7
                8
                9
            """))
        shell.run('cat c.csv | bsv > c')
        shell.run('bdisjoint out a b c', stream=True)
        stdout = """
        a.out.csv:1
        a.out.csv:2
        b.out.csv:4
        c.out.csv:7
        c.out.csv:8
        c.out.csv:9
        """
        shell.run(f'cat a.out | csv > a.out.csv')
        shell.run(f'cat b.out | csv > b.out.csv')
        shell.run(f'cat c.out | csv > c.out.csv')
        assert rm_whitespace(stdout) == shell.run(f'grep ".*" *.out.csv')

def test_basic2():
    with shell.tempdir(cleanup=False):
        with open('a.csv', 'w') as f:
            f.write(rm_whitespace("""
                1
            """))
        shell.run('cat a.csv | bsv > a')
        with open('b.csv', 'w') as f:
            f.write(rm_whitespace("""
                2
            """))
        shell.run('cat b.csv | bsv > b')
        with open('c.csv', 'w') as f:
            f.write(rm_whitespace("""
                1
                2
                3
            """))
        shell.run('cat c.csv | bsv > c')
        shell.run('bdisjoint out a b c', stream=True)
        stdout = """
        c.out.csv:3
        """
        assert ['c.out'] == shell.run('ls *.out').splitlines()
        shell.run(f'cat c.out | csv > c.out.csv')
        assert rm_whitespace(stdout) == shell.run('grep -H ".*" *.out.csv')

def test_dedupes():
    with shell.tempdir(cleanup=False):
        with open('a.csv', 'w') as f:
            f.write(rm_whitespace("""
                1
                1
                1
            """))
        shell.run('cat a.csv | bsv > a')
        with open('b.csv', 'w') as f:
            f.write(rm_whitespace("""
                2
            """))
        shell.run('cat b.csv | bsv > b')
        with open('c.csv', 'w') as f:
            f.write(rm_whitespace("""
                1
                2
                3
                3
                3
            """))
        shell.run('cat c.csv | bsv > c')
        shell.run('bdisjoint out a b c', stream=True)
        stdout = """
        c.out.csv:3
        """
        assert ['c.out'] == shell.run('ls *.out').splitlines()
        shell.run(f'cat c.out | csv > c.out.csv')
        assert rm_whitespace(stdout) == shell.run('grep -H ".*" *.out.csv')
