# CERNLIB_ROOT = /cernlib/2005
CERNLIB_LIB  = /usr/lib64/cernlib/2006/lib
CERNLIB_INC  = /usr/include/cernlib/2006

LIBDIRS += -L$(CERNLIB_LIB)
LDLIBS  += -Wl,-static -lpacklib -lkernlib -Wl,-dy -lgfortran
INCLUDEDIRS += -I$(CERNLIB_INC)
CFLAGS      += -Dgfortran
CXXFLAGS    += -Dgfortran

LINK.o = $(CXX) $(LDFLAGS) $(TARGET_ARCH)