# Switched this back to bibtex because the current version of crosstex
# seems to have a bug causing incorrect capitalization in the titles.

LATEX=pdflatex
TEXFLAGS=

TARGETS=Resubmit.pdf eprint.pdf final.pdf
PARTIALS=main.pdf supplementary.pdf

final: final.build

all: $(TARGETS:.pdf=.build)
	$(LATEX) main.tex
	$(LATEX) supplementary.tex

main: Resubmit.build
	$(LATEX) main.tex

supplementary: Resubmit.build
	$(LATEX) supplementary.tex

eprint: eprint.build

%.build: %.tex
	$(LATEX) $(TEXFLAGS) $<
	bibtex $(basename $<)
	$(LATEX) $(TEXFLAGS) $<
	$(LATEX) $(TEXFLAGS) $<

light:
	pdflatex $(TARGETS:.pdf=.tex)

clean:
	rm -f $(TARGETS) $(PARTIALS) *.log *.aux *.bbl *.blg 
