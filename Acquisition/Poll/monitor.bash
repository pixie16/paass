#!/bin/bash
# @authors D. Miller
RATESFILE="/tmp/rates.txt"
TMPFILE="/tmp/.rates.current.tmp"
LASTTIME=""

tput clear
tput civis

while true; do
    MAXLI=$(tput lines)
    MAXCOL=$(tput cols)
    if [[ $LASTCOLS != $MAXCOL || $MAXLI != $LASTLI ]]; then
	tput clear
    fi
    LASTCOLS=$MAXCOL
    LASTLI=$MAXLI

    tput cup 0 0
    tput smul
    date
    tput sgr0

    if [[ ! -f $RATESFILE ]]; then
	echo -n "Scalar file $RATESFILE not found"	
	tput ed
	sleep 0.2	
	continue
    fi
  
    #first copy to a temporary file so it doesn't change unexpectedly
    cp $RATESFILE $TMPFILE

    let COL=MAXCOL/2
    tput cup 0 $COL
    TIME=$(head -n1 $TMPFILE | awk '{print $3}')
    if [[ $TIME != $LASTTIME ]]; then
	tput setf 4
    fi
    echo -n "$TIME seconds elapsed"
    LASTTIME=$TIME
    tput el
    tput sgr0

    DATARATE=$(awk '{if ($4=="dataRate") print $6}' $TMPFILE)

    MAXMOD=$(awk '$1 ~ /[0-9]+/ {print $1}' $TMPFILE | sort -nr | head -n1)
    MAXCH=$(awk '$2 ~ /[0-9]+/ {print $2}' $TMPFILE | sort -nr | head -n1)

    let WIDTH='MAXCOL/(MAXMOD+2)'

    let MINLI=MAXCH+6
    let MINCOL='5*(MAXMOD + 1)'

    if (( MINLI > MAXLI )); then
	tput cup 2 0
	echo "Not enough lines -- Need at least $MINLI"
	tput ed
	sleep 0.2
	continue
    fi
    if (( WIDTH < 5 )); then
	tput cup 2 0
	echo "Not enough columns -- Need at least $MINCOL"
	tput ed
	sleep 0.2
	continue
    fi

    for (( CH=0; CH<=MAXCH; CH++ )); do
	let ROW=CH+2
	tput cup $ROW 1
	echo C$CH
    done

    for (( MOD=0; MOD<=MAXMOD; MOD++ )); do
	let COL='(MOD+1)*WIDTH'
	tput cup 1 $COL
	echo M$MOD
	for (( CH=0; CH<=MAXCH; CH++ )); do
	    let ROW=CH+2
	    tput cup $ROW $COL
	    RATE=$(awk "{if (\$1==$MOD && \$2==$CH) print \$3}" $TMPFILE)
	    TOTAL=$(awk "{if (\$1==$MOD && \$2==CH) print \$4}" $TMPFILE)
	    # do some mumbo jumbo to get the right formatting
	    if (( ${#RATE}>9 )); then
		FACTOR=1000000
		NCUT=9
		SUFFIX="M"
	    elif (( ${#RATE}>6 )); then
		FACTOR=1000
		NCUT=6
		SUFFIX="k"
	    else
		FACTOR=1
		NCUT=3
		SUFFIX=" "
	    fi
	    let REMAIN=${#RATE}-NCUT
	    if (( REMAIN == 1 )); then
		if [[ "z$SUFFIX" == "z " ]]; then
		    echo " ${RATE:0:3}  "
		else
		    echo "${RATE:0:1}.${RATE:1:1}$SUFFIX "
		fi
	    elif (( REMAIN == 2 )); then
		echo " ${RATE:0:2}$SUFFIX "
	    else
		echo "${RATE:0:3}$SUFFIX "
	    fi
	done
    done
    let ROW=MAXCH+5
    tput cup $ROW 1
    echo -n "Incoming data: $DATARATE B/s"
    tput ed

    sleep 0.2
done
