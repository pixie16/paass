#!/bin/make

# CERNLIB_ROOT = /cernlib/2005
# CERNLIB_LIB  = /usr/lib64/cernlib/2006/lib
# CERNLIB_INC  = /usr/include/cernlib/2006
CERNLIB_LIB = /cernlib/2006b/x86_64-slc5-gcc34-opt/lib
CERNLIB_INC = /cernlib/2006b/include

LIBDIRS += -L$(CERNLIB_LIB)
# LDLIBS  += -Wl,-static -lpacklib -lkernlib -Wl,-dy -lgfortran
LDLIBS += -Wl,-static -lpacklib -lkernlib -Wl,-dy -lg2c
INCLUDEDIRS += -I$(CERNLIB_INC)
CFLAGS   += -Df2cFortran
CXXFLAGS += -Df2cFortran
# CFLAGS      += -Dgfortran
# CXXFLAGS    += -Dgfortran

LINK.o = $(CXX) $(LDFLAGS) $(TARGET_ARCH)