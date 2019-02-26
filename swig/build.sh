#!/bin/sh

runall() {
  for d in ./*/; do
    [ "$d" = "." ] && continue
    cd "$d"
    ./build.sh || return $?
    [ ! -z $PUBLISH ] && ./publish.sh
    cd ..
  done
}

dir="$(dirname "$0")"
olddir="$(pwd)"
cd "$dir"
runall
res=$?
cd "$olddir"
exit $res
