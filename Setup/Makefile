#!/bin/make
include ../makepixie.inc

ifeq ($(USE_PAW),)
USE_PAW=0
endif

SETFILE:=$(shell grep DspSetFile ../Setup/pixie.cfg | awk '{if(!/\#/) print $$2}')

vpath %.cpp src

ADJUSTOBJS    = adjust_offsets.o
FINDOBJS      = find_tau.o
COPYOBJS      = copy_params.o
PREADOBJS     = pread.o
PWRITEOBJS    = pwrite.o
PMREADOBJS    = pmread.o
PMWRITEOBJS   = pmwrite.o
RATEOBJS      = rate.o 
BOOTOBJS      = boot.o
TRACEOBJS     = trace.o
GETTRACESOBJS = get_traces.o
CSRTESTOBJS   = csr_test.o
TGGAINOBJS    = toggle_gain.o
TGGOODOBJS    = toggle_good.o
TGPOLOBJS     = toggle_polarity.o
TGTRACEOBJS   = toggle_trace.o
TGPILEUPOBJS  = toggle_pileup.o
TGCATCHEROBJS = toggle_catcher.o
SETSTDOBJS    = set_standard.o
SETPUONLYOBJS = set_pileups_only.o
SETPURJCTOBJS = set_pileups_reject.o
SETHYBRIDOBJS = set_hybrid.o

COMMONOBJS  = $(INTERFACELIB)

TARGETS = pmread pmwrite pread pwrite rate adjust_offsets copy_params boot \
	trace csr_test find_tau toggle_gain toggle_good toggle_polarity \
	toggle_trace toggle_pileup toggle_catcher set_standard set_hybrid \
	set_pileups_only set_pileups_reject get_traces

CHANNEL_PARMS = TRIGGER_RISETIME TRIGGER_FLATTOP TRIGGER_THRESHOLD ENERGY_RISETIME \
	ENERGY_FLATTOP TAU TRACE_LENGTH TRACE_DELAY VOFFSET XDT BASELINE_PERCENT \
	EMIN BINFACTOR CHANNEL_CSRA CHANNEL_CSRB BLCUT ENERGYCUTOFF ExternDelayLen \
	ExtTrigStretch ChanTrigStretch FtrigoutDelay FASTTRIGBACKLEN
MODULE_PARMS = MODULE_CSRA MODULE_CSRB MODULE_FORMAT MAX_EVENTS SYNCH_WAIT \
	IN_SYNCH SLOW_FILTER_RANGE FAST_FILTER_RANGE MODULE_NUMBER TrigConfig0 \
	TrigConfig1 TrigConfig2

MODULE_RLINKS  = $(foreach parm,$(MODULE_PARMS),R_$(parm))
MODULE_WLINKS  = $(foreach parm,$(MODULE_PARMS),W_$(parm))
CHANNEL_RLINKS = $(foreach parm,$(CHANNEL_PARMS),R_$(parm))
CHANNEL_WLINKS = $(foreach parm,$(CHANNEL_PARMS),W_$(parm))

LINKS = $(MODULE_RLINKS) $(MODULE_WLINKS) $(CHANNEL_RLINKS) $(CHANNEL_WLINKS) \
	pxisys.ini slot_def.set current.set
# make things copasetic with c++
LINK.o = $(CXX) $(LDFLAGS) $(TARGET_ARCH)

ifeq (0,$(USE_PAW))
PLOTTER = $(MAKE) --no-print-directory -f Makefile.mca
else
PLOTTER = $(MAKE) --no-print-directory -f Makefile.paw
endif

.phony: clean all links veryclean plotter progs
all: progs links current.set

progs: $(TARGETS) plotter

plotter:
	@$(PLOTTER)

adjust_offsets: $(ADJUSTOBJS) $(COMMONOBJS)
find_tau: $(FINDOBJS) $(COMMONOBJS)
pmread: $(PMREADOBJS) $(COMMONOBJS)
pmwrite: $(PMWRITEOBJS) $(COMMONOBJS)
pread: $(PREADOBJS) $(COMMONOBJS)
pwrite: $(PWRITEOBJS) $(COMMONOBJS)
copy_params: $(COPYOBJS) $(COMMONOBJS)
rate: $(RATEOBJS) $(COMMONOBJS)
boot: $(BOOTOBJS) $(COMMONOBJS)
trace: $(TRACEOBJS) $(COMMONOBJS)
get_traces: $(GETTRACESOBJS) $(COMMONOBJS)
toggle_gain: $(TGGAINOBJS) $(COMMONOBJS)
toggle_good: $(TGGOODOBJS) $(COMMONOBJS)
toggle_polarity: $(TGPOLOBJS) $(COMMONOBJS)
toggle_trace: $(TGTRACEOBJS) $(COMMONOBJS)
toggle_catcher: $(TGCATCHEROBJS) $(COMMONOBJS)
toggle_pileup: $(TGPILEUPOBJS) $(COMMONOBJS)
set_standard: $(SETSTDOBJS) $(COMMONOBJS)
set_hybrid: $(SETHYBRIDOBJS) $(COMMONOBJS)
set_pileups_reject: $(SETPURJCTOBJS) $(COMMONOBJS)
set_pileups_only: $(SETPUONLYOBJS) $(COMMONOBJS)
csr_test: $(CSRTESTOBJS)

clean: 
	@echo "Cleaning up..."
	@$(RM) *.o $(TARGETS) $(LINKS) *~
	@$(PLOTTER) $@

links: $(LINKS)
	@echo "Creating links..."

$(MODULE_RLINKS):
	@ln -s RMOD $@
$(MODULE_WLINKS):
	@ln -s WMOD $@
$(CHANNEL_RLINKS):
	@ln -s RCHA $@
$(CHANNEL_WLINKS):
	@ln -s WCHA $@
pxisys.ini:
	@ln -s $(PXI_ROOT)/test/$@ .
current.set:
	@ln -s $(PXI_ROOT)/$(SETFILE) $@
slot_def.set:
	@ln -s $(PXI_ROOT)/configuration/$@ .
