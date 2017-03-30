import pytest
import itertools
import os
import string
from hypothesis import given, settings
from hypothesis.strategies import text, characters, lists, composite, integers, sampled_from
import shell
import hashlib

tmp = os.environ.get('TMP_DIR', '/tmp').rstrip('/')
stdinpath = '%s/%s.stdin' % (tmp, hashlib.md5(__file__.encode('ascii')).hexdigest())
stdoutpath = '%s/%s.stdout' % (tmp, hashlib.md5(__file__.encode('ascii')).hexdigest())

MAX_COLUMNS = 64

def rm_whitespace(x):
    return '\n'.join([y.strip().replace(' ', '')
                      for y in x.splitlines()
                      if y.strip()])

def run(*args):
    try:
        shell.run(*(args + ('>', stdoutpath, '2>/dev/null')))
    except:
        raise AssertionError from None
    with open(stdoutpath) as f:
        return f.read()

buffers = [8, 11, 17, 64, 256, 1024]

shell.run('mv reads.c reads.c.bak')
shell.run('make clean')
for i in buffers:
    shell.run('cat reads.c.bak | sed -r "s/#define READS_BUFFER_SIZE.*/#define READS_BUFFER_SIZE %s/" > reads.c' % i)
    print(shell.run('cat reads.c|grep define'))
    shell.run('make reads')
    shell.run('mv reads reads.%s' % i, stream=True)
shell.run('mv -f reads.c.bak reads.c')

@composite
def inputs(draw):
    buffer = draw(sampled_from(buffers))
    line = text(string.ascii_lowercase, min_size=1, max_size=min(64, buffer - 1))
    lines = lists(line, min_size=1)
    files = lists(lines, min_size=1)
    cmd = './reads.%s' % buffer
    return cmd, draw(files)

def expected(files):
    res = []
    for xs in itertools.zip_longest(*files, fillvalue=''):
        for x in xs:
            if x:
                res.append(x)
    return '\n'.join(res) + '\n'

@given(inputs())
@settings(max_examples=100 * int(os.environ.get('TEST_FACTOR', 1)))
def test_props(arg):
    cmd, files = arg
    cmd = os.path.abspath(cmd)
    paths = []
    with shell.tempdir():
        for i, file in enumerate(files):
            paths.append(str(i))
            with open(paths[-1], 'w') as f:
                for line in file:
                    f.write(line + '\n')
        assert expected(files) == run(cmd, *paths)

def test_basic():
    cmd = os.path.abspath('reads.8')
    with shell.tempdir():
        with open('a.txt', 'w') as f:
            f.write(rm_whitespace("""
            a
            b
            """))
        with open('b.txt', 'w') as f:
            f.write(rm_whitespace("""
            1
            2
            3
            """))
        with open('c.txt', 'w') as f:
            f.write(rm_whitespace("""
            Q
            W
            E
            R
            """))
        args = [
            'a.txt',
            'b.txt',
            'c.txt',
        ]
        stdout = rm_whitespace("""
        a
        1
        Q
        b
        2
        W
        3
        E
        R
        """)
        assert stdout == run(cmd, *args).strip()

# def test_oversized_line():
#     stdin = 'a' * 8
#     res = shell.run('./reads.8 2>&1', stdin=stdin, warn=True)
#     assert res['exitcode'] == 1
#     assert 'error: line longer than READS_BUFFER_SIZE' == res['output']
