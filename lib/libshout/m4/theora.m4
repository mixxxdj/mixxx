# Configure paths for libtheora
# Karl Heyes 02-Feb-2004

dnl XIPH_PATH_THEORA([ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
dnl Test for libtheora, and define THEORA_CFLAGS THEORA_LIBS
dnl

AC_DEFUN([XIPH_PATH_THEORA],
[
AC_REQUIRE([XIPH_PATH_OGG])

dnl Get the cflags and libraries for theora
dnl
AC_ARG_VAR([THEORA],[path to theora installation])
AC_ARG_WITH(theora,
    AC_HELP_STRING([--with-theora=PREFIX],
        [Prefix where libtheora is installed (optional)]),
    theora_prefix="$withval",
    theora_prefix="$THEORA_PREFIX"
    )

if test "x$with_theora" = "xno"
then
  AC_MSG_RESULT([Theora support disabled by request])
else
  if test "x$theora_prefix" = "x" -o "x$theora_prefix" = "xyes"; then
    if test "x$prefix" = "xNONE"; then
        theora_prefix="/usr/local"
    else
        theora_prefix="$prefix"
    fi
  fi

  THEORA_CFLAGS="$OGG_CFLAGS"
  THEORA_LDFLAGS="$OGG_LDFLAGS"
  if test "x$theora_prefix" != "x$ogg_prefix"; then
      XIPH_GCC_WARNING(-I"$theora_prefix/include",,
              [THEORA_CFLAGS="$THEORA_CFLAGS -I$theora_prefix/include"
              THEORA_LDFLAGS="-L$theora_prefix/lib $THEORA_LDFLAGS"
              ])
  fi

  THEORA_LIBS="-ltheora -logg"

  ac_save_LIBS="$LIBS"
  ac_save_LDFLAGS="$LDFLAGS"
  LDFLAGS="$LDFLAGS $THEORA_LDFLAGS"
  LIBS="$LIBS $THEORA_LIBS"
  AC_MSG_CHECKING([for libtheora])
  AC_TRY_LINK_FUNC(theora_decode_header, [xt_have_theora="yes"],
          [xt_have_theora="Not found"])
  if test "x$xt_have_theora" = "xyes"
  then
    AC_TRY_LINK_FUNC(theora_packet_isheader, [xt_have_theora="yes"],
          [xt_have_theora="newer version required"])
  fi

  LIBS="$ac_save_LIBS"
  LDFLAGS="$ac_save_LDFLAGS"

  if test "x$xt_have_theora" = "xyes"
  then
    AC_MSG_RESULT([ok])
    AC_DEFINE([HAVE_THEORA],[1],[Define if Theora support is available])
    $1
  else
    THEORA_CFLAGS=""
    THEORA_LDFLAGS=""
    THEORA_LIBS=""
    AC_MSG_RESULT([$xt_have_theora])
    $2
  fi
fi
AC_SUBST(THEORA_CFLAGS)
AC_SUBST(THEORA_LDFLAGS)
AC_SUBST(THEORA_LIBS)
])
