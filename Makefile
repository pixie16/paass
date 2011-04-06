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

# DIRA2=/usr/hhirf/g77
# DIRB= /usr/acq2/lib
DIRA2 = /usr/hhirf
DIRB  = /usr/hhirf

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
# FC        = g77
#uncomment to compile with gfortran (>=4.2) if required for the hhirf libs
FC        = gfortran
GCC       = gcc 
CXX       = g++
LINK.o    = $(FC) $(LDFLAGS)

# -Dnewreadout is needed to account for a change to pixie16 readout
# structure change on 03/20/08.  Remove for backwards compatability
#
# for debug and profiling add options -g -pg
# and remove -O
#------- define basic compiler flags, no warnings on code that is not my own
FFLAGS   += -O3
GCCFLAGS += -fPIC $(CINCLUDEDIRS) -Dnewreadout
CXXFLAGS += -Wall -fPIC $(CINCLUDEDIRS) -Dnewreadout
ifdef REVISIOND
CXXFLAGS += -DREVD
endif

#------- include directories for the pixie c files
CINCLUDEDIRS  = -Iinclude

#------- basic linking instructions
LDLIBS   += -lm -lstdc++
ifdef PULSEFIT
LDLIBS   += -lgsl -lgslcblas
CXXFLAGS += -Dpulsefit
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
DETECTORDRIVERO  = DetectorDriver.$(ObjSuf)
CORRELATORO      = Correlator.$(ObjSuf)
RAWEVENTO        = RawEvent.$(ObjSuf)
ROOTPROCESSORO   = RootProcessor.$(ObjSuf)
RANDOMPOOLO      = RandomPool.$(ObjSuf)
SSDPROCESSORO    = SsdProcessor.$(ObjSuf)
STATSDATAO       = StatsData.$(ObjSuf)
PULSERPROCESSORO = PulserProcessor.$(ObjSuf)
VANDLEPROCESSORO = VandleProcessor.$(ObjSuf)
TRACEO		 = Trace.$(ObjSuf)
TRACESUBO        = TraceAnalyzer.$(ObjSuf)
TRACEPLOTO       = TracePlotter.$(ObjSuf)
TRACEFILTERO     = TraceFilterer.$(ObjSuf)
DOUBLETRACEO     = DoubleTraceAnalyzer.$(ObjSuf)
WAVEFORMSUBO     = WaveformAnalyzer.$(ObjSuf)

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
	$(TRACEPLOTO) $(TRACEFILTERO) $(DOUBLETRACEO) \
	$(MESSLOGO) $(MILDATIMO) $(SCANORUXO) $(ACCUMULATORO) $(PIXIEO) \
	$(HISTOGRAMMERO) $(EVENTPROCESSORO) $(SCINTPROCESSORO) $(TRACEO) \
	$(GEPROCESSORO) $(DSSDPROCESSORO) $(RAWEVENTO) $(RANDOMPOOLO) \
	$(SSDPROCESSORO) \
	$(STATSDATAO) $(WAVEFORMSUBO) $(VANDLEPROCESSORO) $(PULSERPROCESSORO)

ifdef USEROOT
OBJS  += $(ROOTPROCESSORO)
endif

PROGRAMS = $(PIXIE)

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

.phony: all clean
all:     $(PROGRAMS)

#----------- remove all objects, core and .so file
clean:
	@echo "Cleaning up..."
	@rm -f $(OBJS) $(PIXIE) core *~ src/*~ include/*~ scan/*~

#----------- link all created objects together
#----------- to create pixie_ldf_c program
$(PIXIE): $(OBJS) $(LIBS)
	$(LINK.o) $(LDLIBS) $^ -o $@