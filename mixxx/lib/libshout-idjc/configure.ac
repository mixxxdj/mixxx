# Process this file with autoconf to produce a configure script.
# $Id$

m4_define(libshout_major, 2)
m4_define(libshout_minor, 4)
m4_define(libshout_micro, 1)
m4_define(libshout_version, libshout_major.libshout_minor.libshout_micro)

AC_INIT([libshout-idjc], libshout_version, [www.sourceforge.net/projects/idjc])
AC_PREREQ([2.54])
AC_CONFIG_SRCDIR([src/shout.c])
AM_CONFIG_HEADER(config.h)
# config.h guard
AH_TOP([#ifndef __CONFIG_H__
#define __CONFIG_H__ 1])
AH_BOTTOM([#endif])

AC_DEFINE([LIBSHOUT_MAJOR], libshout_major, [Shout library major version])
AC_DEFINE([LIBSHOUT_MINOR], libshout_minor, [Shout library minor version])
AC_DEFINE([LIBSHOUT_MICRO], libshout_micro, [Shout library patch version])

VERSION=libshout_version

AM_INIT_AUTOMAKE
AM_MAINTAINER_MODE

dnl create our name mangling macro
dnl the prefix must be hardwired because of AH limitations
AH_VERBATIM([_mangle], [
/* name mangling to protect code we share with other libraries */
#define _mangle(proc) _shout_ ## proc
])

AC_PROG_CC
AM_PROG_LIBTOOL

dnl Set some options based on environment

dnl openbsd headers break when _XOPEN_SOURCE is defined but without it seems
dnl to be fine
case "$ac_cv_host" in
   *openbsd* | *solaris* | *irix*)
   ;;
   *) AC_DEFINE(_XOPEN_SOURCE, 600, [Define if you have POSIX and XPG specifications])
   ;;
esac
if test -z "$GCC"; then
        case $host in 
        *-*-irix*)
                DEBUG="-g -signed" 
                CFLAGS="-O2 -w -signed"
                PROFILE="-p -g3 -O2 -signed"
		;;
        sparc-sun-solaris*)
                DEBUG="-v -g"
                CFLAGS="-xO4 -fast -w -fsimple -native -xcg92"
                PROFILE="-v -xpg -g -xO4 -fast -native -fsimple -xcg92 -Dsuncc"
		;;
        *)
                DEBUG="-g"
                CFLAGS="-O"
                PROFILE="-g -p" 
		;;
        esac
else
        XIPH_CFLAGS="-Wall -ffast-math -fsigned-char"
        AC_DEFINE(_GNU_SOURCE, ,[Define if you have POSIX and GNU specifications])
        DEBUG="-g"
        PROFILE="-pg -g"
fi

dnl Checks for programs.

dnl Checks for header files.
AC_HEADER_STDC
AC_HEADER_TIME
AC_CHECK_HEADERS([strings.h sys/timeb.h arpa/inet.h])

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
XIPH_C99_INTTYPES

dnl Checks for library functions.
AC_CHECK_FUNCS([gettimeofday ftime])
AC_SEARCH_LIBS([nanosleep], [rt],
  [AC_DEFINE([HAVE_NANOSLEEP], [1],
    [Define if you have the nanosleep function])])

dnl Module checks
XIPH_NET

dnl Extra dependencies
AC_ARG_ENABLE([thread],
  AC_HELP_STRING([--disable-thread],[do not build with thread support even if it is available]))

SHOUT_THREADSAFE="0"
if test "$enable_thread" != "no"
then
  ACX_PTHREAD([
    LIBS="$LIBS $PTHREAD_LIBS"
    XIPH_CFLAGS="$XIPH_CFLAGS $PTHREAD_CFLAGS $PTHREAD_CPPFLAGS"
    CC="$PTHREAD_CC"
    SHOUT_THREADSAFE="1"
    ])
