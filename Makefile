#!/bin/make
# GNUmakefile using implicit rules and standard definitions
SHELL=/bin/sh

# Uncomment the following line for root functionality
# USEROOT = 1
# Uncomment this line for a more verbose scan
# CXXFLAGS += -DVERBOSE
# Undefine to make a "online" version
# ONLINE = 1
# Define to see debugging information for TreeCorrelator
#DEBUG = 1
#Uncomment this line to use the Pulse Fitting routine
PULSEFIT = 1
# Define to use Gamma-Gamma gates in GeProcessor
# GGATES = 1

# We'll do some tests here to find if environment variables have been set
ifeq ($(HHIRF_GFORTRAN),)
HHIRF_GFORTRAN = 1
endif

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

#------- instruct make to search through these
#------- directories to find files
vpath %.f scan/
vpath %.hpp include/:include/experiment
vpath %.h include/
vpath %.icc include/
vpath %.cpp src/analyzers:src/core:src/experiment:src/processors
vpath %.o obj/

LIBS = $(HHIRF_DIR)/scanorlib.a $(HHIRF_DIR)/orphlib.a \
       $(ACQ2_LIBDIR)/acqlib.a  $(ACQ2_LIBDIR)/ipclib.a

OutPutOpt     = -o # keep whitespace after "-o"

#------- define file suffixes
fSrcSuf   = f
cSrcSuf   = c
c++SrcSuf = cpp
cxxSrcSuf = cxx
ObjSuf    = o


#------- define compilers
#define to compile with gfortran (>=4.2) if required for the hhirf libs
ifneq ($(HHIRF_GFORTRAN),1)
FC        = g77
else
FC        = gfortran
endif

GCC       = gcc
CXX       = g++
LINK.o    = $(FC) $(LDFLAGS)

#------- include directories for the pixie c files
CINCLUDEDIRS += -Iinclude

# -Dnewreadout is needed to account for a change to pixie16 readout
# structure change on 03/20/08.  Remove for backwards compatability
#
# for debug and profiling add options -g -pg
# and remove -O
#------- define basic compiler flags, no warnings on code that is not our own
FFLAGS   += -O3
GCCFLAGS += -fPIC $(CINCLUDEDIRS) -Dnewreadout
CXXFLAGS += -Wall -g -fPIC $(CINCLUDEDIRS) -Dnewreadout

ifdef ONLINE
CXXFLAGS += -DONLINE
endif

#------- basic linking instructions
LDLIBS   += -lm -lstdc++
ifdef PULSEFIT
LDLIBS   += -lgsl -lgslcblas
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

#---------- Include the list of objects
include Makefile.objs

#---------- Change the executable name if necessary
ifdef ONLINE
PIXIE = pixie_ldf_c_online$(ExeSuf)
else
PIXIE = pixie_ldf_c$(ExeSuf)
endif

#---------- Adjust compilation if ROOT capability is desired
ifdef USEROOT
CXX_OBJS  += $(ROOTPROCESSORO)
PIXIE = pixie_ldf_c_root$(ExeSuf)
ROOTCONFIG   := root-config

#no uncomment ROOTCLFAGS   := $(filter-out pthread,$(ROOTCFLAGS))
CXXFLAGS     += $(shell $(ROOTCONFIG) --cflags) -Duseroot
LDFLAGS      += $(shell $(ROOTCONFIG) --ldflags)
LDLIBS       += $(shell $(ROOTCONFIG) --libs)
endif

#------------ Compile with Gamma-Gamma gates support in GeProcessor
ifdef GGATES
CXXFLAGS	+= -DGGATES
endif

#------------ Compile with debug information for TreeCorrelator
ifdef GGATES
CXXFLAGS	+= -DDEBUG
endif

#---------- Update some information about the object files
OBJDIR = ./obj
FORT_OBJDIR = $(OBJDIR)/fortran
FORT_OBJS_W_DIR = $(addprefix $(FORT_OBJDIR)/,$(FORT_OBJS))
CXX_OBJDIR = $(OBJDIR)/c++
CXX_OBJS_W_DIR = $(addprefix $(CXX_OBJDIR)/,$(CXX_OBJS))

#--------- Add to list of known file suffixes
.SUFFIXES: .$(cxxSrcSuf) .$(fSrcSuf) .$(c++SrcSuf) .$(cSrcSuf)

all: $(FORT_OBJS_W_DIR) $(CXX_OBJS_W_DIR) $(PIXIE)

$(FORT_OBJS_W_DIR): | $(FORT_OBJDIR)

$(FORT_OBJDIR):
	mkdir -p $(FORT_OBJDIR)

$(FORT_OBJDIR)/%.o: %.f
	$(FC) $(FFLAGS) -c $< -o $@

$(CXX_OBJS_W_DIR): | $(CXX_OBJDIR)

$(CXX_OBJDIR):
	mkdir -p $(CXX_OBJDIR)

$(CXX_OBJDIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

#----------- link all created objects together
#----------- to create pixie_ldf_c program
$(PIXIE): $(FORT_OBJS_W_DIR) $(CXX_OBJS_W_DIR) $(LIBS)
	$(LINK.o) $^ -o $@ $(LDLIBS)

.PHONY: clean tidy doc
clean:
	@echo "Cleaning up..."
	@rm -rf ./$(OBJDIR) $(PIXIE) ./core ./*~ ./*.save \
	./src/*/*~ ./include/*~ ./scan/*~ ./src/*/*.orig ./include/*.orig
tidy:
	@echo "Tidying up..."
	@rm -f ./core ./*~ ./src/*/*~ ./src/*/*.orig ./include/*~ ./include/*.orig ./scan/*~ ./*.save
doc: doc/Doxyfile
	@doxygen $^
