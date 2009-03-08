/*
 * The contents of this file are subject to the Mozilla Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 * 
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 * 
 * The Original Code is MPEG4IP.
 * 
 * The Initial Developer of the Original Code is Cisco Systems Inc.
 * Portions created by Cisco Systems Inc. are
 * Copyright (C) Cisco Systems Inc. 2005.  All Rights Reserved.
 * 
 * Contributor(s): 
 *		Bill May wmay@cisco.com
 */
/* windows defines */
#ifndef __MPEG4IP_WIN32_H__
#define __MPEG4IP_WIN32_H__
#define HAVE_IN_PORT_T
#define HAVE_SOCKLEN_T
#define NEED_SDL_VIDEO_IN_MAIN_THREAD
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
#define _WINSOCKAPI_
#define _INTEGRAL_MAX_BITS 64
#ifndef __GNUC__
#define _CRT_SECURE_NO_DEPRECATE 1
#ifndef _WIN32
#define _WIN32
#endif
#endif
#include <windows.h>
#include <winsock2.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>

#ifndef inline
#define inline __inline
#endif
#ifndef __MINGW32__
typedef unsigned __int64 uint64_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int16 uint16_t;
typedef unsigned __int8 uint8_t;
typedef unsigned __int64 u_int64_t;
typedef unsigned __int32 u_int32_t;
typedef unsigned __int16 u_int16_t;
typedef unsigned __int8 u_int8_t;
typedef signed __int64 int64_t;
typedef signed __int32 int32_t;
typedef signed __int16 int16_t;
typedef signed __int8  int8_t;
typedef unsigned short in_port_t;
typedef int socklen_t;
typedef int ssize_t;
typedef unsigned int uint;
static inline int snprintf(char *buffer, size_t count,
			  const char *format, ...) {
  va_list ap;
  int ret;
  va_start(ap, format);
  ret = vsnprintf_s(buffer, count, _TRUNCATE, format, ap);
  va_end(ap);
  if (ret == -1) {
    if (errno == EINVAL) return -1;
    return (int)count;
  }
  return ret;
}
#endif
#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#define localtime_r(a,b) localtime_s(b,a)
#define printf printf_s
#define fprintf fprintf_s

#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifndef __GNUC__
#define read _read
#define write _write
#define lseek _lseek
#define close _close
#define open _open
#define access _access
#define vsnprintf _vsnprintf
#define stat _stati64
#define fstat _fstati64
#define fileno _fileno
#define strdup _strdup
#endif
#define F_OK 0
#define OPEN_RDWR (_O_RDWR | _O_BINARY)
#define OPEN_CREAT (_O_CREAT | _O_BINARY)
#define OPEN_RDONLY (_O_RDONLY | _O_BINARY)
#define srandom srand
#define random rand

#define IOSBINARY ios::binary

#ifdef __cplusplus
extern "C" {
#endif
int gettimeofday(struct timeval *t, void *);
#ifdef __cplusplus
}
#endif

#ifndef PATH_MAX
#define PATH_MAX MAX_PATH
#endif
#define MAX_UINT64 -1

#define D64F "I64d"
#define U64F  "I64u"
#define X64F "I64x"

#define TO_D64(a) (a##I64)
#define TO_U64(a) (a##UI64)

#define LOG_EMERG 0
#define LOG_ALERT 1
#define LOG_CRIT 2
#define LOG_ERR 3
#define LOG_WARNING 4
#define LOG_NOTICE 5
#define LOG_INFO 6
#define LOG_DEBUG 7

#if     defined (__GNUC__) || (!__STDC__ && _INTEGRAL_MAX_BITS >= 64)
#define VAR_TO_FPOS(fpos, var) (fpos) = (var)
#define FPOS_TO_VAR(fpos, typed, var) (var) = (typed)(fpos)
#else
#define VAR_TO_FPOS(fpos, var) (fpos).lopart = ((var) & UINT_MAX); (fpos).hipart = ((var) >> 32)
#define FPOS_TO_VAR(fpos, typed, var) (var) = (typed)((uint64_t)((fpos).hipart ) << 32 | (fpos).lopart)
#endif

#define __STRING(expr) #expr

#define FOPEN_READ_BINARY "rb"
#define FOPEN_WRITE_BINARY "wb"

#define UINT64_TO_DOUBLE(a) ((double)((int64_t)(a)))
#ifdef __cplusplus
extern "C" {
#endif
char *strcasestr(const char *haystack, const char *needle);
#ifdef __cplusplus
}
#endif


#define SIZEOF_BOOL 1

#ifndef __GNUC__
#ifndef _SS_PAD1SIZE
struct sockaddr_storage {
	unsigned short ss_family;
	uint32_t ss_align;
	char __ss_padding[128 - 2 * sizeof(uint32_t)];
};
#endif
#pragma warning(disable : 4244)
#pragma warning(disable: 4996)
#define HAVE_STRUCT_SOCKADDR_STORAGE 1
#define HAVE_INET_PTON 1
#define HAVE_INET_NTOP 1
#endif
#endif
