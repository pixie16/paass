SUBDIRS = Interface Poll Setup
LASTDIR = $(notdir $(CURDIR))
DISTNAME = PixieSuite
DISTTARGETS = doc Interface Poll Setup makepaw.inc makepixie.inc Makefile

.PHONY: subdirs $(SUBDIRS) clean dist
subdirs: $(SUBDIRS)

Poll Setup: Interface

$(SUBDIRS):
	@$(MAKE) -C $@

clean:
	@$(RM) *~
	@for dir in $(SUBDIRS); do \
	$(MAKE) -C $$dir $@; \
	done

dist: clean
	@mkdir $(DISTNAME)
	@cp -R $(DISTTARGETS) $(DISTNAME)
	@tar -czf $(DISTNAME)-`date +%d%m%y`.tgz $(DISTNAME) --exclude=*~
	@$(RM) -r $(DISTNAME)