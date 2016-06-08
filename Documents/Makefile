#!/bin/make
PDFTARGETS = daq.pdf
PSTARGETS  = daq.ps
INTERMEDIATES = $(PDFTARGETS:.pdf=.aux) $(PDFTARGETS:.pdf=.dvi) \
	$(PDFTARGETS:.pdf=.log) $(PDFTARGETS:.pdf=.toc)

.phony: all clean veryclean
all: $(PDFTARGETS)

%.dvi: %.tex
	latex $<
	latex $<
	latex $<

%.ps: %.dvi
	dvips $<

%.pdf: %.tex
	pdflatex $<
	pdflatex $<
	pdflatex $<

clean:
	@$(RM) $(INTERMEDIATES) *~
veryclean: clean
	@$(RM) $(PDFTARGETS) $(PSTARGETS)
