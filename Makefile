#!/bin/make
# GNUmakefile using implicit rules and standard definitions
SHELL=/bin/sh

# Uncomment the following line for root functionality
# USEROOT = 1

# Uncomment this line if processing Rev. D data
#REVISIOND = 1
# Uncomment this line if processing Rev. F data
REVISIONF = 1

# Uncomment this line for a more verbose scan
# CXXFLAGS += -DVERBOSE

# Undefine to make a "online" version
# ONLINE = 1 

# Define to use Gamma-Gamma gates in GeProcessor
# This turns on Gamma-Gamma angular distribution
# and Gamma-Gamma-Gamma gates
GGATES = 1

# Use gfortran
HHIRF_GFORTRAN = 1

#These will set the analysis used on the waveforms
#Uncomment this line to use the Pulse Fitting routine
#PULSEFIT = 1
#Uncomment this line to use the cfd
#DCFD = 1


#------- instruct make to search through these
#------- directories to find files
vpath %.f scan/ 
vpath %.hpp include/
vpath %.h include/
vpath %.icc include/
vpath %.cpp src/

HHIRF_DIR = /usr/hhirf
ACQ2_LIBDIR = /usr/hhirf

LIBS = $(HHIRF_DIR)/scanorlib.a $(HHIRF_DIR)/orphlib.a\
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
SCANORO          = scanor.$(ObjSuf)

#XML parser
PUGIXMLO = pugixml.$(ObjSuf)

# objects from cpp
PIXIEO           = PixieStd.$(ObjSuf)

