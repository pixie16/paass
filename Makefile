SUBDIRS = Interface Poll Setup PxiDump
DOCDIRS = Documents
ALLDIRS = $(SUBDIRS) $(DOCDIRS)
LASTDIR = $(notdir $(CURDIR))

.PHONY: subdirs $(ALLDIRS) clean docs
subdirs: $(SUBDIRS)
docs: $(DOCDIRS)

Poll Setup: Interface

$(ALLDIRS):
	@$(MAKE) -C $@

clean:
	@$(RM) *~
	@for dir in $(ALLDIRS); do \
	$(MAKE) -C $$dir $@; \
	done
