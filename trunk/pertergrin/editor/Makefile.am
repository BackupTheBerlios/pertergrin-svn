## Process this file with automake to produce Makefile.in

SUBDIRS = mapeditwidget

INCLUDES = \
	-I$(top_srcdir)/intl \
	@GTK_CFLAGS@ @GDK_PIXBUF_CFLAGS@ @LIBGLADE_CFLAGS@

bin_PROGRAMS = ptgeditor

ptgeditor_SOURCES = \
	editor.c \
	support.c support.h \
	files.c files.h \
	gui.c gui.h \
	callbacks.c callbacks.h

ptgeditor_LDADD = @GTK_LIBS@ @GDK_PIXBUF_LIBS@ @LIBGLADE_LIBS@ $(INTLLIBS) \
	 mapeditwidget/mapeditclass.o

install-data-local:
	@$(NORMAL_INSTALL)
	if test -d $(srcdir)/pixmaps; then \
	  $(mkinstalldirs) $(DESTDIR)$(pkgdatadir)/pixmaps; \
	  for pixmap in $(srcdir)/pixmaps/*; do \
	    if test -f $$pixmap; then \
	      $(INSTALL_DATA) $$pixmap $(DESTDIR)$(pkgdatadir)/pixmaps; \
	    fi \
	  done \
	fi

dist-hook:
	if test -d pixmaps; then \
	  mkdir $(distdir)/pixmaps; \
	  for pixmap in pixmaps/*; do \
	    if test -f $$pixmap; then \
	      cp -p $$pixmap $(distdir)/pixmaps; \
	    fi \
	  done \
	fi

