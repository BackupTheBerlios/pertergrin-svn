## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/kernel -I$(top_srcdir)/modules/world \
	$(LIBGLADEMM2_CFLAGS)

noinst_LIBRARIES = libptggui.a

AM_CXXFLAGS =\
	 -Wall -Wimplicit -Wreturn-type -Wunused -Wswitch -Wcomment -Wuninitialized -Wparentheses -Wpointer-arith -Woverloaded-virtual\
	 -g

libptggui_a_SOURCES = \
	mainwindow.cc mainwindow.h \
	worlddialog.cc worlddialog.h \
	mapdrawarea.cc mapdrawarea.h

libptggui_a_includedir = $(pkgincludedir)

libptggui.a: $(libptggui_a_OBJECTS)
	-rm -f libptggui.a
	$(AR) cru libptggui.a $(libptggui_a_OBJECTS)
	$(RANLIB) libptggui.a

clean-compile:
	-rm -f *.$(a)
	-rm -f *.res
	-rm -f *.RES
