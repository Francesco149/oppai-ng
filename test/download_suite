#!/bin/sh

if [ $(find test_suite -name *.osu 2>/dev/null | wc -l) = "0" ]
then
    wget http://www.hnng.moe/stuff/test_suite_20170811.tar.xz \
        || exit 1
    tar xf test_suite_20170811.tar.xz || exit 1
else
    echo "using existing test_suite"
fi