fi
AC_SUBST([SHOUT_THREADSAFE])
AM_CONDITIONAL([HAVE_THREAD], [test "$SHOUT_THREADSAFE" = "1"])
if test "$SHOUT_THREADSAFE" != "1"
then
  AC_DEFINE([NO_THREAD], 1, [Define if you don't want to use the thread library])
fi

SHOUT_REQUIRES="ogg"

PKG_CHECK_MODULES(VORBIS, vorbis, [
    HAVE_VORBIS="yes"
    SHOUT_REQUIRES="$SHOUT_REQUIRES, vorbis"
  ], [
    XIPH_PATH_VORBIS(, [AC_MSG_ERROR([required Ogg Vorbis library not found])])
  ])
VORBIS_LIBS="$VORBIS_LDFLAGS $VORBIS_LIBS"
XIPH_CFLAGS="$XIPH_CFLAGS $VORBIS_CFLAGS"

AC_ARG_ENABLE([theora],
  AC_HELP_STRING([--disable-theora],[do not build with Theora support]))

if test "x$enable_theora" != "xno"; then
PKG_CHECK_MODULES(THEORA, theora, [
    HAVE_THEORA="yes"
    SHOUT_REQUIRES="$SHOUT_REQUIRES, theora"
  ], [
    XIPH_PATH_THEORA(, [AC_MSG_WARN([Theora library not found, disabling])])
  ])
fi
XIPH_VAR_APPEND([XIPH_CPPFLAGS],[$THEORA_CFLAGS])
XIPH_VAR_PREPEND([XIPH_LIBS],[$THEORA LDFLAGS $THEORA_LIBS])
AM_CONDITIONAL([HAVE_THEORA], [test -n "$THEORA_LIBS"])
if test -n "$THEORA_LIBS"
then
  AC_DEFINE([HAVE_THEORA], 1, [Define if you want theora streams supported])
fi

AC_ARG_ENABLE([speex],
  AC_HELP_STRING([--disable-speex],[do not build with Speex support]))

if test "x$enable_speex" != "xno"; then
PKG_CHECK_MODULES(SPEEX, speex, [
    HAVE_SPEEX="yes"
    SHOUT_REQUIRES="$SHOUT_REQUIRES, speex"
  ], [
    XIPH_PATH_SPEEX(, [AC_MSG_WARN([Speex library not found, disabling])])
  ])
fi
XIPH_VAR_APPEND([XIPH_CPPFLAGS],[$SPEEX_CFLAGS])
XIPH_VAR_PREPEND([XIPH_LIBS],[$SPEEX LDFLAGS $SPEEX_LIBS])
AM_CONDITIONAL([HAVE_SPEEX], [test -n "$SPEEX_LIBS"])
if test -n "$SPEEX_LIBS"
then
  AC_DEFINE([HAVE_SPEEX], 1, [Define if you want speex streams supported])
fi

dnl If pkgconfig is found, install a shout.pc file.

AC_ARG_ENABLE([pkgconfig],
  AC_HELP_STRING([--disable-pkgconfig],[disable pkgconfig data files (auto)]),
  [dopkgconfig="$enableval"], [dopkgconfig="maybe"])
if test "$dopkgconfig" = "maybe"
then
  AC_CHECK_PROG([PKGCONFIG], [pkg-config], [yes], [no])
else
  AC_MSG_CHECKING([whether pkgconfig should be used])
  PKGCONFIG="$dopkgconfig"
  AC_MSG_RESULT([$PKGCONFIG])
fi
AM_CONDITIONAL([HAVE_PKGCONFIG], [test "$PKGCONFIG" != "no"])

# Collect flags for shout.pc

# I hate myself for doing this.
save_prefix="$prefix"
if test "$prefix" = "NONE"
then
  prefix="$ac_default_prefix"
fi
eval shout_includedir="$includedir"
prefix="$save_prefix"

XIPH_PATH_OPENSSL([
    XIPH_VAR_APPEND([XIPH_CPPFLAGS],[$OPENSSL_CFLAGS])
    XIPH_VAR_APPEND([XIPH_LDFLAGS],[$OPENSSL_LDFLAGS])
    XIPH_VAR_PREPEND([XIPH_LIBS],[$OPENSSL_LIBS])
    SHOUT_TLS="1"
    ],
    [ AC_MSG_NOTICE([SSL disabled!])
      SHOUT_TLS="0"
    ])
AC_SUBST([SHOUT_TLS])
AM_CONDITIONAL([HAVE_TLS], [test -n "$OPENSSL_LIBS"])

SHOUT_VERSION="$VERSION"
SHOUT_CPPFLAGS="-I$shout_includedir $VORBIS_CFLAGS $PTHREAD_CPPFLAGS"
SHOUT_CFLAGS="$PTHREAD_CFLAGS"
SHOUT_LIBS="-lshout"

XIPH_CLEAN_CCFLAGS([$SHOUT_CPPFLAGS], [SHOUT_CPPFLAGS])
XIPH_CLEAN_CCFLAGS([$SHOUT_CFLAGS], [SHOUT_CFLAGS])
XIPH_CLEAN_CCFLAGS([$VORBIS_LIBS $THEORA_LIBS $SPEEX_LIBS $PTHREAD_LIBS $OPENSSL_LIBS $OPENSSL_LIBS $LIBS], [SHOUT_LIBDEPS])
AC_SUBST(PTHREAD_CPPFLAGS)
AC_SUBST(SHOUT_LIBDEPS)
AC_SUBST(SHOUT_REQUIRES)
AC_SUBST(SHOUT_CPPFLAGS)
AC_SUBST(SHOUT_CFLAGS)

dnl Make substitutions

AC_SUBST(LIBTOOL_DEPS)
AC_SUBST(OPT)
AC_SUBST(LIBS)
AC_SUBST(DEBUG)
AC_SUBST(CFLAGS)
AC_SUBST(PROFILE)
AC_SUBST(XIPH_CFLAGS)
AC_SUBST(XIPH_CPPFLAGS)
AC_SUBST(XIPH_LIBS)

AC_OUTPUT([Makefile include/Makefile include/shoutidjc/Makefile
  include/shoutidjc/shout.h src/Makefile src/common/net/Makefile src/common/timing/Makefile
  src/common/thread/Makefile src/common/avl/Makefile src/common/httpp/Makefile doc/Makefile
  examples/Makefile win32/Makefile shout-idjc.pc])
