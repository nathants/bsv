#!/bin/bash
set -euo pipefail

cd $(dirname $(dirname $0))

hash=$(git log -1 --pretty=%H || echo -)
date=$(date -u +'%Y-%m-%dT%H:%M:%SZ')

if [ -z "$(git status --porcelain)" ]; then
    devel=false
else
    devel=true
fi

cat - <<EOF >util/version.h
#define VERSION_GIT_HASH "git:      $hash"
#define VERSION_DATE     "date:     $date"
#define VERSION_DEVEL    "devel:    $devel"
EOF
