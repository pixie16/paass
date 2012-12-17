#!/bin/bash

fnm=varFile.var
for ((x=303104; x <= 304383; x++)); do
absl=$( printf '0x%08x'  "$x") 
printf '%s' "${absl}"         
 while read line ; do

    index=$(($index+1))
    num="${line:0:10}"
    fld="${line:11:28}"
    if [  "${absl}" = "${num}" ];
    then
	printf '%s'  "${fld}"         
    fi    

done < $fnm
printf '\n'         
done



 MYARRAY[$index]="$line"
#arrayT[$x]=$x
#   printf '%s %s\n'  "${line:0:10}" "${line:11:40}"
#for ((x=303104; x <= 304191; x++)); do
#arrayT[$x]=$x
#printf '0x%08x\n'  "$x" 
#printf '0x%08x\n'  "${arrayT[$x]}"
#done

# printf '%3d | %04o | 0x%08x\n' "$x" "$x" "$x"
#    printf '%20s\n'  "${MYARRAY[$index]}"
#    printf '%x\n' "$index"
