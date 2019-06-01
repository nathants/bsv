import os
import string
import shell
import hypothesis
from hypothesis import given, settings
from hypothesis.strategies import text, lists, composite, integers
from test_util import run, rm_whitespace, rm_whitespace, max_columns

def setup_module():
    with shell.climb_git_root():
        shell.run('make clean && make bsv csv bdisjoint', stream=True)

def teardown_module():
    with shell.climb_git_root():
        shell.run('rm *.out', stream=True)

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
@settings(max_examples=50 * int(os.environ.get('TEST_FACTOR', 1)), timeout=hypothesis.unlimited, suppress_health_check=[hypothesis.HealthCheck.hung_test])
def test_props(inputs):
    cmd = os.path.abspath('bin/bdisjoint')
    bsv = os.path.abspath('bin/bsv')
    csv = os.path.abspath('bin/csv')
    with shell.tempdir():
        result = expected(inputs)
        for path, lines in inputs.items():
            with open(f'{path}.csv', 'w') as f:
                f.write('\n'.join(lines) + '\n')
            shell.run(f'cat {path}.csv | {bsv} > {path}')
        shell.run(cmd, 'suffix.csv', *inputs)
        for path in shell.run('ls').splitlines():
            if path.endswith('.suffix.csv'):
                shell.run(f'cat {path} | {csv} > {path.split(".csv")[0]}')
        assert result == shell.run('(grep --with-filename ".*" *.suffix || true) | LC_ALL=C sort')

def test_basic():
    bsv = os.path.abspath('bin/bsv')
    with shell.tempdir(cleanup=False):
        with open('a.csv', 'w') as f:
            f.write(rm_whitespace("""
                1
                2
                3
            """))
        a = os.path.abspath('a')
        shell.run('cat a.csv |', bsv, '> a')
        with open('b.csv', 'w') as f:
            f.write(rm_whitespace("""
                3
                4
                5
                6
            """))
        shell.run('cat b.csv |', bsv, '> b')
        b = os.path.abspath('b')
        with open('c.csv', 'w') as f:
            f.write(rm_whitespace("""
                5
                6
                7
                8
                9
            """))
        shell.run('cat c.csv |', bsv, '> c')
        c = os.path.abspath('c')
    try:
        shell.run('bin/bdisjoint out', a, b, c, stream=True)
        stdout = """
        a.out:1
        a.out:2
        b.out:4
        c.out:7
        c.out:8
        c.out:9
        """
        shell.run(f'cat {a}.out | csv > a.out')
        shell.run(f'cat {b}.out | csv > b.out')
        shell.run(f'cat {c}.out | csv > c.out')
        assert rm_whitespace(stdout) == shell.run(f'grep ".*" a.out b.out c.out')
    finally:
        shell.run('rm', a, b, c)

def test_basic2():
    bsv = os.path.abspath('bin/bsv')
    with shell.tempdir(cleanup=False):
        with open('a.csv', 'w') as f:
            f.write(rm_whitespace("""
                1
            """))
        a = os.path.abspath('a')
        shell.run('cat a.csv |', bsv, '> a')
        with open('b.csv', 'w') as f:
            f.write(rm_whitespace("""
                2
            """))
        shell.run('cat b.csv |', bsv, '> b')
        b = os.path.abspath('b')
        with open('c.csv', 'w') as f:
            f.write(rm_whitespace("""
                1
                2
                3
            """))
        shell.run('cat c.csv |', bsv, '> c')
        c = os.path.abspath('c')
    try:
        shell.run('bin/bdisjoint out', a, b, c, stream=True)
        stdout = """
        a.out:
        b.out:
        c.out:3
        """
        shell.run(f'cat {a}.out | csv > a.out')
        shell.run(f'cat {b}.out | csv > b.out')
        shell.run(f'cat {c}.out | csv > c.out')
        assert rm_whitespace(stdout) == shell.run('grep ".*" a.out b.out c.out')
    finally:
        shell.run('rm', a, b, c)
