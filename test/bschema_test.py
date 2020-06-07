import os
import pytest
import shell
from test_util import clone_source

def setup_module(m):
    m.tempdir = clone_source()
    m.orig = os.getcwd()
    m.path = os.environ['PATH']
    os.chdir(m.tempdir)
    os.environ['PATH'] = f'{os.getcwd()}/bin:/usr/bin:/usr/local/bin:/sbin:/usr/sbin:/bin'
    shell.run('make clean && make bsv csv bschema', stream=True)

def teardown_module(m):
    os.chdir(m.orig)
    os.environ['PATH'] = m.path
    assert m.tempdir.startswith('/tmp/') or m.tempdir.startswith('/private/var/folders/')
    shell.run('rm -rf', m.tempdir)

def test_basic():
    with pytest.raises(Exception):
        shell.run('echo 1,2,3 | bsv | bschema 1,1,1,1 | csv')
    with pytest.raises(Exception):
        shell.run('echo 1,2,3 | bsv | bschema 1,2,1 | csv')
    assert '49,50,51' == shell.run('echo 1,2,3 | bsv | bschema u16:a,u16:a,u16:a | csv')
    assert '1,2,3'    == shell.run('echo 1,2,3 | bsv | bschema 1,1,1 | csv')
    assert '1,2,3'    == shell.run('echo 1,2,3 | bsv | bschema 1,... | csv')
    assert '1,2'      == shell.run('echo 1,2,3 | bsv | bschema 1,1 | csv')
    assert '1,2'      == shell.run('echo 1,2,3 | bsv | bschema *,* | csv')
