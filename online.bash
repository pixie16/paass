#!/bin/bash

if [[ -x ./pixie_ldf_c_online ]]; then
    PROG=./pixie_ldf_c_online
else
    PROG=./pixie_ldf_c
fi

if [[ ! -d ${HISDIR:=HIS} ]]; then
    echo "Histogram directory $HISDIR does not exist."
    exit 1
fi

echo -e "zero\nshm\ngo\nend\n" | $PROG $HISDIR/current
