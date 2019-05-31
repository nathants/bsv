import shell
from test_util import run, rm_whitespace, rm_whitespace

# TODO generative test multiple buffer sizes
# TODO use python bsv/csv to avoid extra shelling out in gen tests

def setup_module():
    with shell.climb_git_root():
        shell.run('make clean && make csv bsv', stream=True)

def test_max_bytes():
    stdin = 'a' * (2**16 - 1)
    assert len(stdin.strip()) == len(run(stdin, 'bin/bsv | bin/csv').strip())

    stdin = 'a' * (2**16)
    with shell.climb_git_root():
        res = shell.run('bin/bsv', stdin=stdin, warn=True)
    assert 'error: cannot have columns with more than 2**16 bytes, column: 0,size: 65536, content: aaaaaaaaaa...' == res['stderr']
    assert res['exitcode'] == 1

def test_encoding():
    stdin = """
    aa,bbb
    xyz
    """
    assert rm_whitespace(stdin) + '\n' == run(rm_whitespace(stdin), 'bin/bsv | bin/csv')

    val = run(rm_whitespace(stdin), 'bin/bsv')
    val = bytes(val, 'utf-8')
    bsv = b''                                                           # ushort:max-index; ushort:size1, ...; bytes:chars
    bsv += b'\x01\x00'            + b'\x02\x00\x03\x00'     + b'aabbb'  # 1;2,3;aabbb
    bsv += b'\x00\x00'            + b'\x03\x00'             + b'xyz'    # 0;3;xyz
    assert bsv == val
