#!/bin/make
#a simple make file to create the necessary links for 
#parsing the default.set into a human readable format

include ../makepixie.inc

VARFILE:=$(shell grep DspVarFile ../Setup/pixie.cfg | awk '{if(!/\#/) print $$2}')
SETFILE:=$(shell grep DspSetFile ../Setup/pixie.cfg | awk '{if(!/\#/) print $$2}')

LINKS = setFile varFile

all: links

links: $(LINKS)
	@echo "Creating Links...."
setFile:
	ln -sf $(PXI_ROOT)/$(SETFILE) $@
varFile:
	ln -sf $(PXI_ROOT)/$(VARFILE) $@

.phony: clean

clean: 
	@echo "Removing Links...."
	@rm -f $(LINKS)
