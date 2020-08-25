#!/bin/sh

rm -rf ./dist
cp ../../oppai.c .
cp ../oppai.i .
swig -python -includeall oppai.i || exit
for img in quay.io/pypa/manylinux2010_x86_64 quay.io/pypa/manylinux2010_i686; do
  docker run --user 1000:1000 --rm -v $(pwd):/io -w /io $img \
    ./build_wheels.sh || exit
done
