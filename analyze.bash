#!/bin/bash
if [[ -z $1 ]];then
    echo "Must provide at least one argument"
    exit 1;
fi

if [[ ! -d ${HISDIR:="HIS"} ]]; then
    echo "Histogram directory $HISDIR does not exist"
    exit 1;
fi

if [[ -z $2 ]];then
    PREFIX=$(basename $1 .ldf)
    HISFILE="$HISDIR/$PREFIX"
else
    HISFILE="$HISDIR/$2"
fi

INFILE=$1

echo -e "zero\nfile $INFILE\ngo\nend\n" | ./pixie_ldf_c $HISFILE
