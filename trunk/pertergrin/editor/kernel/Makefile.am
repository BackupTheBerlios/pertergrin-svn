## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/gui -I$(top_srcdir)/modules/world \
	$(LIBGLADEMM2_CFLAGS)

noinst_LIBRARIES = libptgkernel.a

AM_CXXFLAGS =\
	 -Wall -Wimplicit -Wreturn-type -Wunused -Wswitch -Wcomment -Wuninitialized -Wparentheses -Wpointer-arith -Woverloaded-virtual\
	 -g

libptgkernel_a_SOURCES = \
	app.cc \
	world.cc

libptgkernel_a_includedir = $(pkgincludedir)

libptgkernel.a: $(libptgkernel_a_OBJECTS)
	-rm -f libptgkernel.a
	$(AR) cru libptgkernel.a $(libptgkernel_a_OBJECTS)
	$(RANLIB) libptgkernel.a

bin_PROGRAMS = ptgeditor

ptgeditor_LDFLAGS =

ptgeditor_SOURCES = main.cc

ptgeditor_LDADD = \
	$(LIBGLADEMM2_LIBS) \
	$(top_srcdir)/gui/libptggui.a \
	$(top_srcdir)/kernel/libptgkernel.a

clean-compile:
	-rm -f *.$(so)
	-rm -f *.res
	-rm -f *.RES
