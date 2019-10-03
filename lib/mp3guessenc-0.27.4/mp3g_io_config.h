/*
 *   mp3g_io_config - basic OS-related settings for mp3guessenc
 *   Copyright (C) 2015-2018 Elio Blanca <eblanca76@users.sourceforge.net>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
/* Modifed by Evan Dekker 2019-09-26 */

#ifndef MP3G_IO_CONFIG_H_
#define MP3G_IO_CONFIG_H_

/* This will enable 64 bit integers as file offset on 32 bit posix machines */
/* Sadly, it won't work on Android, though */
#define _FILE_OFFSET_BITS                        64

#ifdef __MINGW32__
#define __USE_MINGW_FSEEK         /* Request mingw internal implementation of fseeko64 */
#define __USE_MINGW_ANSI_STDIO 1  /* Select a more ANSI C99 compatible implementation of printf() and friends. */
#define __have_typedef_off_t      /* This will prevent types.h from defining the type off_t - I don't need it */
#include <sys/types.h>
#define off_t     off64_t
#define _off_t    int             /* But some functions still need a _off_t type which is 32 bit long */
#endif /* __MINGW32__ */

#include <stdio.h>

#ifdef __MINGW32__
#define fseeko    fseeko64
#define ftello    ftello64
#endif /* __MINGW32__ */

#if defined(OS2) || defined(_OS2) || defined(__OS2__) || defined(AMIGA) || defined(__amigaos__) || defined(__WINDOWS__)
#define fseeko    fseek
#define ftello    ftell
#if defined(OS2) || defined(_OS2) || defined(__OS2__)
typedef long int off_t;
#endif /* os/2 */
#endif /* os/2 or amiga */

typedef long long bitoffs_t;

#endif /* MP3G_IO_CONFIG_H_ */

