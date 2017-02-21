#!/bin/sh
cd ../vendor/gtest
autoreconf -i
./configure

which -s gmake && gmake || make
