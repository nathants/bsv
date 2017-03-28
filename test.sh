#!/bin/bash
set -e
for name in *.py; do
    py.test $name --tb=native
done
