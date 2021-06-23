dnl xiph_types.m4
dnl macros for type checks not covered by autoconf

dnl XIPH_C99_INTTYPES
dnl Brendan Cully
dnl
# XIPH_C99_INTTYPES
# Check for C99 integer type definitions, or define if missing
AC_DEFUN([XIPH_C99_INTTYPES],
[dnl
AC_CHECK_HEADERS([inttypes.h])
AC_CHECK_TYPE([uint32_t],
  [AC_DEFINE(HAVE_C99_INTTYPES, 1, [Define if you have the C99 integer types])],
  [AC_CHECK_SIZEOF(short)
   AC_CHECK_SIZEOF(int)
   AC_CHECK_SIZEOF(long)
   AC_CHECK_SIZEOF(long long)])
AH_VERBATIM([X_HAVE_C99_INTTYPES],
  [#ifndef HAVE_C99_INTTYPES
#  if SIZEOF_SHORT == 4
typedef unsigned short uint32_t;
#  elif SIZEOF_INT == 4
typedef unsigned int uint32_t;
#  elif SIZEOF_LONG == 4
typedef unsigned long uint32_t;
#  endif
#  if SIZEOF_INT == 8
typedef unsigned int uint64_t;
#  elif SIZEOF_LONG == 8
typedef unsigned long uint64_t;
#  elif SIZEOF_LONG_LONG == 8
typedef unsigned long long uint64_t;
#  endif
#endif
  ])
])

dnl XIPH_TYPE_SOCKLEN_T
dnl Brendan Cully
dnl
# XIPH_TYPE_SOCKLEN_T
# Check for socklen_t, or define as int if missing
AC_DEFUN([XIPH_TYPE_SOCKLEN_T],
[dnl
AC_CHECK_HEADERS([sys/socket.h])
AC_CHECK_TYPES([socklen_t],,,
  [#if HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#if HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif
  ])
AH_VERBATIM([X_HAVE_SOCKLEN_T],
  [#ifndef HAVE_SOCKLEN_T
typedef int socklen_t;
#endif
  ])
])
