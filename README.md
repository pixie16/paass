University of Tennessee at Knoxville Pixie-16 Analysis Software
===============================================================

Introduction
------------
This software analyzes data from the Pixie-16 data acquisition system produced
by XIA, LLC. The recognized data format is currently .ldf, which are produced
by the HRIBF/ORNL data acquisition libraries. The code is capable of processing
a variety of detector types. The Processors are written in a modular format
such that they can easily be added or removed from the analysis. This modular
format gives the code the ability to be adapted to many experimental setups.

This code is the work of many people, their names are (hopefully) documented in
the codes they wrote, and in the documentation files. If your name is missing
or you have a contribution, please contact the code maintainers.

Documentation
-------------
The documentation for the code can be found in the doc folder. Doxygen provides
automatic documentation for the various pieces of the program. To generate the
documentation one may simply type `make doc` in the command line. This will
generate the html and latex documentation for the project. In addition there are
two setup guides for the software. the first setup guide is for the scan code
to make sure that the various environment variables, processors, etc are set
properly. The second guide contains notes about K. Miernik's TreeCorrelator.

Licensing
---------
This code is licensed under the GNU GPL v. 3.0. If you have received a copy of
this code without this license you should download it and familiarize yourself
with its contents.

Copyright
---------
Copyright (c) 2009 - 2014, University of Tennessee Board of Trustees
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
