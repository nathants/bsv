import os
import shell
from test_util import unindent, rm_whitespace, clone_source

def setup_module(m):
    m.tempdir = clone_source()
    m.orig = os.getcwd()
    m.path = os.environ['PATH']
    os.chdir(m.tempdir)
    os.environ['PATH'] = f'{os.getcwd()}/bin:/usr/bin:/usr/local/bin'
    shell.run('make clean && make bsv csv bcat', stream=True)

def teardown_module(m):
    os.chdir(m.orig)
    os.environ['PATH'] = m.path
    assert m.tempdir.startswith('/tmp/')
    shell.run('rm -rf', m.tempdir)

def test_basic():
    with shell.tempdir():
        shell.run('for char in a a b b c c; do echo $char | bsv >> $char; done')
        stdout = """
        a:a
        b:b
        c:c
        """
        assert rm_whitespace(unindent(stdout)) == shell.run(f'bcat --prefix --head 1 a b c')
        stdout = """
        a:a
        a:a
        b:b
        b:b
        c:c
        c:c
        """
        assert rm_whitespace(unindent(stdout)) == shell.run(f'bcat --prefix --head 2 a b c')
        assert rm_whitespace(unindent(stdout)) == shell.run(f'bcat --head 2 --prefix a b c')
        assert rm_whitespace(unindent(stdout)) == shell.run(f'bcat --prefix a b c')
        stdout = """
        a
        b
        c
        """
        assert rm_whitespace(unindent(stdout)) == shell.run(f'bcat --head 1 a b c')
        stdout = """
        a
        a
        b
        b
        c
        c
        """
        assert rm_whitespace(unindent(stdout)) == shell.run(f'bcat a b c')
