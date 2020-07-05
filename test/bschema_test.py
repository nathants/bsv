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
    assert '12593,12850,13107' == shell.run('echo 11,22,33 | bsv | bschema u16:a,u16:a,u16:a | csv')
    assert '1,2,3'    == shell.run('echo 1,2,3 | bsv | bschema 1,1,1 | csv')
    assert '1,2,3'    == shell.run('echo 1,2,3 | bsv | bschema 1,... | csv')
    assert '1,2'      == shell.run('echo 1,2,3 | bsv | bschema 1,1 | csv')
    assert '1,2'      == shell.run('echo 1,2,3 | bsv | bschema *,* | csv')
    assert '11,22' == shell.run('echo 11,22,33 | bsv | bschema *,* | csv')
    assert 'df,er' == shell.run('echo asdf,qwer,123 | bsv | bschema "*2,*2" | csv')
    assert 'as,qw' == shell.run('echo asdf,qwer,123 | bsv | bschema "2*,2*" | csv')
    with pytest.raises(Exception):
        shell.run('echo a,qwer,123 | bsv | bschema "2*,2*" | csv')

def test_filtering():
    assert '22\n33' == shell.run('echo -e "1\n22\n33\n" | bsv | bschema 2 --filter | csv')
    assert '12850\n13107' == shell.run('echo -e "1\n22\n33\n" | bsv | bschema u16:a --filter | csv')
    assert 'as\n12' == shell.run('echo -e "asdf\nq\n123\n" | bsv | bschema "2*" --filter | csv')
