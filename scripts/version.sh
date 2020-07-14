#!/bin/bash
set -euo pipefail
cd $(dirname $(dirname $0))

hash=$(git log -1 --pretty=%H || echo -)
date=$(date -u +'%Y-%m-%dT%H:%M:%SZ')
compiler=$(gcc --version | head -n1)
arch=$(gcc -march=native -Q --help=target | grep march= | head -n1 | awk '{print $NF}')
if [ -z "$(git status --porcelain)" ]; then
    devel=false
else
    devel=true
fi

cat - <<EOF >util/version.h
#define VERSION_GIT_HASH "git:      $hash"
#define VERSION_DATE     "date:     $date"
#define VERSION_COMPILER "compiler: $compiler"
#define VERSION_ARCH     "arch:     $arch"
#define VERSION_DEVEL    "devel:    $devel"
EOF
