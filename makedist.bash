#!/bin/bash
DISTNAME=pixie_scan

if [[ -d .git ]]; then
    VERSION=`git describe --tags --abbrev=1`;
    git archive --format=tar --prefix=$PREFIX/ HEAD | \
	gzip > $DISTNAME-$VERSION.tgz
else
    echo "This is not a git repository, copying whole tree"
    mkdir $DISTNAME
    cp -t $DISTNAME -r * 
    tar -czf $DISTNAME-`date +%d%m%y`.tgz --exclude=*~ $DISTNAME
    rm -r $DISTNAME
fi