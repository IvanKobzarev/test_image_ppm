#!/bin/sha
set -eux -o pipefail

rm -rf build/*

mkdir -p build
pushd build
cmake -H.. -B.
make
popd

