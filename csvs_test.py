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
        shell.run(*(args + ('>', stdoutpath, '2>/dev/null')), echo=True)
    except:
        raise AssertionError from None
    with open(stdoutpath) as f:
        return f.read()

buffers = [8, 11, 17, 64, 256, 1024]

shell.run('mv csvs.c csvs.c.bak')
shell.run('make clean')
for i in buffers:
    shell.run('cat csvs.c.bak | sed -r "s/#define CSVS_BUFFER_SIZE.*/#define CSVS_BUFFER_SIZE %s/" > csvs.c' % i)
    print(shell.run('cat csvs.c|grep define'))
    shell.run('make csvs')
    shell.run('mv csvs csvs.%s' % i, stream=True)
shell.run('mv -f csvs.c.bak csvs.c')

@composite
def inputs(draw):
    buffer = draw(sampled_from(buffers))
    num_columns = draw(integers(min_value=1, max_value=64))
    column = text(string.ascii_lowercase + ' ', min_size=1, max_size=64, average_size=16)
    line = lists(column, min_size=1, max_size=num_columns)
    lines = lists(line)
    files = lists(lines, min_size=1, max_size=3)
    csvs = ['\n'.join([','.join(x)[:buffer - 1] for x in lines]) + '\n' for lines in draw(files)]
    cmd = './csvs.%s' % buffer
    return cmd, csvs

def expected(files):
    res = []
    for xs in itertools.zip_longest(*[f.splitlines() for f in files], fillvalue=''):
        for x in xs:
            if x:
                for y in x.split(','):
                    if y:
                        res.append(y)
    if res:
        return '\n'.join(res) + '\n'
    else:
        return ''

@given(inputs())
@settings(max_examples=100 * int(os.environ.get('TEST_FACTOR', 1)), perform_health_check=False)
def test_props(arg):
    cmd, files = arg
    cmd = os.path.abspath(cmd)
    paths = []
    with shell.tempdir():
        for i, file in enumerate(files):
            paths.append(str(i))
            with open(paths[-1], 'w') as f:
                f.write(file)
        assert expected(files) == run(cmd, *paths)

def test_oversized_line():
    cmd = os.path.abspath('csvs.8')
    with shell.tempdir():
        with open('a.txt', 'w') as f:
            f.write('a' * 8)
        res = shell.run(cmd, 'a.txt 2>&1', warn=True)
        assert res['exitcode'] == 1
        assert 'error: line longer than CSVS_BUFFER_SIZE' == res['output']
