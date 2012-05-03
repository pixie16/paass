#!/bin/bash

if [[ -z $1 ]];then
    echo "Must provide at least one argument"
    exit 1;
fi

if [ ! -d ${HISDIR:="HIS"} ]; then
    echo "Histogram directory $HISDIR does not exist."
    exit 1
fi

if [ ! -d ${LDFDIF:="$HOME/LDF"} ]; then
    echo "LDF directory $LDFDIR does not exist."
    exit 1
fi

if [[ -z $2 ]];then
    PREFIX=$(basename $1 .ldf)
    HISFILE="$HISDIR/sum$PREFIX"
else
    HISFILE="$HISDIR/sum$2"
fi

echo -e "zero" > .tmpfile
# sort all files which haven't changed in the last minute in order by increasing modification time
#   also touch the access times so the crunch cron job won't move them
find -L $LDFDIR -mmin +1 -name "$1*.ldf" -execdir touch -a '{}' \; -printf "%T@ %p\n" | sort -n | awk '{printf "file %s\ngo\n",$2 >> ".tmpfile" }'
echo -e "end" >> .tmpfile

cat .tmpfile | ./pixie_ldf_c $HISFILE
