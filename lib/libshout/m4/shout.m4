dnl XIPH_PATH_SHOUT
dnl Jack Moffitt <jack@icecast.org> 08-06-2001
dnl Rewritten for libshout 2
dnl Brendan Cully <brendan@xiph.org> 20030612
dnl 
dnl $Id$

# XIPH_PATH_SHOUT([ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
# Test for libshout, and define SHOUT_CPPFLAGS SHOUT_CFLAGS SHOUT_LIBS, and
# SHOUT_THREADSAFE
AC_DEFUN([XIPH_PATH_SHOUT],
[dnl
xt_have_shout="no"
SHOUT_THREADSAFE="no"
SHOUT_CPPFLAGS=""
SHOUT_CFLAGS=""
SHOUT_LIBS=""

# NB: PKG_CHECK_MODULES exits if pkg-config is unavailable on the target
# system, so we can't use it.

# seed pkg-config with the default libshout location
PKG_CONFIG_PATH=${PKG_CONFIG_PATH:-/usr/local/lib/pkgconfig}
export PKG_CONFIG_PATH

# Step 1: Use pkg-config if available
AC_PATH_PROG([PKGCONFIG], [pkg-config], [no])
if test "$PKGCONFIG" != "no" && `$PKGCONFIG --exists shout`
then
  SHOUT_CFLAGS=`$PKGCONFIG --variable=cflags_only shout`
  SHOUT_CPPFLAGS=`$PKGCONFIG --variable=cppflags shout`
  SHOUT_LIBS=`$PKGCONFIG --libs shout`
  xt_have_shout="maybe"
else
  if test "$PKGCONFIG" != "no"
  then
    AC_MSG_NOTICE([$PKGCONFIG couldn't find libshout. Try adjusting PKG_CONFIG_PATH.])
  fi
  # pkg-config unavailable, try shout-config
  AC_PATH_PROG([SHOUTCONFIG], [shout-config], [no])
  if test "$SHOUTCONFIG" != "no" && test `$SHOUTCONFIG --package` = "libshout"
  then
    SHOUT_CPPFLAGS=`$SHOUTCONFIG --cppflags`
    SHOUT_CFLAGS=`$SHOUTCONFIG --cflags-only`
    SHOUT_LIBS=`$SHOUTCONFIG --libs`
    xt_have_shout="maybe"
  fi
fi

# Now try actually using libshout
if test "$xt_have_shout" != "no"
then
  ac_save_CPPFLAGS="$CPPFLAGS"
  ac_save_CFLAGS="$CFLAGS"
  ac_save_LIBS="$LIBS"
  CPPFLAGS="$CPPFLAGS $SHOUT_CPPFLAGS"
  CFLAGS="$CFLAGS $SHOUT_CFLAGS"
  LIBS="$SHOUT_LIBS $LIBS"
  AC_CHECK_HEADERS([shout/shout.h], [
    AC_CHECK_FUNC([shout_new], [
      ifelse([$1], , :, [$1])
      xt_have_shout="yes"
    ])
    AC_EGREP_CPP([yes], [#include <shout/shout.h>
#if SHOUT_THREADSAFE
yes
#endif
], [SHOUT_THREADSAFE="yes"])
  ])
  CPPFLAGS="$ac_save_CPPFLAGS"
  CFLAGS="$ac_save_CFLAGS"
  LIBS="$ac_save_LIBS"
fi

if test "$xt_have_shout" != "yes"
then
  ifelse([$2], , :, [$2])
fi
])dnl XIPH_PATH_SHOUT
