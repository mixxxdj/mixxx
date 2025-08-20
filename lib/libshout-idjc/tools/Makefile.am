AUTOMAKE_OPTIONS = foreign

EXTRA_DIST = shoutidjc.1

bin_PROGRAMS = shoutidjc
AM_CFLAGS = @XIPH_CFLAGS@ -I$(top_builddir)/include -Wall -Wextra

man1_MANS = shoutidjc.1

shoutidjc_SOURCES = shout.c
shoutidjc_LDADD = $(top_builddir)/src/libshout-idjc.la
