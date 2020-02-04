## Process this file with automake to create Makefile.in

AUTOMAKE_OPTIONS = foreign

noinst_PROGRAMS = example nonblocking

example_SOURCES = example.c
example_LDADD = $(top_builddir)/src/libshout.la @SHOUT_LIBDEPS@

nonblocking_SOURCES = nonblocking.c
nonblocking_LDADD = $(top_builddir)/src/libshout.la @SHOUT_LIBDEPS@

AM_CFLAGS = @XIPH_CFLAGS@
AM_CPPFLAGS = @XIPH_CPPFLAGS@ -I$(top_builddir)/include
