#!/bin/make
# GNUmakefile using implicit rules and standard definitions
SHELL=/bin/sh

# uncomment the following line for root functionality
#USEROOT = 1
# uncomment this line if processing Rev. D data
#REVISIOND = 1
#Uncomment this line to use the Pulse Fitting routine
#PULSEFIT = 1

#------- instruct make to search through these
#------- directories to find files
vpath %.f scan/ 
vpath %.h include/
vpath %.cpp src/
vpath %.cxx src/

DIRA2=/usr/hhirf/g77
DIRB= /usr/acq2/lib
# DIRA2 = /usr/hhirf
# DIRB  = /usr/hhirf

LIBS = $(DIRA2)/scanorlib.a $(DIRA2)/orphlib.a \
       $(DIRB)/acqlib.a  $(DIRB)/ipclib.a

OutPutOpt     = -o # keep whitespace after "-o"
ObjSuf        = o

#------- define file suffixes
fSrcSuf   = f
cSrcSuf   = c
c++SrcSuf = cpp
cxxSrcSuf = cxx

#------- define compilers
FC        = g77
#uncomment to compile with gfortran (>=4.2) if required for the hhirf libs
#FC        = gfortran
GCC       = gcc 
CXX       = g++
LINK.o    = $(FC) $(LDFLAGS) $(TARGET_ARCH)

#------- define basic compiler flags, no warnings on code that is not my own
FFLAGS   += -O3
GCCFLAGS += -fPIC $(CINCLUDEDIRS)
CXXFLAGS += -Wall -fPIC $(CINCLUDEDIRS) -Dnewreadout
ifdef REVISIOND
CXXFLAGS += -DREVD
endif

# -Dnewreadout is needed to account for a change to pixie16 readout
# structure change on 03/20/08.  Remove for backwards compatability
#
# for debug and profiling add options -g -pg
# and remove -O

#------- include directories for the pixie c files
CINCLUDEDIRS  = -Iinclude

#------- basic linking instructions
ifdef PULSEFIT
LDLIBS   += -lm -lgsl -lgslcblas -lstdc++ -lg2c
CXXFLAGS += -Dpulsefit
else
LDLIBS   += -lm -lstdc++ 
endif

ifeq ($(FC),gfortran)
FFLAGS	 += -fsecond-underscore
LDLIBS	 += -lgfortran
GCCFLAGS += -O3
CXXFLAGS += -O3
else
LDFLAGS += -g77libs
LDLIBS	+= -lg2c
endif

#-------- define file variables -----------------------

# objects from fortran
SET2CCO          = set2cc.$(ObjSuf)
MESSLOGO         = messlog.$(ObjSuf)
MILDATIMO        = mildatim.$(ObjSuf)
SCANORUXO        = scanorux.$(ObjSuf)

# objects from cpp
PIXIEO           = PixieStd.$(ObjSuf)
ACCUMULATORO     = StatsAccumulator.$(ObjSuf)
HISTOGRAMMERO    = DeclareHistogram.$(ObjSuf)
EVENTPROCESSORO  = EventProcessor.$(ObjSuf)
SCINTPROCESSORO  = ScintProcessor.$(ObjSuf)
GEPROCESSORO     = GeProcessor.$(ObjSuf)
MCPPROCESSORO    = McpProcessor.$(ObjSuf)
MTCPROCESSORO    = MtcProcessor.$(ObjSuf)
DSSDPROCESSORO   = DssdProcessor.$(ObjSuf)
TRACESUBO        = TraceAnalyzer.$(ObjSuf)
DETECTORDRIVERO  = DetectorDriver.$(ObjSuf)
CORRELATORO      = Correlator.$(ObjSuf)
RAWEVENTO        = RawEvent.$(ObjSuf)
ROOTPROCESSORO   = RootProcessor.$(ObjSuf)
RANDOMPOOLO      = RandomPool.$(ObjSuf)
STATSDATAO       = StatsData.$(ObjSuf)
WAVEFORMPROCESSORO = WaveformProcessor.$(ObjSuf)
PULSERPROCESSORO   = PulserProcessor.$(ObjSuf)
VANDLEPROCESSORO   = VandleProcessor.$(ObjSuf)

ifdef USEROOT
PIXIE            = pixie_ldf_c_root$(ExeSuf)
else
PIXIE            = pixie_ldf_c$(ExeSuf)
endif

ifdef REVISIOND
READBUFFDATAO    = ReadBuffData.RevD.$(ObjSuf)
else
READBUFFDATAO    = ReadBuffData.$(ObjSuf)
endif

#----- list of objects
OBJS   = $(READBUFFDATAO) $(SET2CCO) $(DSSDSUBO) $(DETECTORDRIVERO) \
	$(MTCPROCESSORO) $(MCPPROCESSORO) $(CORRELATORO) $(TRACESUBO) \
	$(MESSLOGO) $(MILDATIMO) $(SCANORUXO) $(ACCUMULATORO) $(PIXIEO) \
	$(HISTOGRAMMERO) $(EVENTPROCESSORO) $(SCINTPROCESSORO) \
	$(GEPROCESSORO) $(SPLINEFITPROCESSORO) $(SPLINEPROCESSORO) \
	$(DSSDPROCESSORO) $(RAWEVENTO) $(RANDOMPOOLO) $(STATSDATAO) \
	$(WAVEFORMPROCESSORO) $(VANDLEPROCESSORO) $(PULSERPROCESSORO)

ifdef USEROOT
OBJS  += $(ROOTPROCESSORO)
endif

PROGRAMS = $(PIXIE)

DISTTARGETS = src include scan manual Makefile Doxyfile map.txt cal.txt
DISTNAME = pixie_scan
DOCSTARGETS = html latex

#------------ adjust compilation if ROOT capability is desired -------
ifdef USEROOT
ROOTCONFIG   := root-config

#no uncomment ROOTCLFAGS   := $(filter-out pthread,$(ROOTCFLAGS))
CXXFLAGS     += $(shell $(ROOTCONFIG) --cflags) -Duseroot
LDFLAGS      += $(shell $(ROOTCONFIG) --ldflags)
LDLIBS       := $(shell $(ROOTCONFIG) --libs)
endif

#--------- Add to list of known file suffixes
.SUFFIXES: .$(cxxSrcSuf) .$(fSrcSuf) .$(c++SrcSuf) .$(cSrcSuf)

.phony: all clean dist distdocs
all:     $(PROGRAMS)

#----------- remove all objects, core and .so file
clean:
	@echo "Cleaning up..."
	@rm -f $(OBJS) $(PIXIE) core *~ src/*~ include/*~ scan/*~

dist:
	@mkdir $(DISTNAME)
	@cp -t $(DISTNAME) -r $(DISTTARGETS)
	@tar -czf $(DISTNAME)-`date +%d%m%y`.tgz --exclude=*~ $(DISTNAME)
	@$(RM) -r $(DISTNAME)

distdocs:
	@mkdir $(DISTNAME)
	@cp -t $(DISTNAME) -r $(DISTTARGETS) $(DOCSTARGETS)
	@tar -czf $(DISTNAME)-with-docs-`date +%d%m%y`.tgz --exclude=*~ $(DISTNAME)
	@$(RM) -r $(DISTNAME)

#----------- link all created objects together
#----------- to create pixie_ldf_c program
$(PIXIE): $(OBJS) $(LIBS)
	$(LINK.o) $(LDLIBS) $^ -o $@