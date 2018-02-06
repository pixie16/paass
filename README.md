Pixie Acquisition and Analysis Software Suite (PAASS)
======================================
This project combines the PixieSuite(2) and pixie_scan projects developed
at the [University of Tennessee at Knoxville](https://utk.edu) by the
[Experimental Nuclear Structure Group](http://www.phys.utk.edu/expnuclear/),
and at [Oak Ridge National Laboratory](https://ornl.gov) by members of
the Physics Division. 

The DAQ component of this software suite is taken from the
[PixieSuite2 project](https://github.com/pixie16/PixieSuite2) that was started
by K. Smith and C. Thornsberry. The orignal software comes from the
[PixieSuite](https://github.com/pixie16/PixieSuite) project developed primarily
by D. Miller. These two projects will soon be superceded by this one.

The analysis component of the software comes from the
[pixie_scan project](https://github.com/pixie16/pixie_scan). This software is
the result of more than a decade of work by many people . There have been
numerous changes to the base analysis software to improve execution
effciency. 

This marks a major milestone in both DAQ and analysis due to the
independence from the HIRBF data acquisition and analysis libraries. These
libaries are still supported by the DAQ but not by the scan software.


#Data Acquisition
#Introduction 
In addition to fixing various polling issues, this project provides a much
cleaner UI, and its own data format (pld). This version of the software may still
be used with the HRIBF data acquisition software via LDF output (default) or
through a PACMAN interface (recommended).

##Dependencies
###Required
* ncurses
* PLX 
* XIA
* gcc v4.4.6+
* cmake v2.8.8+

###Optional (must have one)
* CERN ROOT - MCA outputs into ROOT format (.root)
* HRIBF - MCA and histogramming done with HRIBF output (.his)

##Installation (assuming admin rights)
1. (admin) Install PLX software: `/opt/plx/<VERSION>/`
2. (admin) Create the PLX link: `ln -s /opt/plx/<VERSION>/PlxSdk /opt/plx/current`
3. (admin) Install XIA firmware files: `/opt/xia/firmware/`
4. (admin) Change XIA firmware directory group to acq: `chgrp -R acq /opt/xia/firmware/`
5. (admin) Add acquisition user to acq group : `usermod -a -G acq <username>`
6. (admin) Create the XIA link: `ln -s /opt/xia/firmware/<VERSION> /opt/xia/current`
7. In PixieSuite2 base directory create the "build" directory: "`mkdir build`"
8. Change to build directory and execute cmake: `cmake ../`
9. If all goes well, "`make clean && make && sudo make install && sudo make config`".
   Default install directory is `PixieSuite2/exec`

*You should ensure that the PLX version and XIA firmware versions are compatable!*

###Options for CMAKE Compilation
* -DCMAKE\_INSTALL\_PREFIX (default PixieSuite2/exec)
* -DUSE\_ROOT (default ON)
* -DUSE\_DAMM (default ON)
* -DUSE\_SETUP (default OFF)
* -DUSE\_NCURSES (default ON; required for poll2)
* -DUSE\_SCAN (default ON)
* -DUSE\_POLL (default ON)

##Tested Systems
The softare has been compiled and tested on the following setups: 

Platform | Kernel | Cmake | GCC
-------- | ------ |:-----:| ---
CentOS 6.2 | 2.6.32-220.el6.i686         | ???????? | 4.4.6 20120305
CentOS 6.3 | 2.6.32-279.19.1.el6.x86\_64 | 2.8.12.2 | 4.4.6
CentOS 6.5 | 2.6.32-431.17.1.el6.x86\_64 | ???????? | 4.4.7 20120313
RHEL 6.6   | 2.6.32-504.23.4.el6.x86\_64 | 2.8.12.2 | 4.4.7

#Data Analysis
##Introduction 
This software analyzes data from the Pixie-16 data acquisition system produced
by XIA, LLC. The recognized data format is currently .ldf, which are produced
by the HRIBF/ORNL data acquisition libraries. The code is capable of processing
a variety of detector types. The Processors are written in a modular format
such that they can easily be added or removed from the analysis. This modular
format gives the code the ability to be adapted to many experimental setups.

This code is the work of many people, their names are (hopefully) documented in
the codes they wrote, and in the documentation files. If your name is missing
or you have a contribution, please contact the code maintainers.

#Licensing
This code is licensed under the GNU GPL v. 3.0. If you have received a copy of
this code without this license you should download it and familiarize yourself
with its contents.

#Copyright
Copyright (c) 2009 - 2016, University of Tennessee Board of Trustees
All rights reserved.

This code was developed as part of the research efforts of the
Experimental Nuclear Physics Group for use with the Pixie-16
electronics. The work was supported by the following grants:

DE-FG52-08NA28552

provided by the Department of Energy and the National Science Foundation.

The work is licensed under the GNU GPLv3

Redistribution and use in source and binary forms,
with or without modification, are permitted provided
that the following conditions are met:

* Redistributions of source code must retain the above
   copyright notice, this list of conditions and the
   following disclaimer.
* Redistributions in binary form must reproduce the
   above copyright notice, this list of conditions and the
   following disclaimer in the documentation and/or other
   materials provided with the distribution.
* Neither the name of University of Tennessee nor the names of its
   contributors may be used to endorse or promote
   products derived from this software without
   specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
SUCH DAMAGE.
