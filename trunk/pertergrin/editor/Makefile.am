## Process this file with automake to produce Makefile.in
## Created by Anjuta - will be overwritten
## If you don't want it to overwrite it,
## 	Please disable it in the Anjuta project configuration

SUBDIRS = intl po pixmaps

INCLUDES = -Igui -Imodules \
	$(LIBGLADEMM2_CFLAGS)

ptgeditor_CFLAGS =\
	 -Wall -Wimplicit -Wreturn-type -Wunused -Wswitch -Wcomment -Wuninitialized -Wparentheses -Wtraditional -Wpointer-arith -Wmissing-prototypes -Woverloaded-virtual\
	 -O1\
	 -g

ptgeditor_CXXFLAGS =\
	 -Wall -Wimplicit -Wreturn-type -Wunused -Wswitch -Wcomment -Wuninitialized -Wparentheses -Wtraditional -Wpointer-arith -Wmissing-prototypes -Woverloaded-virtual\
	 -O1\
	 -g

bin_PROGRAMS = ptgeditor

ptgeditor_SOURCES = \
	kernel/main.cc \
	gui/mainwindow.cc gui/mainwindow.h \
	gui/worlddialog.cc gui/worlddialog.h

ptgeditor_LDFLAGS =

ptgeditor_LDADD = \
	$(LIBGLADEMM2_LIBS)

ptgeditor_includedir = $(pkgincludedir)

EXTRA_DIST = $(ptgeditordoc_DATA)

ptgeditordocdir = ${prefix}/doc/Pertergrin
ptgeditordoc_DATA = \
	README\
	COPYING\
	AUTHORS\
	ChangeLog\
	INSTALL\
	NEWS\
	TODO\
	ABOUT-NLS

# Copy all the spec files. Of cource, only one is actually used.
dist-hook:
	for specfile in *.spec; do \
		if test -f $$specfile; then \
			cp -p $$specfile $(distdir); \
		fi \
	done

