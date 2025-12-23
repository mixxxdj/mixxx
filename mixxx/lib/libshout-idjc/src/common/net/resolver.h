/*
 * resolver.h
 *
 * name resolver library header
 *
 * Copyright (C) 2014 Brendan Cully <brendan@xiph.org>,
 *                    Jack Moffitt <jack@icecast.org>
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

#ifndef __RESOLVER_H
#define __RESOLVER_H


/*
** resolver_lookup
**
** resolves a hosts name from it's ip address
** or
** resolves an ip address from it's host name
**
** returns a pointer to buff, or NULL if an error occured
**
*/

#ifdef _mangle
# define resolver_initialize _mangle(resolver_initialize)
# define resolver_shutdown _mangle(resolver_shutdown)
# define resolver_getname _mangle(resolver_getname)
# define resolver_getip _mangle(resolver_getip)
#endif

void resolver_initialize(void);
void resolver_shutdown(void);

char *resolver_getname(const char *ip, char *buff, int len);
char *resolver_getip(const char *name, char *buff, int len);

#endif





