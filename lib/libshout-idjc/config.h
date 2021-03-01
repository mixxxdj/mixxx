/* config.h.  Generated from config.h.in by configure.  */
/* config.h.in.  Generated from configure.ac by autoheader.  */

#ifndef __CONFIG_H__
#define __CONFIG_H__ 1

/* Define to 1 if you have the <arpa/inet.h> header file. */
#define HAVE_ARPA_INET_H 1

/* Define if you have the C99 integer types */
#define HAVE_C99_INTTYPES 1

/* Define to 1 if you have the <dlfcn.h> header file. */
#define HAVE_DLFCN_H 1

/* Define to 1 if you have the `endhostent' function. */
#define HAVE_ENDHOSTENT 1

/* Define to 1 if you have the `ftime' function. */
#define HAVE_FTIME 1

/* Define to 1 if you have the `getaddrinfo' function. */
#define HAVE_GETADDRINFO 1

/* Define if you have the getnameinfo function */
#define HAVE_GETNAMEINFO 1

/* Define to 1 if you have the `gettimeofday' function. */
#define HAVE_GETTIMEOFDAY 1

/* Define to 1 if you have the `inet_aton' function. */
#define HAVE_INET_ATON 1

/* Define to 1 if you have the `inet_pton' function. */
#define HAVE_INET_PTON 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define if you have the nanosleep function */
#define HAVE_NANOSLEEP 1

/* Define if you have libogg installed */
#define HAVE_OGG 1

/* Define if you have libopenssl. */
#define HAVE_OPENSSL 1

/* Define if you have POSIX threads libraries and header files. */
/* #undef HAVE_PTHREAD */

/* Define to 1 if you have the `pthread_spin_lock' function. */
#define HAVE_PTHREAD_SPIN_LOCK 1

/* Define if you have the sethostent function */
#define HAVE_SETHOSTENT 1

/* Define to 1 if the system has the type `socklen_t'. */
#define HAVE_SOCKLEN_T 1

/* Define if you want speex streams supported */
//define HAVE_SPEEX 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if `ss_family' is a member of `struct sockaddr_storage'. */
#define HAVE_STRUCT_SOCKADDR_STORAGE_SS_FAMILY 1

/* Define to 1 if you have the <sys/select.h> header file. */
#define HAVE_SYS_SELECT_H 1

/* Define to 1 if you have the <sys/socket.h> header file. */
#define HAVE_SYS_SOCKET_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/timeb.h> header file. */
#define HAVE_SYS_TIMEB_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <sys/uio.h> header file. */
#define HAVE_SYS_UIO_H 1

/* Define if you want theora streams supported */
//define HAVE_THEORA 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Define if you have winsock2.h on MINGW */
/* #undef HAVE_WINSOCK2_H */

/* Define to 1 if you have the `writev' function. */
#define HAVE_WRITEV 1

/* Shout library major version */
#define LIBSHOUT_MAJOR 2

/* Shout library patch version */
#define LIBSHOUT_MICRO 1

/* Shout library minor version */
#define LIBSHOUT_MINOR 4

/* Define to the sub-directory where libtool stores uninstalled libraries. */
#define LT_OBJDIR ".libs/"

/* Define if you don't want to use the thread library */
/* #undef NO_THREAD */

/* Name of package */
#define PACKAGE "libshout-idjc"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "www.sourceforge.net/projects/idjc"

/* Define to the full name of this package. */
#define PACKAGE_NAME "libshout-idjc"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "libshout-idjc 2.4.1"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "libshout-idjc"

/* Define to the home page for this package. */
#define PACKAGE_URL ""

/* Define to the version of this package. */
#define PACKAGE_VERSION "2.4.1"

/* Define to necessary symbol if this constant uses a non-standard name on
   your system. */
/* #undef PTHREAD_CREATE_JOINABLE */

/* The size of `int', as computed by sizeof. */
/* #undef SIZEOF_INT */

/* The size of `long', as computed by sizeof. */
/* #undef SIZEOF_LONG */

/* The size of `long long', as computed by sizeof. */
/* #undef SIZEOF_LONG_LONG */

/* The size of `short', as computed by sizeof. */
/* #undef SIZEOF_SHORT */

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Define to 1 if you can safely include both <sys/time.h> and <time.h>. */
#define TIME_WITH_SYS_TIME 1

/* Version number of package */
#define VERSION "2.4.1"

#ifndef HAVE_C99_INTTYPES
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
  

#ifndef HAVE_SOCKLEN_T
typedef int socklen_t;
#endif
  

/* Define if you have POSIX and GNU specifications */
#define _GNU_SOURCE /**/

/* Define if you have POSIX and XPG specifications */
#define _XOPEN_SOURCE 600


/* name mangling to protect code we share with other libraries */
#define _mangle(proc) _shout_ ## proc


/* Define to empty if `const' does not conform to ANSI C. */
/* #undef const */

/* Define to `__inline__' or `__inline' if that's what the C compiler
   calls it, or to nothing if 'inline' is not supported under any name.  */
#ifndef __cplusplus
/* #undef inline */
#endif

/* define if va_copy is not available */
/* #undef va_copy */

#endif
