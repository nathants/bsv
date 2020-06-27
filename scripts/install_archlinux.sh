#!/bin/bash
set -euo pipefail
tempdir=$(mktemp -d)
trap "cd /tmp && rm -rf $tempdir" EXIT
cd $tempdir
git clone https://github.com/nathants/bsv
cd bsv
make -j
sudo mv -fv bin/* /usr/local/bin
