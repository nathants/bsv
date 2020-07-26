#!/bin/bash
set -euo pipefail
sudo pacman --needed --noconfirm --noprogressbar -Sy \
     gcc \
     make
cd ~
rm -rf bsv
git clone https://github.com/nathants/bsv
cd bsv
make -j
sudo mv -fv bin/* /usr/local/bin
