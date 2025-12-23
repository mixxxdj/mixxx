dnl XIPH_PATH_OPENSSL([ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
dnl Karl Heyes <karl@xiph.org>
dnl Test for openssl, and define OPENSSL_CFLAGS and OPENSSL_LIBS
dnl
AC_DEFUN([XIPH_PATH_OPENSSL],
[dnl 
dnl Get the cflags and libraries
dnl
AC_ARG_WITH(openssl,
    AC_HELP_STRING([--with-openssl=PFX],[Prefix where openssl is installed (optional)]),
    openssl_prefix="$withval", openssl_prefix="")

if test "x$openssl_prefix" != "x" -a "x$openssl_prefix" != "xyes"; then
    OPENSSL_LIBS="-L$openssl_prefix/lib -lssl"
    OPENSSL_CFLAGS="-I$openssl_prefix/include"
else
    AC_PATH_PROG([PKGCONFIG], [pkg-config], [no])
    if test "$PKGCONFIG" != "no" && `$PKGCONFIG --exists openssl`; then
        OPENSSL_CFLAGS=`$PKGCONFIG --cflags openssl`
        OPENSSL_LIBS=`$PKGCONFIG --libs openssl`
    else
        if test "x$prefix" = "xNONE"; then
            openssl_prefix="/usr/local"
        else
            openssl_prefix="$prefix"
        fi
        OPENSSL_LIBS="-L$openssl_prefix/lib -lssl"
        OPENSSL_CFLAGS="-I$openssl_prefix/include"
    fi
fi

# Now try linking to openssl
xt_save_CFLAGS="$CFLAGS"
xt_save_LIBS="$LIBS"
CFLAGS="$CFLAGS $OPENSSL_CFLAGS"
LIBS="$OPENSSL_LIBS $LIBS"
AC_TRY_LINK([#include <openssl/ssl.h>], [void *a = SSL_new], [openssl_ok='yes'])
CFLAGS="$xt_save_CFLAGS"
LIBS="$xt_save_LIBS"

if test "$openssl_ok" = "yes"; then
    AC_DEFINE(HAVE_OPENSSL, 1, [Define if you have libopenssl.])
    ifelse([$1], , :, [$1])     
else
    OPENSSL_LIBS=""
    OPENSSL_CFLAGS=""
    ifelse([$2], , :, [$2])
fi
])
