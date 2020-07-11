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
    assert '5' == shell.run('echo 5 | bsv | bschema a:u16 | bschema u16:a | csv')
    assert '5' == shell.run('echo 5 | bsv | bschema a:u32 | bschema u32:a | csv')
    assert '5' == shell.run('echo 5 | bsv | bschema a:u64 | bschema u64:a | csv')
    assert '5' == shell.run('echo 5 | bsv | bschema a:i16 | bschema i16:a | csv')
    assert '5' == shell.run('echo 5 | bsv | bschema a:i32 | bschema i32:a | csv')
    assert '5' == shell.run('echo 5 | bsv | bschema a:i64 | bschema i64:a | csv')
    assert '5' == shell.run('echo 5 | bsv | bschema a:f32 | bschema f32:a | csv').split('.')[0]
    assert '5' == shell.run('echo 5 | bsv | bschema a:f64 | bschema f64:a | csv').split('.')[0]
    assert '1' == shell.run('echo 1 | bsv | bschema 1,... | csv')
    assert '1,2' == shell.run('echo 1,2,3 | bsv | bschema 1,1,... | csv')
    with pytest.raises(Exception):
        shell.run('echo 1,2,3 | bsv | bschema fake,schema,errors | csv')
    with pytest.raises(Exception):
        shell.run('echo 1,2,3 | bsv | bschema 1,1 | csv')
    with pytest.raises(Exception):
        shell.run('echo 1,2,3 | bsv | bschema 1,1,1,1 | csv')
    with pytest.raises(Exception):
        shell.run('echo 1,2,3 | bsv | bschema 1,2,1 | csv')
    assert '12593,12850,13107' == shell.run('echo 11,22,33 | bsv | bschema u16:a,u16:a,u16:a | csv')
    assert '1,2,3'    == shell.run('echo 1,2,3 | bsv | bschema 1,1,1 | csv')
    assert '1'    == shell.run('echo 1,2,3 | bsv | bschema 1,... | csv')
    assert '1,2'      == shell.run('echo 1,2,3 | bsv | bschema *,*,... | csv')
    assert '11,22' == shell.run('echo 11,22,33 | bsv | bschema *,*,... | csv')
    assert 'df,er' == shell.run('echo asdf,qwer | bsv | bschema "*2,*2" | csv')
    assert 'as,qw' == shell.run('echo asdf,qwer | bsv | bschema "2*,2*" | csv')
    with pytest.raises(Exception):
        shell.run('echo a,qwer,123 | bsv | bschema "2*,2*" | csv')
    with pytest.raises(Exception):
        shell.run('echo -1 | bsv | bschema "a:u64" | csv')

def test_filtering():
    assert '1,1\n2,2' == shell.run('echo -e "1,1\n2,2\n3\n" | bsv | bschema 1,1 --filter | csv')
    assert '22\n33' == shell.run('echo -e "1\n22\n33\n" | bsv | bschema 2 --filter | csv')
    assert '12850\n13107' == shell.run('echo -e "1\n22\n33\n" | bsv | bschema u16:a --filter | csv')
    assert 'as\n12' == shell.run('echo -e "asdf\nq\n123\n" | bsv | bschema "2*" --filter | csv')

def test_maxint():
    with pytest.raises(Exception):
        shell.run('echo 32768 | bsv | bschema a:i16')
    with pytest.raises(Exception):
        shell.run('echo -32769 | bsv | bschema a:i16')
    with pytest.raises(Exception):
        shell.run('echo -1 | bsv | bschema a:u16')
    with pytest.raises(Exception):
        shell.run('echo 65536 | bsv | bschema a:u16')
    with pytest.raises(Exception):
        shell.run('echo 2147483648 | bsv | bschema a:i32')
    with pytest.raises(Exception):
        shell.run('echo -2147483649 | bsv | bschema a:i32')
    with pytest.raises(Exception):
        shell.run('echo -1 | bsv | bschema a:u32')
    with pytest.raises(Exception):
        shell.run('echo 4294967296 | bsv | bschema a:u32')
    with pytest.raises(Exception):
        shell.run('echo -9223372036854775808 | bsv | bschema a:i64')
    with pytest.raises(Exception):
        shell.run('echo 9223372036854775807 | bsv | bschema a:i64')
    with pytest.raises(Exception):
        shell.run('echo -1 | bsv | bschema a:u64')
    with pytest.raises(Exception):
        shell.run('echo 18446744073709551615 | bsv | bschema a:u64')
