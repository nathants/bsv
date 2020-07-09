import os
import shell
from test_util import run, rm_whitespace, rm_whitespace, clone_source

def setup_module(m):
    m.tempdir = clone_source()
    m.orig = os.getcwd()
    m.path = os.environ['PATH']
    os.chdir(m.tempdir)
    os.environ['PATH'] = f'{os.getcwd()}/bin:/usr/bin:/usr/local/bin:/sbin:/usr/sbin:/bin'
    shell.run('make clean && make bsv csv bsort bschema bsumeach-hash-i64', stream=True)

def teardown_module(m):
    os.chdir(m.orig)
    os.environ['PATH'] = m.path
    assert m.tempdir.startswith('/tmp/') or m.tempdir.startswith('/private/var/folders/')
    shell.run('rm -rf', m.tempdir)

def test_basic():
    stdin = """
    a,1
    a,2
    a,3
    b,4
    b,5
    a,6
    """
    stdout = """
    a,12
    b,9
    """
    assert rm_whitespace(stdout) + '\n' == run(rm_whitespace(stdin), 'bsv | bschema *,a:i64 | bsumeach-hash-i64 | bschema *,i64:a | bsort | csv')
