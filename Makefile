#!/bin/make
# GNUmakefile using implicit rules and standard definitions
SHELL=/bin/sh

# Uncomment the following line for root functionality
# USEROOT = 1
# Uncomment this line if processing Rev. D data
REVISIOND = 1
# Uncomment this line to use the Pulse Fitting routine
# PULSEFIT = 1
# Uncomment this line for a more verbose scan
# CXXFLAGS += -DVERBOSE
# Undefine to make a "online" version
# ONLINE = 1 

#------- instruct make to search through these
#------- directories to find files
vpath %.f scan/ 
vpath %.hpp include/
vpath %.h include/
vpath %.icc include/
vpath %.cpp src/

ifeq ($(HHIRF_DIR),)
HHIRF_DIR = /usr/hhirf
endif

ifeq ($(ACQ2_LIBDIR),)
ifneq ($(ACQ2_DIR),) 
ACQ2_LIBDIR = $(ACQ2_DIR)
else
ACQ2_LIBDIR = /usr/acq2/lib
endif
endif

LIBS = $(HHIRF_DIR)/scanorlib.a $(HHIRF_DIR)/orphlib.a \
       $(ACQ2_LIBDIR)/acqlib.a  $(ACQ2_LIBDIR)/ipclib.a

OutPutOpt     = -o # keep whitespace after "-o"
ObjSuf        = o

#------- define file suffixes
fSrcSuf   = f
cSrcSuf   = c
c++SrcSuf = cpp
cxxSrcSuf = cxx

#------- define compilers
#define to compile with gfortran (>=4.2) if required for the hhirf libs
ifeq ($(HHIRF_GFORTRAN), )
FC        = g77
else
FC        = gfortran
endif

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
# CXXFLAGS += -Wall $(CINCLUDEDIRS) -Dnewreadout
ifdef REVISIOND
CXXFLAGS += -DREVD
endif
ifdef ONLINE
CXXFLAGS += -DONLINE
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
LOGICPROCESSORO  = LogicProcessor.$(ObjSuf)
TRIGGERLOGICPROCESSORO = TriggerLogicProcessor.$(ObjSuf)
MCPPROCESSORO    = McpProcessor.$(ObjSuf)
MTCPROCESSORO    = MtcProcessor.$(ObjSuf)
DSSDPROCESSORO   = DssdProcessor.$(ObjSuf)
DETECTORDRIVERO  = DetectorDriver.$(ObjSuf)
CORRELATORO      = Correlator.$(ObjSuf)
RAWEVENTO        = RawEvent.$(ObjSuf)
ROOTPROCESSORO   = RootProcessor.$(ObjSuf)
RANDOMPOOLO      = RandomPool.$(ObjSuf)
SSDPROCESSORO    = SsdProcessor.$(ObjSuf)
TAUANALYZERO     = TauAnalyzer.$(ObjSuf)
IONCHAMBERPROCESSORO = IonChamberProcessor.$(ObjSuf)
ISSDPROCESSORO   = ImplantSsdProcessor.$(ObjSuf)
STATSDATAO       = StatsData.$(ObjSuf)
PULSERPROCESSORO = PulserProcessor.$(ObjSuf)
VANDLEPROCESSORO = VandleProcessor.$(ObjSuf)
TRACEO		 = Trace.$(ObjSuf)
TRACEEXTRACTERO  = TraceExtracter.$(ObjSuf)
TRACESUBO        = TraceAnalyzer.$(ObjSuf)
TRACEPLOTO       = TracePlotter.$(ObjSuf)
TRACEFILTERO     = TraceFilterer.$(ObjSuf)
DOUBLETRACEO     = DoubleTraceAnalyzer.$(ObjSuf)
WAVEFORMSUBO     = WaveformAnalyzer.$(ObjSuf)
QDCPROCESSORO    = QdcProcessor.$(ObjSuf)
MAPFILEO         = MapFile.$(ObjSuf)
DETECTORLIBRARYO = DetectorLibrary.$(ObjSuf)

ifdef USEROOT
PIXIE = pixie_ldf_c_root$(ExeSuf)
else
ifdef ONLINE
PIXIE = pixie_ldf_c_online$(ExeSuf)
else
PIXIE = pixie_ldf_c$(ExeSuf)
endif
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
	$(SSDPROCESSORO) $(ISSDPROCESSORO) $(TAUANALYZERO) $(LOGICPROCESSORO) \
	$(TRIGGERLOGICPROCESSORO) $(TRACEEXTRACTERO) $(IONCHAMBERPROCESSORO) \
	$(STATSDATAO) $(WAVEFORMSUBO) $(VANDLEPROCESSORO) $(PULSERPROCESSORO) \
	$(QDCPROCESSORO) $(MAPFILEO) $(DETECTORLIBRARYO)

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