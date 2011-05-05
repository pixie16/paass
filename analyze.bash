#!/bin/bash
if [[ "z$1" == "z" ]];then
    echo "Must provide at least one argument"
    exit 1;
fi

HISDIR="HIS"

if [[ "z$2" == "z" ]];then
    PREFIX=$(basename $1 .ldf)
    HISFILE="$HISDIR/$PREFIX"
else
    HISFILE="$HISDIR/$2"
fi

INFILE=$1

echo -e "zero\nfile $INFILE\ngo\nend\n" | ./pixie_ldf_c $HISFILE