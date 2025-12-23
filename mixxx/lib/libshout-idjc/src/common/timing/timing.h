/*
 * Timing functions.
 *
 * Copyright (C) 2014 Michael Smith <msmith@icecast.org>,
 *                    Karl Heyes <karl@xiph.org>,
 *                    Jack Moffitt <jack@icecast.org>,
 *                    Moritz Grimm <mdgrimm@gmx.net>,
 *                    Ralph Giles <giles@xiph.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 *
 */

#ifndef __TIMING_H__
#define __TIMING_H__

#include <sys/types.h>
#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#elif defined(HAVE_STDINT_H)
#include <stdint.h>
#endif

#if defined(_WIN32) && !defined(int64_t)
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
#endif

/* config.h should be included before we are to define _mangle */
#ifdef _mangle
# define timing_get_time _mangle(timing_get_time)
# define timing_sleep _mangle(timing_sleep)
#endif

uint64_t timing_get_time(void);
void timing_sleep(uint64_t sleeptime);

#endif  /* __TIMING_H__ */
