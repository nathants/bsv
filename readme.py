#!/usr/bin/env python3
import os
os.chdir(os.path.dirname(__file__))
co = lambda *a: __import__('subprocess').check_output(' '.join(map(str, a)), shell=True, executable='/bin/bash').decode('utf-8').strip()

with open('readme.md') as f:
    xs = f.read().splitlines()

before = []
for x in xs:
    before.append(x)
    if x.startswith('## utilities'):
        before.append('')
        break

after = []

for path in co('ls src/*.c').splitlines():
    if not path.split('/')[-1].startswith('_'):
        with open(path) as f:
            xs = f.read().splitlines()
        try:
            assert any(x.strip() == 'HELP();' for x in xs), path
            name = path.split('/')[-1].split('.c')[0]
            description = [x for x in xs if x.startswith('#define DESCRIPTION')][0].replace('\\n', '\n').split('"')[1]
            usage = [x for x in xs if x.startswith('#define USAGE')][0].replace('\\n', '\n').split('"')[1]
            try:
                example = [x for x in xs if x.startswith('#define EXAMPLE')][0].replace('\\n', '\n').split('"')[1]
            except IndexError:
                example = ''
                while True:
                    x = xs.pop(0)
                    if x.startswith('#define EXAMPLE'):
                        break
                while True:
                    x = xs.pop(0)
                    if not x.strip():
                        break
                    example += x.replace('\\n', '\n').split('"')[1]
        except:
            print(f'fatal: failed to parse docs in file: {name}.c')
            raise
        before.append(f'- [{name}](#{name}) - {description}'.strip())
        after.append(f'\n### [{name}](https://github.com/nathants/bsv/blob/master/src/{name}.c)\n\n{description}usage: `{usage.strip()}`\n\n```\n{example.strip()}\n```')

with open('readme.md', 'w') as f:
    f.write('\n'.join(before + after))
