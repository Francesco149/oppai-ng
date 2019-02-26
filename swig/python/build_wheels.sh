#!/bin/sh
# this is meant to be used from docker

rm *.so
for pybin in /opt/python/*/bin
do
  "$pybin/python" ./setup.py build_ext --inplace || exit
  "$pybin/pip" wheel . -w dist/ || exit
done

"$pybin/python" ./setup.py sdist || exit

for w in dist/*linux_*.whl; do
  auditwheel repair "$w" -w dist/ || exit
done
rm dist/*linux_*.whl
