#!/bin/sh

srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.

ORIGDIR=`pwd`
cd $srcdir

#autoreconf -v --install || exit 1
#intltoolize --copy --force --automake || exit 1
#glib-gettextize --copy --force || exit 1

# Have to be in this order because of some wierd behaviors
# of intltoolize not compiling the *.po files
autoreconf -v --install || exit 1
glib-gettextize --copy --force || exit 1
intltoolize --copy --force --automake || exit 1

cd $ORIGDIR || exit $?

$srcdir/configure --enable-maintainer-mode "$@"
