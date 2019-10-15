import shell

def setup_module():
    with shell.climb_git_root():
        shell.run('make clean && make xxh3', stream=True)

def teardown_module():
    with shell.climb_git_root():
        shell.run('make clean', stream=True)

def test_hex():
    assert 'b5ca312e51d77d64' == shell.run('echo abc | xxh3')

def test_int():
    assert '13099336541171842404' == shell.run('echo abc | xxh3 --int')

def test_stream():
    assert 'abc' == shell.run('echo abc | xxh3 --stream')
    assert 'b5ca312e51d77d64' == shell.run('echo abc | xxh3 --stream 2>&1 1>/dev/null')
