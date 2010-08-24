#nice and simple make file using typical implicit rules

include ../makepixie.inc

vpath %.cpp src

ADJUSTOBJS  =   adjust_offsets.o
FINDOBJS      = find_tau.o
COPYOBJS      = copy_params.o
PREADOBJS     = pread.o
PWRITEOBJS    = pwrite.o
PMREADOBJS    = pmread.o
PMWRITEOBJS   = pmwrite.o
RATEOBJS      = rate.o 
BOOTOBJS      = boot.o
TRACEOBJS     = trace.o
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

OBJS        = $(PMREADOBJS) $(PMWRITEOBJS) $(PREADOBJS) $(PWRITEOBJS) $(RATEOBJS) $(BOOTOBJS) $(ADJUSTOBJS) $(COPYOBJS) $(TRACEOBJS) $(CSRTESTOBJS) $(FINDOBJS) $(TGGAINOBJS) $(TGGOODOBJS) $(TGPOLOBJS) $(TGTRACEOBJS) $(TGPILEUPOBJS) $(TGCATCHEROBJS) $(SETSTDOBJS) $(SETPUONLYOBJS) $(SETPURJCTOBJS) $(SETHYBRIDOBJS)

TARGETS     = pmread pmwrite pread pwrite rate adjust_offsets copy_params boot trace csr_test find_tau toggle_gain toggle_good toggle_polarity toggle_trace toggle_pileup toggle_catcher set_standard set_hybrid set_pileups_only set_pileups_reject

CHANNEL_PARMS = TRIGGER_RISETIME TRIGGER_FLATTOP TRIGGER_THRESHOLD ENERGY_RISETIME ENERGY_FLATTOP TAU TRACE_LENGTH TRACE_DELAY VOFFSET XDT BASELINE_PERCENT EMIN BINFACTOR CHANNEL_CSRA CHANNEL_CSRB BLCUT ENERGYCUTOFF
MODULE_PARMS = MODULE_CSRA MODULE_CSRB MODULE_FORMAT MAX_EVENTS SYNCH_WAIT IN_SYNCH SLOW_FILTER_RANGE FAST_FILTER_RANGE MODULE_NUMBER
MODULE_RLINKS = $(foreach parm,$(MODULE_PARMS),R_$(parm))
MODULE_WLINKS = $(foreach parm,$(MODULE_PARMS),W_$(parm))
CHANNEL_RLINKS = $(foreach parm,$(CHANNEL_PARMS),R_$(parm))
CHANNEL_WLINKS = $(foreach parm,$(CHANNEL_PARMS),W_$(parm))

LINKS = $(MODULE_RLINKS) $(MODULE_WLINKS) $(CHANNEL_RLINKS) $(CHANNEL_WLINKS) pxisys.ini
# make things copasetic with c++
LINK.o      = $(CXX) $(LDFLAGS) $(TARGET_ARCH)

PAWMAKE = $(MAKE) --no-print-directory -f Makefile.paw
.phony: clean all links veryclean paw progs
all: progs links

progs: $(TARGETS) paw

paw:
	@$(PAWMAKE)

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
	@$(RM) $(OBJS) $(TARGETS) $(LINKS) *~
	@$(PAWMAKE) $@

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