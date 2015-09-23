PixieSuite2
===========
A software suite used to configure and poll data from XIA LLC. Pixie16 electronics.
The PixieSuite2 project was started by K. Smith and C. Thornsberry. The project
became necessary as the original PixieSuite software began to show its age.

In addition to fixing various polling issues, this project provides a much
cleaner UI, and its own data format (pld). This version of the software may still
be used with the HRIBF data acquisition software via LDF output (default) or
through a PACMAN interface (recommended).

##Dependencies
###Required
* ncurses
* PLX 
* PXI
* gcc v4.4.7+
* cmake v2.8.8+

###Optional (must have one)
* CERN ROOT - MCA outputs into ROOT format (.root)
* HRIBF - MCA and histogramming done with HRIBF output (.his)

##Installation (assuming admin rights)
1. (admin) Install PLX software: `/opt/plx/<VERSION>/`
2. (admin) Create the PLX link: `ln -s /opt/plx/<VERSION>/PlxSdk /opt/plx/current`
3. (admin) Install PXI firmware files: `/opt/xia/firmware/`
4. (admin) Change PXI firmware directory group to acq: `chgrp -R acq /opt/xia/firmware/`
5. (admin) Add acquisition user to acq group : `usermod -a -G acq <username>`
6. (admin) Create the PXI link: `ln -s /opt/xia/firmware/<VERSION> /opt/xia/current`
7. In PixieSuite2 base directory create the "build" directory: "`mkdir build`"
8. Change to build directory and execute cmake: `cmake ../`
9. If all goes well, "`make clean && make && make install`". Default install directory
is `PixieSuite2/exec`

*You should ensure that the PLX version and PXI firmware versions are compatable!*

###Options for CMAKE Compilation
* -DCMAKE\_INSTALL\_PREFIX (default PixieSuite2/exec)
* -DUSE_ROOT (default ON)
* -DUSE_DAMM (default ON)
* -DUSE_SETUP (default OFF)
* -DUSE_NCURSES (default ON; required for poll2)
* -DCORE_ONLY (default OFF)

##Tested Systems
The softare has been compiled and tested on the following Setups

* CentOS 6.5 (Final) w/ kernel 2.6.32-431.17.1.el6.x86_64 and gcc 4.4.7 20120313
* CentOS 6.2 (Final) w/ kernel 2.6.32-220.el6.i686 and gcc 4.4.6 20120305
