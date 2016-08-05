#!/bin/bash

rm -f callgrind.out.* test00.*
#make -C build/ -j4 install && valgrind --tool=callgrind ./utkscan --config config/svp/pulsertst.xml -i /home/vincent/prof/data/utk/skutek/sipmSkutek_001.ldf -o test00

make -C build/ -j4 install && ./utkscan --config config/svp/pulsertst.xml -i /home/vincent/prof/data/utk/skutek/sipmSkutek_001.ldf -o test00 
