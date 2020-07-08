#!/bin/bash
set -eou pipefail

name=$1

if ! which aws-ec2-new &>/dev/null; then
    echo fatal: need to install https://github.com/nathants/cli-aws
    exit 1
fi

cd $(dirname $(dirname $0))

# push code
aws-ec2-rsync . :bsv/ $name -y

# reinstall bsv
aws-ec2-ssh $name -yc "
    cd ~/bsv
    make -j && sudo mv -fv bin/* /usr/local/bin
"

# kill any running reloaders
aws-ec2-ssh $name -yc "(ps -ef | grep entr | grep make | grep -v grep | awk '{print \$2}' | xargs kill) || true"

# setup the remote reloader
aws-ec2-ssh $name --no-tty -yc "
    cd ~/bsv
    ((find -type f -name '*.c' -o -name '*.h' | entr -r bash -c 'make -j && sudo mv -fv bin/* /usr/local/bin') &> ~/bsv.log </dev/null) &
"

# run the local file pusher
find -type f -name '*.c' -o -name '*.h' | entr aws-ec2-rsync . :bsv/ $name -y
