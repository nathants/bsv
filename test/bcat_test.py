import os
import shell
from test_util import unindent, rm_whitespace

def setup_module():
    with shell.climb_git_root():
        shell.run('make clean && make bsv csv bcat', stream=True)

def teardown_module():
    with shell.climb_git_root():
        shell.run('make clean', stream=True)

csv = os.path.abspath('./bin/bsv')
bsv = os.path.abspath('./bin/csv')
bcat = os.path.abspath('./bin/bcat')

def test_basic():
    with shell.tempdir():
        shell.run('for char in a a b b c c; do echo $char | bsv >> $char; done')
        stdout = """
        a:a
        b:b
        c:c
        """
        assert rm_whitespace(unindent(stdout)) == shell.run(f'{bcat} --prefix --head 1 a b c')
        stdout = """
        a:a
        a:a
        b:b
        b:b
        c:c
        c:c
        """
        assert rm_whitespace(unindent(stdout)) == shell.run(f'{bcat} --prefix --head 2 a b c')
        assert rm_whitespace(unindent(stdout)) == shell.run(f'{bcat} --head 2 --prefix a b c')
        assert rm_whitespace(unindent(stdout)) == shell.run(f'{bcat} --prefix a b c')
        stdout = """
        a
        b
        c
        """
        assert rm_whitespace(unindent(stdout)) == shell.run(f'{bcat} --head 1 a b c')
        stdout = """
        a
        a
        b
        b
        c
        c
        """
        assert rm_whitespace(unindent(stdout)) == shell.run(f'{bcat} a b c')
