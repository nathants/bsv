#!/bin/bash
set -eou pipefail
cd $(dirname $(realpath $0))

echo ".PHONY: all clean test" > Makefile
echo "CFLAGS=-Iutil -Wall -O3 -march=native -mtune=native" >> Makefile
echo ALL=$(for src in {src,util}/*.c; do basename $src | cut -d. -f1; done) >> Makefile
echo >> Makefile

echo "all: \$(ALL)" >> Makefile
echo >> Makefile

echo clean: >> Makefile
echo -e '\tcd bin && rm -f -- $(ALL)' >> Makefile
echo >> Makefile

echo test: >> Makefile
echo -e '\tpy.test -vx --tb native test/*.py' >> Makefile
echo >> Makefile

f() {
    for path in $1/*.c; do
        name=$(basename $path | cut -d. -f1)
        echo "$name:" >> Makefile
        echo -e "\tgcc \$(CFLAGS) $path -o bin/$name" >> Makefile
        echo >> Makefile
        if ! cat .gitignore | grep ^$name &>/dev/null; then
            echo $name >> .gitignore
        fi
    done
}

f src
f util
