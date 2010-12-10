SUBDIRS = Interface Poll Setup
DOCDIRS = Documents
ALLDIRS = $(SUBDIRS) $(DOCDIRS)
LASTDIR = $(notdir $(CURDIR))
DISTNAME = PixieSuite
DISTTARGETS = Documents Interface Poll Setup makepaw.inc makepixie.inc Makefile

.PHONY: subdirs $(ALLDIRS) clean dist docs
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

dist: clean
	@mkdir $(DISTNAME)
	@cp -R $(DISTTARGETS) $(DISTNAME)
	@tar -czf $(DISTNAME)-`date +%d%m%y`.tgz $(DISTNAME) --exclude=*~
	@$(RM) -r $(DISTNAME)