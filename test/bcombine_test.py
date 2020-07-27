import pytest
import os
import shell
from test_util import run, rm_whitespace, rm_whitespace, clone_source

def setup_module(m):
    m.tempdir = clone_source()
    m.orig = os.getcwd()
    m.path = os.environ['PATH']
    os.chdir(m.tempdir)
    os.environ['PATH'] = f'{os.getcwd()}/bin:/usr/bin:/usr/local/bin:/sbin:/usr/sbin:/bin'
    shell.run('make clean && make bsv csv bcombine', stream=True)

def teardown_module(m):
    os.chdir(m.orig)
    os.environ['PATH'] = m.path
    assert m.tempdir.startswith('/tmp/') or m.tempdir.startswith('/private/var/folders/')
    shell.run('rm -rf', m.tempdir)

def test_basic1():
    stdin = """
    a,b,c,d
    1,2,3
    x,y
    """
    stdout = """
    a:b,a,b,c,d
    1:2,1,2,3
    x:y,x,y
    """
    assert rm_whitespace(stdout) + '\n' == run(rm_whitespace(stdin), 'bsv | bcombine 1,2 | csv')

def test_basic2():
    stdin = """
    a,b,c,d
    1,2,3
    x,y
    """
    stdout = """
    b:a,a,b,c,d
    2:1,1,2,3
    y:x,x,y
    """
    assert rm_whitespace(stdout) + '\n' == run(rm_whitespace(stdin), 'bsv | bcombine 2,1 | csv')

def test_basic3():
    stdin = """
    a,b,c,d
    1,2,3
    x
    """
    with pytest.raises(Exception):
        run(rm_whitespace(stdin), 'bsv | bcombine 2,1 | csv')
