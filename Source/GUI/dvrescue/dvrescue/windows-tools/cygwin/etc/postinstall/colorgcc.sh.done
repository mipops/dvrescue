#!/bin/sh
# This is automatically generated file
# Please do not remove section comments '#:<name>'

PATH=/bin:/sbin:/usr/bin:/usr/sbin
LC_ALL=C
dest=$1

set -e

#:etc
fromdir=/etc/defaults
for i in   etc etc/colorgcc etc/colorgcc/colorgccrc
do
    src=$fromdir/$i
    destdir=$dest/$i

    [ -e $destdir ] && continue

    if [ -d $src ] ; then
	install -d -m 755 $destdir
	continue
    fi

    install -m 644 $src $destdir
done

