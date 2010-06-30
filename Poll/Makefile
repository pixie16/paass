#nice and simple make file using typical implicit rules

include ../makepixie.inc

vpath %.cpp src
vpath %.c src

COMMONOBJS  = $(INTERFACELIB) $(INTERFACE)/Utility.o
POLLOBJS    = poll.o sendbuf.o spkt_io_udp.o
COMMOBJS    = commtest.o sendbuf.o spkt_io_udp.o
OBJS        = $(POLLOBJS) $(COMMOBJS)
TARGETS     = poll commtest

# make things copasetic with c++
LINK.o      = $(CXX) $(LDFLAGS) $(TARGET_ARCH)

.phony: clean all
all: $(TARGETS)

poll: $(POLLOBJS) $(COMMONOBJS)

commtest: $(COMMOBJS)

clean: 
	@echo "Cleaning up..."
	@$(RM) $(OBJS) $(TARGETS) *~