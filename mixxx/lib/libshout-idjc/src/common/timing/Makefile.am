## Process this with automake to create Makefile.in

AUTOMAKE_OPTIONS = foreign

EXTRA_DIST = BUILDING COPYING README TODO

noinst_LTLIBRARIES = libicetiming.la
noinst_HEADERS = timing.h

libicetiming_la_SOURCES = timing.c
libicetiming_la_CFLAGS = @XIPH_CFLAGS@

debug:
	$(MAKE) all CFLAGS="@DEBUG@"

profile:
	$(MAKE) all CFLAGS="@PROFILE@"

