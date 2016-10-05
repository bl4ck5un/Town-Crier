#!/bin/sh
libtoolize --install --copy --force --automake
aclocal -I m4
autoconf
automake --add-missing --copy
