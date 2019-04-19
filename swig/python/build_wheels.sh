#!/bin/sh
# this is meant to be used from docker

for pybin in /opt/python/*/bin
do
  rm *.so
  "$pybin/pip" wheel . -w dist/ || exit
done

"$pybin/python" ./setup.py sdist || exit

for w in dist/*linux_*.whl; do
  auditwheel repair "$w" -w dist/ || exit
done
rm dist/*linux_*.whl
