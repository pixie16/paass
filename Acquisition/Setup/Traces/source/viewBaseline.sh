#!/bin/bash
#Author: Karl Smith, K. Miernik
#Date: Nov. 12, 2015
#A helper function to launch gnuplot and plot the baseline from get_traces

if [ $# -ne 2 ]; then
	echo "Usage: $0 <mod> <ch>"
	exit 1
fi

mod=$1
ch=$2
dir=`poll2 --prefix`/share/traces

gnuplot -e "MOD=$mod;CH=$ch" $dir/tra
