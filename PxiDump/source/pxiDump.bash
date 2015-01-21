#!/bin/bash
ParseSetFile () {
    if [ -n "$2" ]
    then
        echo "1/4 \"%08x\n\"" > fmt.tmp
    else
        echo "1/4 \"%010d\n\"" > fmt.tmp
    fi
    size=5120
    offset=`expr $1 \* $size`
    offset=`expr $offset + 0`
    hexdump -v -s $offset -n $size -f fmt.tmp \
        setFile > mod$1.temp
    rm -f fmt.tmp
}

rm -f varList.txt
for i in `seq 303104 304191`
do
    num=`printf "0x000%03x\n" $i`
    name=`grep $num varFile | awk '{print $2}'`
    echo $num $name >> varList.txt
done

if [ $1 -eq "-1" ]
then
    for i in `seq 0 7`
    do 
        ParseSetFile $i $2
    done
    paste -d " " mod*.temp varList.txt > all.pars
    rm -f mod*.temp varList.txt
else
    ParseSetFile $1 $2
    paste -d " " mod$1.temp varList.txt > mod$1.pars
    rm -f mod$1.temp varList.txt
fi
