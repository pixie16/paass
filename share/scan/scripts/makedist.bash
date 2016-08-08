#!/bin/bash
DISTNAME=pixie_scan

if [[ -d .git ]]; then
    BRANCH=`git branch 2>/dev/null | sed -e '/^[^*]/d' -e 's/* //'`
    VERSION=`git describe --tags --abbrev=1`
    git archive --format=tar --prefix=$DISTNAME/ HEAD | \
	gzip > $DISTNAME-$BRANCH-$VERSION.tgz
else
    if [[ -d $DISTNAME ]]; then
	echo "Temporary directory $DISTNAME already exists."
	exit
    fi
    echo "This is not a git repository, copying whole tree"
    FILELIST=$(ls --ignore=*.tgz)
    mkdir $DISTNAME
    cp -t $DISTNAME -r $FILELIST
    tar -czf $DISTNAME-$HOSTNAME-`date +%d%m%y`.tgz --exclude=*~ $DISTNAME
    rm -r $DISTNAME
fi