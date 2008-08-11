#!/bin/sh

PROJECT=panda

srcdir=$(dirname $0)
test -z "$srcdir" && srcdir=.

(test -f $srcdir/configure.ac) || {
    echo "Directory \"$srcdir\" does not look like the top-level $PROJECT directory" 1>&2
    exit 1
}

autoreconf --force --install --verbose
./configure $@

