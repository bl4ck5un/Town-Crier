dist_doc_DATA=\
../AUTHORS\
../ChangeLog\
../COPYING.GPL\
../COPYING.LGPL\
../LICENSE\
../NEWS\
../README\
coding_style.txt\
netstrings.txt


if HAVE_DOXYGEN

noinst_HEADERS=$(srcdir)/doxygen-main.h $(top_srcdir)/Doxyfile 

dist_html_DATA=$(builddir)/html/*

$(dist_html_DATA): ../Doxyfile doxygen-main.h
	sed -e 's:src/:$(top_srcdir)/src/:' -e 's:doc/::' -e 's:doxygen-main.h:$(top_srcdir)/doc/doxygen-main.h:' -e 's:include/:$(top_srcdir)/include/:' $< | $(DOXYGEN) $(DOXYFLAGS) -

clean-local:
	-rm -rf html doxywarn.txt doxygen.pyc

endif
