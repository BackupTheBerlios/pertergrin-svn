## Process this file with automake to produce Makefile.in
## Created by Anjuta - will be overwritten
## If you don't want it to overwrite it,
## 	Please disable it in the Anjuta project configuration

SUBDIRS = intl po pixmaps gui kernel 

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

EXTRA_DIST = $(ptgeditordoc_DATA)

all-redirect: all-recursive-am
	mv kernel/ptgeditor .

# Copy all the spec files. Of course, only one is actually used.
dist-hook:
	for specfile in *.spec; do \
		if test -f $$specfile; then \
			cp -p $$specfile $(distdir); \
		fi \
	done

