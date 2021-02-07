# Configure paths for libogg
# updated by Karl Heyes 10-Jun-2003
# Jack Moffitt <jack@icecast.org> 10-21-2000
# Shamelessly stolen from Owen Taylor and Manish Singh

dnl XIPH_PATH_OGG([ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
dnl Test for libogg, and define OGG_CFLAGS OGG_LDFLAGS and OGG_LIBS
dnl
AC_DEFUN([XIPH_PATH_OGG],
[dnl 
AC_ARG_VAR([OGG_PREFIX],[path to ogg installation])
AC_ARG_WITH(ogg,
    [AC_HELP_STRING([--with-ogg=PREFIX],
        [Prefix where libogg is installed (optional)])],
    ogg_prefix="$withval",
    ogg_prefix="$OGG_PREFIX"
    )
if test "x$ogg_prefix" = "x" -o "x$ogg_prefix" = "xyes"; then
    if test "x$prefix" = "xNONE"; then
        ogg_prefix=/usr/local
    else
        ogg_prefix="$prefix"
    fi
fi

XIPH_GCC_WARNING([-I$ogg_prefix/include],,
    [ OGG_CFLAGS="-I$ogg_prefix/include"
      OGG_LDFLAGS="-L$ogg_prefix/lib"
      ])
AC_CACHE_CHECK([for libogg], xt_cv_lib_ogg,
[dnl
OGG_LIBS="-logg"

#
# check if the installed Ogg is sufficiently new.
#
ac_save_CFLAGS="$CFLAGS"
ac_save_LIBS="$LIBS"
ac_save_LDFLAGS="$LDFLAGS"
CFLAGS="$CFLAGS $OGG_CFLAGS"
LIBS="$LIBS $OGG_LIBS"
LDFLAGS="$LDFLAGS $OGG_LDFLAGS"
AC_TRY_LINK_FUNC(ogg_sync_init,
    [ xt_cv_lib_ogg=ok ],
    [ AC_TRY_LINK([#include <ogg/ogg.h>],, 
        [ xt_cv_lib_ogg="pre v1.0, needs updating" ],
        [ xt_cv_lib_ogg="not found" ])
    ])
CFLAGS="$ac_save_CFLAGS"
LDFLAGS="$ac_save_LDFLAGS"
LIBS="$ac_save_LIBS"
])
if test "x$xt_cv_lib_ogg" = "xok"; then
    ifelse([$1],,,[$1])
    AC_DEFINE([HAVE_OGG], [1], [Define if you have libogg installed])
else
    OGG_LIBS=""
    OGG_CFLAGS=""
    OGG_LDFLAGS=""
    ifelse([$2],,,[$2])
fi
AC_SUBST(OGG_LIBS)
AC_SUBST(OGG_CFLAGS)
AC_SUBST(OGG_LDFLAGS)
])
