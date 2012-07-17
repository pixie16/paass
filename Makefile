#!/bin/make
# GNUmakefile using implicit rules and standard definitions
SHELL=/bin/sh

# Uncomment the following line for root functionality
# USEROOT = 1
# Uncomment this line if processing Rev. D data
REVISIOND = 1
# Uncomment this line if processing Rev. F data
#REVISIONF = 1
# Uncomment this line for a more verbose scan
# CXXFLAGS += -DVERBOSE
# Undefine to make a "online" version
# ONLINE = 1 
# Use gfortran
# HHIRF_GFORTRAN = 1
# Libs in HHIRF DIR
# LIBS_IN_HHIRF = 1

#These will set the analysis used on the waveforms
#Uncomment this line to use the Pulse Fitting routine
PULSEFIT = 1
#Uncomment this line to use the cfd
#DCFD = 1


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

ifneq ($(LIBS_IN_HHIRF),)
ACQ2_LIBDIR = $(HHIRF_DIR)
else
ifeq ($(ACQ2_LIBDIR),)
ifneq ($(ACQ2_DIR),) 
ACQ2_LIBDIR = $(ACQ2_DIR)
else
ACQ2_LIBDIR = /usr/acq2/lib
endif
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
ifdef REVISIONF
CXXFLAGS += -DREVF
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
else ifdef DCFD
CXXFLAGS += -Ddcfd
endif

ifeq ($(FC),gfortran)
FFLAGS	 += -fsecond-underscore
LDLIBS	 += -lgfortran
GCCFLAGS += -O3
CXXFLAGS += -O3 -DLINK_GFORTRAN
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
#General Objects
ACCUMULATORO     = StatsAccumulator.$(ObjSuf)
CORRELATORO      = Correlator.$(ObjSuf)
DETECTORDRIVERO  = DetectorDriver.$(ObjSuf)
DETECTORLIBRARYO = DetectorLibrary.$(ObjSuf)
EVENTPROCESSORO  = EventProcessor.$(ObjSuf)
MAPFILEO         = MapFile.$(ObjSuf)
PIXIEO           = PixieStd.$(ObjSuf)
RAWEVENTO        = RawEvent.$(ObjSuf)
RANDOMPOOLO      = RandomPool.$(ObjSuf)
STATSDATAO       = StatsData.$(ObjSuf)
TIMINGINFOO      = TimingInformation.$(ObjSuf)
#Plot Objects
HISTOGRAMMERO    = DeclareHistogram.$(ObjSuf)
PLOTSREGISTERO   = PlotsRegister.$(ObjSuf)
PLOTSO           = Plots.$(ObjSuf)
#Trace Related Objects
CFDANALYZERO     = CfdAnalyzer.$(ObjSuf)
DOUBLETRACEO     = DoubleTraceAnalyzer.$(ObjSuf)
FITTINGANALYZERO = FittingAnalyzer.$(ObjSuf)
TAUANALYZERO     = TauAnalyzer.$(ObjSuf)
TRACEO		 = Trace.$(ObjSuf)
TRACEEXTRACTERO  = TraceExtracter.$(ObjSuf)
TRACEFILTERO     = TraceFilterer.$(ObjSuf)
TRACEPLOTO       = TracePlotter.$(ObjSuf)
TRACESUBO        = TraceAnalyzer.$(ObjSuf)
WAVEFORMSUBO     = WaveformAnalyzer.$(ObjSuf)
#Dectector Processor Objects
DSSDPROCESSORO   = DssdProcessor.$(ObjSuf)
GEPROCESSORO     = GeProcessor.$(ObjSuf)
ISSDPROCESSORO   = ImplantSsdProcessor.$(ObjSuf)
IONCHAMBERPROCESSORO = IonChamberProcessor.$(ObjSuf)
LOGICPROCESSORO  = LogicProcessor.$(ObjSuf)
MCPPROCESSORO    = McpProcessor.$(ObjSuf)
MTCPROCESSORO    = MtcProcessor.$(ObjSuf)
POSITIONPROCESSORO = PositionProcessor.$(ObjSuf)
PULSERPROCESSORO = PulserProcessor.$(ObjSuf)
SCINTPROCESSORO  = ScintProcessor.$(ObjSuf)
SSDPROCESSORO    = SsdProcessor.$(ObjSuf)
TRIGGERLOGICPROCESSORO = TriggerLogicProcessor.$(ObjSuf)
VANDLEPROCESSORO = VandleProcessor.$(ObjSuf)
VALIDPROCESSORO  = ValidProcessor.$(ObjSuf)
#ROOT Objects
SCINTROOTO       = ScintROOT.$(ObjSuf)
ROOTPROCESSORO   = RootProcessor.$(ObjSuf)
VANDLEROOTO      = VandleROOT.$(ObjSuf)


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
ifdef REVISIONF
READBUFFDATAO    = ReadBuffData.RevD.$(ObjSuf)
else
READBUFFDATAO    = ReadBuffData.$(ObjSuf)
endif
endif

#----- list of objects
OBJS   = $(READBUFFDATAO) $(SET2CCO) $(DSSDSUBO) $(DETECTORDRIVERO) \
	$(PLOTSREGISTERO) $(PLOTSO) $(MTCPROCESSORO) $(MCPPROCESSORO) \
	$(LOGICPROCESSORO) $(CORRELATORO) $(TRACESUBO) $(TRACEO) $(TRACEPLOTO) \
	$(TRACEFILTERO) $(DOUBLETRACEO) $(MESSLOGO) $(MILDATIMO) $(SCANORUXO) \
	$(ACCUMULATORO) $(PIXIEO) $(HISTOGRAMMERO) $(EVENTPROCESSORO) \
	$(SCINTPROCESSORO) $(GEPROCESSORO) $(DSSDPROCESSORO) $(RAWEVENTO) \
	$(RANDOMPOOLO) $(SSDPROCESSORO) $(ISSDPROCESSORO) $(TAUANALYZERO) \
	$(TRIGGERLOGICPROCESSORO) $(TRACEEXTRACTERO) $(IONCHAMBERPROCESSORO) \
	$(STATSDATAO) $(WAVEFORMSUBO) $(VANDLEPROCESSORO) $(PULSERPROCESSORO) \
	$(POSITIONPROCESSORO) $(MAPFILEO) $(DETECTORLIBRARYO) $(WAVEFORMSUBO) \
	$(VANDLEPROCESSORO) $(PULSERPROCESSORO) $(TIMINGINFOO) $(VALIDPROCESSORO) \

ifdef PULSEFIT
OBJS += $(FITTINGANALYZERO)
else ifdef DCFD
OBJS += $(CFDANALYZERO) 
endif

ifdef USEROOT
OBJS  += $(ROOTPROCESSORO) $(VANDLEROOTO) $(SCINTROOTO)
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
	$(LINK.o) $^ -o $@ $(LDLIBS)
