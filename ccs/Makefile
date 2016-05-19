# Switched this back to bibtex because the current version of crosstex
# seems to have a bug causing incorrect capitalization in the titles.

LATEX=pdflatex
TEXFLAGS=

TARGET=resubmit.pdf 

all: $(TARGET)

$(TARGET): $(TARGET:.pdf=.tex) *.tex *.bib 
#	epstopdf figures/impl.eps
	$(LATEX) $(TEXFLAGS) $<
	$(LATEX) $(TEXFLAGS) $<
	bibtex $(basename $<)
	bibtex $(basename $<)
	$(LATEX) $(TEXFLAGS) $<
	$(LATEX) $(TEXFLAGS) $<

light:
	pdflatex $(TARGET:.pdf=.tex)


clean:
	rm -f $(TARGET) *.log *.aux *.bbl *.blg 
