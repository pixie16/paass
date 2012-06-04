#!/usr/bin/tclsh
#
# Calculates Qdc sums for trace
# Format of trace is "i top bottom" 

if {$argc == 1} {
    set name [lindex $argv 0]
} else {
    puts "Wrong number of arguments"
    puts "USAGE: "
    puts "qdcsum file.txt"
    exit
}

set fileIn [open $name r]

set sumT {0 0 0 0 0 0 0 0} 
set sumB {0 0 0 0 0 0 0 0} 
set qdc {29.9 59.9 99.9 174.9 249.9 349.9 949.9 1000}

set iQdc 0 
while {[gets $fileIn line] >= 0} {

    if {[string index $line 0] != "#"} {
        set x [lindex $line 0]
        set y1 [lindex $line 1]
        set y2 [lindex $line 2]

        while {$x > [lindex $qdc $iQdc] && $iQdc < [llength $qdc]} {
            incr iQdc
        }

       set sumT [lreplace $sumT $iQdc $iQdc [expr {[lindex $sumT $iQdc] + $y1}]]
       set sumB [lreplace $sumB $iQdc $iQdc [expr {[lindex $sumB $iQdc] + $y2}]]
    }
}

set baseT [lindex $sumT 0]
set baseB [lindex $sumB 0]

puts "QDC(0)\t$baseT\t$baseB"

for {set iQdc 1} {$iQdc < [llength $qdc]} {incr iQdc} { 
    set T [lindex $sumT $iQdc]
    set B [lindex $sumB $iQdc]
    set R [expr {1.0 * ($T - $baseT) / ($B - $baseB)}]
    puts "QDC($iQdc)\t$T\t$B\t$R"
}
close $fileIn
