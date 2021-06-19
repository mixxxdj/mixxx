## Process this with automake to create Makefile.in

AUTOMAKE_OPTIONS = foreign

EXTRA_DIST = BUILDING COPYING README TODO

noinst_LTLIBRARIES = libicethread.la
noinst_HEADERS = thread.h

libicethread_la_SOURCES = thread.c
libicethread_la_CFLAGS = @XIPH_CFLAGS@

AM_CPPFLAGS = -I$(srcdir)/..

debug:
	$(MAKE) all CFLAGS="@DEBUG@"

profile:
	$(MAKE) all CFLAGS="@PROFILE@"

