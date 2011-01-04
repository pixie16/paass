#!/bin/make
# generic definitions required for building code dependent on pixie libs
THISDIR      = $(dir $(lastword $(MAKEFILE_LIST)))
PXI_ROOT     = $(HOME)/Pixie16RevD_General_1.4.3_Update_05072010
LIB_ROOT     = $(HOME)/PLX631/PlxSdk/Linux/PlxApi/Library/
INTERFACE    = $(realpath $(THISDIR)/Interface)
INTERFACELIB = $(INTERFACE)/libPixieInterface.a

INCLUDEDIRS += -I$(PXI_ROOT)/software/inc/ -I$(PXI_ROOT)/software/sys/ -I$(PXI_ROOT)/software/app/ -I./include -I$(INTERFACE)
LIBDIRS     += -L$(PXI_ROOT)/software -L$(LIB_ROOT)
GLOBAL_FLAGS = -D "PLX_LINUX" -D "PCI_CODE" -D "PLX_LITTLE_ENDIAN"
CFLAGS      += $(INCLUDEDIRS) -g -Wall $(GLOBAL_FLAGS)
CXXFLAGS    += $(INCLUDEDIRS) -g -Wall $(GLOBAL_FLAGS)
LDFLAGS     += $(LIBDIRS)
# dl library not necessarily included for new gcc
LDLIBS      += -Wl,-dy -ldl -lm -lPixie16App -lPixie16Sys -lPlxApi