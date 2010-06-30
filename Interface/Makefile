include ../makepixie.inc

CC            = g++  # make sure everything is copasetic with c++
INTERFACEOBJS = PixieInterface.o Lock.o Display.o
TESTOBJS      = test.o libPixieInterface.a
UTILITYOBJS   = Utility.o
OBJS          = $(TESTOBJS) $(UTILITYOBJS)
TARGETS       = test libpixieInterface.a

test: $(TESTOBJS)

libPixieInterface.a: $(INTERFACEOBJS)
	ar -rc $@ $^

.PHONY: clean
clean:
	@echo "Cleaning up..."
	@$(RM) $(OBJS) $(INTERFACEOBJS) $(TARGETS) *~
