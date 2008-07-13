#!/bin/sh

PROJECT=panda

srcdir=$(dirname $0)
test -z "$srcdir" && srcdir=.

(test -f $srcdir/configure.ac) || {
	echo "Directory \"$srcdir\" does not look like the top-level $PROJECT directory" 1>&2
	exit 1
}

libtoolize --force --copy
aclocal $ACLOCAL_FLAGS
autoconf
autoheader
test -f config.h.in && touch config.h.in
automake --foreign --add-missing --force --copy

if [ $# = 0 ]; then
	echo "WARNING: I am going to run configure without any arguments."
fi

./configure $@

