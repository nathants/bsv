import shell
import hashlib

stdinpath = '/tmp/%s.stdin' % hashlib.md5(__file__.encode('ascii')).hexdigest()
stdoutpath = '/tmp/%s.stdout' % hashlib.md5(__file__.encode('ascii')).hexdigest()

def unindent(text):
    return '\n'.join([x.lstrip() for x in text.splitlines()])

def run(stdin, *args):
    with open(stdinpath, 'w') as f:
        f.write(unindent(stdin))
    try:
        shell.run(*(('cat', stdinpath, '|') + args + ('>', stdoutpath)), stream=True)
    except:
        raise AssertionError from None
    with open(stdoutpath) as f:
        return f.read()

shell.run('make', stream=True)

def test_compatability():
    stdin = """
    a,b,c,d
    1,2,3
    x,y
    """
    stdout = """
    a,b
    1,2
    x,y
    """
    assert unindent(stdout) == run(stdin, './rcut , 1,2')
    assert unindent(stdout) == run(stdin, 'cut -d, -f1,2')


def test_single_column():
    stdin = """
    x,y
    1,2,3
    a,b,c,d
    """
    stdout = """
    x
    1
    a
    """
    assert unindent(stdout) == run(stdin, './rcut , 1')
    stdin = """
    a,b,c,d
    1,2,3
    x,y
    """
    stdout = """
    a
    1
    x
    """
    assert unindent(stdout) == run(stdin, './rcut , 1')
    stdin = """
    a,b,c,d
    1,2,3
    x,y
    """
    stdout = """
    b
    2
    y
    """
    assert unindent(stdout) == run(stdin, './rcut , 2')


def test_forward():
    stdin = """
    a,b,c,d
    1,2,3
    x,y
    """
    stdout = """
    a,b
    1,2
    x,y
    """
    assert unindent(stdout) == run(stdin, './rcut , 1,2')
    stdin = """
    a,b,c,d
    1,2,3
    x,y
    """
    stdout = """
    a,c
    1,3
    x
    """
    assert unindent(stdout) == run(stdin, './rcut , 1,3')
    stdin = """
    x,y
    1,2,3
    a,b,c,d
    """
    stdout = """
    x
    1,3
    a,c
    """
    assert unindent(stdout) == run(stdin, './rcut , 1,3')


def test_reverse():
    stdin = """
    a,b,c,d
    1,2,3
    x,y
    """
    stdout = """
    b,a
    2,1
    y,x
    """
    assert unindent(stdout) == run(stdin, './rcut , 2,1')
    stdin = """
    a,b,c,d
    1,2,3
    x,y
    """
    stdout = """
    c,a
    3,1
    x
    """
    assert unindent(stdout) == run(stdin, './rcut , 3,1')
    stdin = """
    x,y
    1,2,3
    a,b,c,d
    """
    stdout = """
    x
    3,1
    c,a
    """
    assert unindent(stdout) == run(stdin, './rcut , 3,1')

def test_holes():
    stdin = """
    a,b,c,d
    1,2,3
    x,y
    """
    stdout = """
    a,c,b
    1,3,2
    x,y
    """
    assert unindent(stdout) == run(stdin, './rcut , 1,3,2')
