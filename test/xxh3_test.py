import shell
import io
import os
import xxh3
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
    assert '079364cbfdf9f4cb' == xxh3.oneshot_hex('abc\n'.encode())

def test_int():
    assert '545890807144117451' == shell.run('echo abc | xxh3 --int')
    assert 545890807144117451 == xxh3.oneshot_int('abc\n'.encode())

def test_stream():
    assert {
        'cmd': 'set -eou pipefail; echo abc | xxh3 --stream',
        'exitcode': 0,
        'stderr': '079364cbfdf9f4cb',
        'stdout': 'abc',
    } == shell.run('echo abc | xxh3 --stream', warn=True)
    assert '079364cbfdf9f4cb' == xxh3.stream_hex(io.BytesIO('abc\n'.encode()))
    assert {
        'cmd': 'set -eou pipefail; echo abc | xxh3 --stream --int',
        'exitcode': 0,
        'stderr': '545890807144117451',
        'stdout': 'abc',
    } == shell.run('echo abc | xxh3 --stream --int', warn=True)
    assert 545890807144117451 == xxh3.stream_int(io.BytesIO('abc\n'.encode()))
