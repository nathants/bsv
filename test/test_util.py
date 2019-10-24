import hashlib
import shell
import os

with shell.climb_git_root():
    max_columns = int(shell.run('cat util/util.h | grep "define MAX_COLUMNS"').split()[-1])

def run(stdin, *args):
    with shell.climb_git_root():
        tmp = os.environ.get('TMP_DIR', '/tmp').rstrip('/')
        stdinpath = '%s/%s.stdin' % (tmp, hashlib.md5(__file__.encode('ascii')).hexdigest())
        stdoutpath = '%s/%s.stdout' % (tmp, hashlib.md5(__file__.encode('ascii')).hexdigest())
        with open(stdinpath, 'w') as f:
            f.write(stdin)
        shell.run(*(('set -o pipefail; cat', stdinpath, '|') + args + ('>', stdoutpath)), stream=True)
        with open(stdoutpath) as f:
            return f.read()

def runb(stdin, *args):
    with shell.climb_git_root():
        tmp = os.environ.get('TMP_DIR', '/tmp').rstrip('/')
        stdinpath = '%s/%s.stdin' % (tmp, hashlib.md5(__file__.encode('ascii')).hexdigest())
        stdoutpath = '%s/%s.stdout' % (tmp, hashlib.md5(__file__.encode('ascii')).hexdigest())
        with open(stdinpath, 'w') as f:
            f.write(stdin)
        shell.run(*(('set -o pipefail; cat', stdinpath, '|') + args + ('>', stdoutpath)), stream=True)
        with open(stdoutpath, 'rb') as f:
            return f.read()

def unindent(text):
    return '\n'.join([x.lstrip() for x in text.splitlines()]) + '\n'

def rm_whitespace(x):
    return '\n'.join([y.strip().replace(' ', '') for y in x.splitlines() if y.strip()])

def compile_buffer_sizes(name, buffers):
    with shell.climb_git_root():
        shell.run('cp -f util/util.h util/util.h.bak')
        try:
            for i in buffers:
                shell.run(f'cat util/util.h.bak | sed -r "s/#define BUFFER_SIZE.*/#define BUFFER_SIZE {i}/" > util/util.h')
                print('compile:', name, i, shell.run('cat util/util.h | grep "define BUFFER_SIZE"'), flush=True)
                shell.run('make', name)
                shell.run(f'mv -f bin/{name} bin/{name}.{i}')
        finally:
            shell.run('cat util/util.h.bak > util/util.h')
            shell.run('rm -f util/util.h.bak')
