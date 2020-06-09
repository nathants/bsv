#!/bin/bash
set -eou pipefail
cd $(dirname $(realpath $0))

echo ".PHONY: all clean test" > Makefile
echo "CFLAGS=-Wno-incompatible-pointer-types -Wno-discarded-qualifiers -Iutil -Ivendor -flto -O3 -march=native -mtune=native" >> Makefile
# echo "CFLAGS=-Wextra -Wall -Iutil -Ivendor -flto -O3 -march=native -mtune=native" >> Makefile
echo ALL=docs $(for src in src/*.c; do basename $src | cut -d. -f1; done) >> Makefile
echo >> Makefile

echo "all: \$(ALL)" >> Makefile
echo >> Makefile

echo setup: >> Makefile
echo -e '\tmkdir -p bin' >> Makefile
echo >> Makefile

echo clean: setup >> Makefile
echo -e '\tcd bin && rm -f -- $(ALL) *.*' >> Makefile
echo >> Makefile

echo docs: >> Makefile
echo -e '\t./readme.py' >> Makefile
echo >> Makefile

echo test: setup >> Makefile
echo -e '\ttox' >> Makefile
echo >> Makefile

for path in src/*.c; do
    name=$(basename $path | cut -d. -f1)
    echo "$name: setup" >> Makefile
    echo -e "\tgcc \$(CFLAGS) $path -o bin/$name" >> Makefile
    echo >> Makefile
    if ! cat .gitignore | grep ^$name &>/dev/null; then
        echo $name >> .gitignore
    fi
done