BETASCINTPROCESSORO  = BetaScintProcessor.$(ObjSuf)
CFDANALYZERO     = CfdAnalyzer.$(ObjSuf)
CHANEVENTO       = ChanEvent.$(ObjSuf)
CHANIDENTIFIERO  = ChanIdentifier.$(ObjSuf)
CORRELATORO      = Correlator.$(ObjSuf)
DETECTORDRIVERO  = DetectorDriver.$(ObjSuf)
DETECTORLIBRARYO = DetectorLibrary.$(ObjSuf)
DETECTORSUMMARYO = DetectorSummary.$(ObjSuf)
DOUBLETRACEO     = DoubleTraceAnalyzer.$(ObjSuf)
DSSDPROCESSORO   = DssdProcessor.$(ObjSuf)
EVENTPROCESSORO  = EventProcessor.$(ObjSuf)
FITTINGANALYZERO = FittingAnalyzer.$(ObjSuf)
GEPROCESSORO     = GeProcessor.$(ObjSuf)
GE4HEN3PROCESSORO= Ge4Hen3Processor.$(ObjSuf)
GECALIBPROCESSORO= GeCalibProcessor.$(ObjSuf)
HEN3PROCESSORO   = Hen3Processor.$(ObjSuf)
ISSDPROCESSORO   = ImplantSsdProcessor.$(ObjSuf)
INITIALIZEO      = Initialize.$(ObjSuf)
IONCHAMBERPROCESSORO = IonChamberProcessor.$(ObjSuf)
LIQUIDSCINTPROCESSORO = LiquidScintProcessor.$(ObjSuf)
LOGICPROCESSORO  = LogicProcessor.$(ObjSuf)
MAPFILEO         = MapFile.$(ObjSuf)
MESSENGERO       = Messenger.$(ObjSuf)
MCPPROCESSORO    = McpProcessor.$(ObjSuf)
MTCPROCESSORO    = MtcProcessor.$(ObjSuf)
NEUTRONSCINTPROCESSORO  = NeutronScintProcessor.$(ObjSuf)
PLOTSO           = Plots.$(ObjSuf)
PLOTSREGISTERO   = PlotsRegister.$(ObjSuf)
POSITIONPROCESSORO = PositionProcessor.$(ObjSuf)
RANDOMPOOLO      = RandomPool.$(ObjSuf)
RAWEVENTO        = RawEvent.$(ObjSuf)
ROOTPROCESSORO   = RootProcessor.$(ObjSuf)
PATHHOLDERO      = PathHolder.$(ObjSuf)
PLACEBUILDERO    = PlaceBuilder.$(ObjSuf)
PLACESO          = Places.$(ObjSuf)
PULSERPROCESSORO = PulserProcessor.$(ObjSuf)
SSDPROCESSORO    = SsdProcessor.$(ObjSuf)
STATSDATAO       = StatsData.$(ObjSuf)
TAUANALYZERO     = TauAnalyzer.$(ObjSuf)
TIMINGINFOO      = TimingInformation.$(ObjSuf)
TRIGGERLOGICPROCESSORO = TriggerLogicProcessor.$(ObjSuf)
TRACEO           = Trace.$(ObjSuf)
TRACEEXTRACTERO  = TraceExtracter.$(ObjSuf)
TRACEFILTERO     = TraceFilterer.$(ObjSuf)
TRACEPLOTO       = TracePlotter.$(ObjSuf)
TRACESUBO        = TraceAnalyzer.$(ObjSuf)
TREECORRELATORO  = TreeCorrelator.$(ObjSuf)
VANDLEPROCESSORO = VandleProcessor.$(ObjSuf)
VANDLEROOTO      = VandleROOT.$(ObjSuf)
WAVEFORMSUBO     = WaveformAnalyzer.$(ObjSuf)
WAVEFORMSUBO     = WaveformAnalyzer.$(ObjSuf)

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
# Fortran objects
OBJS   = \
$(SET2CCO)\
$(MESSLOGO)\
$(MILDATIMO)\
$(SCANORO)
# Important to compile READBUFFDATA first
OBJS += $(READBUFFDATAO)
# other C++ objects
OBJS += \
$(PUGIXMLO)\
$(PIXIEO)\
$(BETASCINTPROCESSORO)\
$(CORRELATORO)\
$(CHANEVENTO)\
$(CHANIDENTIFIERO)\
$(HISTOGRAMMERO)\
$(DETECTORDRIVERO)\
$(DETECTORLIBRARYO)\
$(DETECTORSUMMARYO)\
$(DOUBLETRACEO)\
$(DSSDPROCESSORO)\
$(EVENTPROCESSORO)\
$(GEPROCESSORO)\
$(GE4HEN3PROCESSORO)\
$(GECALIBPROCESSORO)\
$(HEN3PROCESSORO)\
$(ISSDPROCESSORO)\
$(INITIALIZEO)\
$(IONCHAMBERPROCESSORO)\
$(LIQUIDSCINTPROCESSORO)\
$(LOGICPROCESSORO)\
$(MAPFILEO)\
$(MESSENGERO)\
$(MCPPROCESSORO)\
$(MTCPROCESSORO)\
$(NEUTRONSCINTPROCESSORO)\
$(PATHHOLDERO)\
$(PLOTSO)\
$(PLOTSREGISTERO)\
$(POSITIONPROCESSORO)\
$(RANDOMPOOLO)\
$(RAWEVENTO)\
$(PLACEBUILDERO)\
$(PLACESO)\
$(PULSERPROCESSORO)\
$(ACCUMULATORO)\
$(SSDPROCESSORO)\
$(STATSDATAO)\
$(TAUANALYZERO)\
$(TIMINGINFOO)\
$(TRIGGERLOGICPROCESSORO)\
$(TRACEO)\
$(TRACEEXTRACTERO)\
$(TRACEFILTERO)\
$(TRACEPLOTO)\
$(TRACESUBO)\
$(TREECORRELATORO)\
$(VANDLEPROCESSORO)\
$(WAVEFORMSUBO)\
$(WAVEFORMSUBO)

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

#------------ Compile with Gamma-Gamma gates support in GeProcessor
ifdef GGATES
CXXFLAGS	+= -DGGATES
endif

#--------- Add to list of known file suffixes
.SUFFIXES: .$(cxxSrcSuf) .$(fSrcSuf) .$(c++SrcSuf) .$(cSrcSuf)

.phony: all clean
all:     $(PROGRAMS)

#----------- remove all objects, core and .so file
clean:
	@echo "Cleaning up..."
	@rm -f $(OBJS) $(PIXIE) core *~ src/*~ include/*~ scan/*~ config/*~

tidy:
	@echo "Tidying up..."
	@rm -f $(OBJS) core *~ src/*~ include/*~ scan/*~

#----------- link all created objects together
#----------- to create pixie_ldf_c program
$(PIXIE): $(OBJS) $(LIBS)
	$(LINK.o) $^ -o $@ $(LDLIBS)
