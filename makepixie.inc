#!/bin/make
# generic definitions required for building code dependent on pixie libs
THISDIR      = $(dir $(lastword $(MAKEFILE_LIST)))

# Set these if they have not been identified in the environment
ifeq ($(PXI_ROOT),)	
PXI_ROOT     = $(HOME)/Pixie16RevD_General_1.4.3_Update_04072011
endif
ifeq ($(PLX_SDK_DIR),)
PLX_SDK_DIR  = $(HOME)/PLX/PlxSdk
endif

LIB_ROOT     = $(PLX_SDK_DIR)/Linux/PlxApi/Library
INTERFACE    = $(realpath $(THISDIR)/Interface)
INTERFACELIB = $(INTERFACE)/libPixieInterface.a

INCLUDEDIRS += -I$(PXI_ROOT)/software/inc -I$(PXI_ROOT)/software/sys -I$(PXI_ROOT)/software/app -Iinclude -I$(INTERFACE)
LIBDIRS     += -L$(PXI_ROOT)/software -L$(LIB_ROOT)
GLOBAL_FLAGS = -D "PLX_LINUX" -D "PCI_CODE" -D "PLX_LITTLE_ENDIAN"
CFLAGS      += $(INCLUDEDIRS) -g -Wall $(GLOBAL_FLAGS)
CXXFLAGS    += $(INCLUDEDIRS) -g -Wall $(GLOBAL_FLAGS)
LDFLAGS     += $(LIBDIRS)
# dl library not necessarily included for new gcc
LDLIBS      += -Wl,-dy -ldl -lm -lPixie16App -lPixie16Sys -lPlxApi