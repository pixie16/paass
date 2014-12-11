#!/bin/awk -f
#Copyright S. V. Paulauskas 2014
#stanpaulauskasATgmailDOTcom
#Distributed under GNU General Public Licence v3

BEGIN {print "Now Converting timingCal.txt to xml..."}{
    if($2 == "small") {
        print "<Bar number=\""$1"\" z0=\""$3"\" xoffset=\""$4"\" zoffset=\""$5"\""
        print "   lroffset=\""$6"\" tofoffset0=\""$7"\" tofoffset1=\""$8"\" />" 
    }
}
END {}
