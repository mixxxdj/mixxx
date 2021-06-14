# Configure paths for libspeex
# updated by Karl Heyes 02-Feb-2004

dnl XIPH_PATH_SPEEX([ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
dnl Test for libspeex, and define SPEEX_CFLAGS SPEEX_LIBS
dnl SPEEX_SPEEXENC_LIBS SPEEX_SPEEXFILE_LIBS SPEEX_LDFLAGS
dnl

AC_DEFUN([XIPH_PATH_SPEEX],
[
AC_REQUIRE([XIPH_PATH_OGG])

dnl Get the cflags and libraries for speex
dnl
AC_ARG_VAR([SPEEX],[path to speex installation])
AC_ARG_WITH(speex,
    AC_HELP_STRING([--with-speex=PREFIX],
        [Prefix where libspeex is installed (optional)]),
    speex_prefix="$withval",
    speex_prefix="$SPEEX_PREFIX"
    )
if test "x$with_speex" = "xno"
then
  AC_MSG_RESULT([Speex support disabled by request])
else
  if test "x$speex_prefix" = "x" -o "x$speex_prefix" = "xyes"; then
      if test "x$prefix" = "xNONE"; then
          speex_prefix="/usr/local"
      else
          speex_prefix="$prefix"
      fi
  fi

  SPEEX_CFLAGS="$OGG_CFLAGS"
  SPEEX_LDFLAGS="$OGG_LDFLAGS"
  if test "x$speex_prefix" != "x$ogg_prefix"; then
      XIPH_GCC_WARNING(-I"$speex_prefix/include",,
              [SPEEX_CFLAGS="$SPEEX_CFLAGS -I$speex_prefix/include"
              SPEEX_LDFLAGS="-L$speex_prefix/lib $SPEEX_LDFLAGS"
              ])
  fi

  SPEEX_LIBS="-lspeex"

  xt_save_LIBS="$LIBS"
  xt_save_LDFLAGS="$LDFLAGS"
  LDFLAGS="$LDFLAGS $SPEEX_LDFLAGS"
  LIBS="$LIBS $SPEEX_LIBS"
  xt_have_speex="yes"
  AC_MSG_CHECKING([for libspeex])
  AC_TRY_LINK_FUNC(ogg_stream_init, [AC_MSG_RESULT([ok])],
          [LIBS="$LIBS $OGG_LIBS"
          AC_TRY_LINK_FUNC(ogg_stream_init,
              [SPEEX_LIBS="$SPEEX_LIBS $OGG_LIBS"],
              [xt_have_speex="no"])
          ])
  if test "x$xt_have_speex" = "xyes"
  then
      AC_LINK_IFELSE([AC_LANG_PROGRAM(
                  [#include <speex/speex_header.h>],
                  [void *p = speex_packet_to_header;])],
              [],
              [xt_have_speex="no"])
  fi

  LIBS="$xt_save_LIBS"
  LDFLAGS="$xt_save_LDFLAGS"
  
  if test "x$xt_have_speex" = "xyes"
  then
      AC_MSG_RESULT([ok])
      AC_DEFINE([HAVE_SPEEX],[1],[Define if Speex support is available])
      $1
  else
      ifelse([$2], , AC_MSG_ERROR([Unable to link to libspeex]), [$2])
      SPEEX_CFLAGS=""
      SPEEX_LDFLAGS=""
      SPEEX_LIBS=""
  fi
  AC_SUBST(SPEEX_CFLAGS)
  AC_SUBST(SPEEX_LDFLAGS)
  AC_SUBST(SPEEX_LIBS)
fi
])
