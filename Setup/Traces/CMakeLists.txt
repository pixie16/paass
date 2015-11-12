#install the traces scripts to share.
install(DIRECTORY share/traces DESTINATION share)

install(PROGRAMS source/viewBaseline.sh DESTINATION bin RENAME viewBaseline)
