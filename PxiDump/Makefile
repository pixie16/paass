#!/bin/make
#a simple make file to create the necessary links for 
#parsing the default.set into a human readable format

include ../makepixie.inc

LINKS = current.set varFile.var

all: links

links: $(LINKS)
	@echo "Creating Links...."
current.set:
	@ln -s $(PXI_ROOT)/configuration/default.set $@
varFile.var:
	@ln -s $(VARFILE) $@

.phony: clean

clean: 
	@echo "Removing Links...."
	rm -f $(LINKS)
