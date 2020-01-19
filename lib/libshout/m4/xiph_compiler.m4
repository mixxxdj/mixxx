dnl xiph_compiler.m4
dnl $Id$

dnl XIPH_FUNC_VA_COPY
dnl Karl Heyes
dnl
# XIPH_FUNC_VA_COPY
# Test for implementation of va_copy, or define appropriately if missing
AC_DEFUN([XIPH_FUNC_VA_COPY],
[dnl
AC_MSG_CHECKING([for va_copy])
AC_TRY_LINK([#include <stdarg.h>], [va_list ap1, ap2; va_copy(ap1, ap2);],
  AC_MSG_RESULT([va_copy]),
  [dnl
  AH_TEMPLATE([va_copy], [define if va_copy is not available])
  AC_TRY_LINK([#include <stdarg.h>], [va_list ap1, ap2; __va_copy(ap1, ap2);],
    [dnl
    AC_DEFINE([va_copy], [__va_copy])
    AC_MSG_RESULT([__va_copy])],
    [dnl
    AC_DEFINE([va_copy(dest,src)], [memcpy(&dest,&src,sizeof(va_list))])
    AC_MSG_RESULT([memcpy])
    ])
  ])
])
])dnl XIPH_FUNC_VA_COPY

dnl XIPH_C_ATTRIBUTE
dnl Karl Heyes
dnl
# XIPH_C_ATTRIBUTE
# Define __attribute__ to be empty if the compiler does not support it
AC_DEFUN([XIPH_C_ATTRIBUTE],
[dnl
AC_TRY_COMPILE([int func(void) __attribute__((unused));],
  [int x __attribute__ ((unused));],,[dnl
  AH_TEMPLATE([__attribute__],[Define to empty if __attribute__ is not supported])
  AC_DEFINE([__attribute__(x)],[])
])
])dnl XIPH_C_ATTRIBUTE

dnl XIPH_GCC_WARNING
dnl Karl Heyes
dnl
# XIPH_GCC_WARNING(flag, action-if-warning, action-if-not)
# Tests whether GCC emits a warning if explicitly asked to use flag.
# Useful for eg system default include paths
AC_DEFUN([XIPH_GCC_WARNING],
[AC_REQUIRE([AC_PROG_CC])
xt_warning=no
if test x"$GCC" = "xyes"
then
  save_cflags="$CFLAGS"
  CFLAGS="-Werror $1"
  AC_TRY_COMPILE(,,,xt_warning=yes)
  CFLAGS="$save_cflags"
fi
if test "$xt_warning" = "yes"
then
  ifelse([$2],,:,[$2])
else
  ifelse([$3],,:,[$3])
fi
])dnl XIPH_GCC_WARNING

dnl XIPH_CLEAN_CCFLAGS
dnl Brendan Cully <brendan@xiph.org> 20030612
dnl
# XIPH_CLEAN_CCFLAGS(flag-list, dest-shell-var-name)
# Filters out duplicate compiler flags, and -I flags if XIPH_GCC_WARNING
# complains about them
# Operates right-to-left on -l flags, left-to-right on everything else
# eg XIPH_CLEAN_CCFLAGS([-L/opt/lib -lfoo -lm -L/opt/lib -lbar -lm], [MY_LDFLAGS])
# => MY_LDFLAGS="-L/opt/lib -lfoo -lbar -lm"
# the cat<<EOF construct makes sure echo doesn't pick, say, -n
AC_DEFUN([XIPH_CLEAN_CCFLAGS],
[AC_REQUIRE([AC_PROG_FGREP])
xt_FLAGS=''

for flag in $1
do
  case "$flag" in
  -l*)
    xt_FLAGS="$flag $xt_FLAGS"
    ;;
  *)
    if { cat <<EOF
 $xt_FLAGS x
EOF
} | $FGREP -v -e " $flag " > /dev/null
    then
      xt_FLAGS="$flag $xt_FLAGS"
    fi
    ;;
  esac
done

$2=''
for flag in $xt_FLAGS
do
  if { cat <<EOF
 $$2 x
EOF
} | $FGREP -v -e " $flag " > /dev/null
  then
    $2="$flag $$2"
  fi
done

# Prune -I flags if $CC warns about them
xt_FLAGS=''
for flag in $$2
do
  case "$flag" in
  -I*)
    XIPH_GCC_WARNING([$flag], [], [xt_FLAGS="$xt_FLAGS $flag"])
    ;;
  *)
    xt_FLAGS="$xt_FLAGS $flag"
    ;;
  esac
done
$2="$xt_FLAGS"
])dnl XIPH_CLEAN_CCFLAGS

dnl XIPH_VAR_APPEND
dnl Karl Heyes
dnl
# XIPH_VAR_APPEND(shell-var, list)
# Append each item in list to shell-var iff shell-var doesn't already have it
# eg XIPH_VAR_APPEND([CFLAGS], [-O2 -I/opt/packages/include])
AC_DEFUN([XIPH_VAR_APPEND],
[dnl
AC_REQUIRE([AC_PROG_FGREP])
for arg in $2
do
  if { cat <<EOF
 $$1 x
EOF
} | $FGREP -v -e " $arg " > /dev/null
  then
    $1="$$1 $arg"
  fi
done
])dnl XIPH_VAR_APPEND

dnl XIPH_VAR_PREPEND
dnl Karl Heyes
dnl
# XIPH_VAR_PREPEND(shell-var, list)
# see XIPH_VAR_APPEND
AC_DEFUN([XIPH_VAR_PREPEND],
[dnl
AC_REQUIRE([AC_PROG_FGREP])
xt_compare="$$1"
xt_filtered=""   
for arg in $2
do
  if { cat <<EOF
 $xt_compare x
EOF
} | $FGREP -v -e " $arg " > /dev/null
  then
    xt_compare="$arg $xt_compare"
    xt_filtered="$xt_filtered $arg"
  fi
done
$1="$xt_filtered $$1"
])dnl XIPH_VAR_PREPEND

dnl XIPH_C__FUNC__
dnl Karl Heyes <karl@xiph.org> 07/2004
AC_DEFUN([XIPH_C__FUNC__],
[dnl
AC_MSG_CHECKING([for __func__])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM(,[const char *x = __func__;])],
    [ AC_MSG_RESULT([yes])],
    [ AH_TEMPLATE([__func__], [Replace __func__ if not supported])
    AC_COMPILE_IFELSE([AC_LANG_PROGRAM([],[const char *x = __FUNCTION__;])],
        [ AC_DEFINE([__func__],[__FUNCTION__])
        AC_MSG_RESULT([Using __FUNCTION__])],
        [ AC_DEFINE([__func__],["__FILE__"])
        AC_MSG_RESULT([using __FILE__])
        ])
    ])
])dnl XIPH_C__FUNC__

