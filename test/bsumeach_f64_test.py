import os
import shell
from test_util import run, rm_whitespace, rm_whitespace, clone_source

def setup_module(m):
    m.tempdir = clone_source()
    m.orig = os.getcwd()
    m.path = os.environ['PATH']
    os.chdir(m.tempdir)
    os.environ['PATH'] = f'{os.getcwd()}/bin:/usr/bin:/usr/local/bin:/sbin:/usr/sbin:/bin'
    shell.run('make clean && make bsv csv bschema bsumeach-f64', stream=True)

def teardown_module(m):
    os.chdir(m.orig)
    os.environ['PATH'] = m.path
    assert m.tempdir.startswith('/tmp/') or m.tempdir.startswith('/private/var/folders/')
    shell.run('rm -rf', m.tempdir)

def test_basic():
    stdin = """
    a,1.1
    a,2.1
    a,3.1
    b,4.1
    b,5.1
    a,6.1
    """
    stdout = """
    a,6.300000
    b,9.200000
    a,6.100000
    """
    assert rm_whitespace(stdout) + '\n' == run(rm_whitespace(stdin), 'bsv | bschema *,a:f64 | bsumeach-f64 | bschema *,f64:a | csv')
