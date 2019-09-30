import shell

def setup_module():
    with shell.climb_git_root():
        shell.run('make clean && make xxh3', stream=True)

def teardown_module():
    with shell.climb_git_root():
        shell.run('make clean', stream=True)

def test_hex():
    assert 'B5CA312E51D77D64' == shell.run('echo abc | xxh3 2>&1')

def test_int():
    assert '13099336541171842404' == shell.run('echo abc | xxh3 --int 2>&1')

def test_stream():
    assert 'abc' == shell.run('echo abc | xxh3 --stream')
    assert 'B5CA312E51D77D64' == shell.run('echo abc | xxh3 --stream 2>&1 1>/dev/null')
