## Process this with automake to create Makefile.in

AUTOMAKE_OPTIONS = foreign

EXTRA_DIST = BUILDING COPYING README TODO avl.dsp test.c

noinst_LTLIBRARIES = libiceavl.la
noinst_HEADERS = avl.h

libiceavl_la_SOURCES = avl.c
libiceavl_la_CFLAGS = @XIPH_CFLAGS@

INCLUDES = -I$(srcdir)/..

debug:
	$(MAKE) all CFLAGS="@DEBUG@"

profile:
	$(MAKE) all CFLAGS="@PROFILE@"

