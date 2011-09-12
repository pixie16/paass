#!/bin/make

# Set some variables if they are not defined in the environment
ifneq ($(CERNLIB_ROOT),)
ifeq ($(CERNLIB_LIB),)
CERNLIB_LIB = $(CERNLIB_ROOT)/lib
endif
ifeq ($(CERNLIB_INC),)
CERNLIB_INC = $(CERNLIB_ROOT)/include
endif
else
ifeq ($(CERNLIB_LIB),)
# default to the "trurl" location
CERNLIB_LIB = /cernlib/2006b/x86_64-slc5-gcc34-opt/lib
endif
ifeq ($(CERNLIB_INC),)
# default to the "trurl" location
CERNLIB_INC = /cernlib/2006b/include
endif
endif

LIBDIRS += -L$(CERNLIB_LIB)
LDLIBS  += -Wl,-static -lpacklib -lkernlib -Wl,-dy
ifneq ($(CERNLIB_GFORTRAN),)
LDLIBS   += -lgfortran
# Necessary for some versions of "cfortran.h"
# CFLAGS   += -Dgfortran
# CXXFLAGS += -Dgfortran
CFLAGS   += -Df2cFortran
CXXFLAGS += -Df2cFortran
else
LDLIBS   += -lg2c
CFLAGS   += -Df2cFortran
CXXFLAGS += -Df2cFortran
endif

INCLUDEDIRS += -I$(CERNLIB_INC)
LINK.o = $(CXX) $(LDFLAGS) $(TARGET_ARCH)