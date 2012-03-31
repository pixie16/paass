#!/bin/bash

if [[ -f ./pixie_ldf_c_online ]]; then
    PROG=./pixie_ldf_c_online
else
    if [[ -f ./pixie_ldf_c ]]; then
	PROG=./pixie_ldf_c
	echo "Running standard version."
    else
	echo "Make a program first, yo."
	exit 1
    fi
    
fi

if [[ ! -d ${HISDIR:=HIS} ]]; then
    echo "Histogram directory $HISDIR does not exist."
    exit 1
fi

echo -e "zero\nshm\ngo\nend\n" | $PROG $HISDIR/current
