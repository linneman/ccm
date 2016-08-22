# Makefile.am - Automake script for tcm

ACLOCAL_AMFLAGS = -I m4

if HAVE_DOXYGEN
DOXYDIR=doc/doxygen
endif

tcmconfdir = $(sysconfdir)
tcmconf_DATA = tcm.rc
EXTRA_DIST = $(tcmconf_DATA)

SUBDIRS = src $(DOXYDIR)
DIST_SUBDIRS=src $(DOXYDIR)
