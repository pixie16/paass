
FILE='trace_'.STRIP.'_'

set yrange [200:420]
set key at 400,410
set xlabel 'Time (samples)'
set ylabel 'Counts / sample'


plot FILE.'T.txt' u 1:2 w histeps lt 1 lw 3 t 'Near Top, Top signal',\
FILE.'T.txt' u 1:3 w histeps lt 1 lw 1 t 'Bottom',\
FILE.'M.txt' u 1:2 w histeps lt 2 lw 3 t 'Near Middle Top signal',\
FILE.'M.txt' u 1:3 w histeps lt 2 lw 1 t 'Bottom',\
FILE.'B.txt' u 1:2 w histeps lt 3 lw 3 t 'Near Bottom, Top signal',\
FILE.'B.txt' u 1:3 w histeps lt 3 lw 1 t 'Bottom'
