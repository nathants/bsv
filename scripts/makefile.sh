#!/bin/bash
set -eou pipefail
cd $(dirname $(dirname $(realpath $0)))

echo ".PHONY: all clean test" > Makefile
echo "CFLAGS=-Wno-int-conversion -Wno-incompatible-pointer-types -Wno-discarded-qualifiers -Iutil -Ivendor -flto -O3 -march=native -mtune=native" >> Makefile
echo ALL=clean docs $(for src in src/*.c; do
                    if basename $src | grep ^_ &>/dev/null; then
                        basename $src | cut -d. -f1
                    else
                        basename $src | cut -d. -f1 | tr '_' '-'
                    fi
                done) >> Makefile
echo >> Makefile

echo "all: \$(ALL)" >> Makefile
echo >> Makefile

echo setup: >> Makefile
echo -e '\tmkdir -p bin' >> Makefile
echo -e '\t./scripts/version.sh' >> Makefile
echo >> Makefile

echo clean: setup >> Makefile
echo -e '\tcd bin && rm -f -- * *.*' >> Makefile
echo >> Makefile

echo docs: >> Makefile
echo -e '\t./scripts/readme.py' >> Makefile
echo >> Makefile

echo test: setup >> Makefile
echo -e '\ttox' >> Makefile
echo >> Makefile

for path in src/*.c; do
    if basename $path | grep ^_ &>/dev/null; then
        name=$(basename $path | cut -d. -f1)
    else
        name=$(basename $path | cut -d. -f1 | tr '_' '-')
    fi
    echo "$name: setup" >> Makefile
    echo -e "\tgcc \$(CFLAGS) vendor/lz4.c $path -o bin/$name" >> Makefile

    echo >> Makefile
    if ! cat .gitignore | grep ^$name$ &>/dev/null; then
        echo $name >> .gitignore
    fi
done
