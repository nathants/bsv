import shell
import os
from test_util import clone_source

def setup_module(m):
    m.tempdir = clone_source()
    m.orig = os.getcwd()
    m.path = os.environ['PATH']
    os.chdir(m.tempdir)
    os.environ['PATH'] = f'{os.getcwd()}/bin:/usr/bin:/usr/local/bin:/sbin:/usr/sbin:/bin'
    shell.run('make clean && make xxh3', stream=True)

def teardown_module(m):
    os.chdir(m.orig)
    os.environ['PATH'] = m.path
    assert m.tempdir.startswith('/tmp/') or m.tempdir.startswith('/private/var/folders/')
    shell.run('rm -rf', m.tempdir)

def test_hex():
    assert '079364cbfdf9f4cb' == shell.run('echo abc | xxh3')

def test_int():
    assert '545890807144117451' == shell.run('echo abc | xxh3 --int')

def test_stream():
    assert 'abc' == shell.run('echo abc | xxh3 --stream')
    assert '079364cbfdf9f4cb' == shell.run('echo abc | xxh3 --stream 2>&1 1>/dev/null')
