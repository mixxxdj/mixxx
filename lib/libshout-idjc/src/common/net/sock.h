/* sock.h
 * - General Socket Function Headers
 *
 * Copyright (C) 2014 Michael Smith <msmith@icecast.org>,
 *                    Brendan Cully <brendan@xiph.org>,
 *                    Karl Heyes <karl@xiph.org>,
 *                    Jack Moffitt <jack@icecast.org>,
 *                    Ed "oddsock" Zaleski <oddsock@xiph.org>
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

#ifndef __SOCK_H
#define __SOCK_H

#include <stdarg.h>

#ifdef HAVE_WINSOCK2_H
#include <winsock2.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#elif _WIN32
#include <os.h>
#endif

#ifdef HAVE_SYS_UIO_H
#include <sys/uio.h>
#else
#ifndef _SYS_UIO_H
struct iovec
{
    void   *iov_base;
    size_t iov_len;
};
#endif
#endif

#if !defined(HAVE_INET_ATON) && defined(HAVE_INET_PTON)
#define inet_aton(a,b) inet_pton(AF_INET, (a), (b))
#endif

#ifdef INET6_ADDRSTRLEN
#define MAX_ADDR_LEN INET6_ADDRSTRLEN
#else
#define MAX_ADDR_LEN 46
#endif

#ifndef sock_t
#define sock_t int
#endif

/* The following values are based on unix avoiding errno value clashes */
#define SOCK_SUCCESS 0
#define SOCK_ERROR (sock_t)-1
#define SOCK_TIMEOUT -2

/* sock connect macro */
#define sock_connect(h, p) sock_connect_wto(h, p, 0)

#ifdef _mangle
# define sock_initialize _mangle(sock_initialize)
# define sock_shutdown _mangle(sock_shutdown)
# define sock_get_localip _mangle(sock_get_localip)
# define sock_error _mangle(sock_error)
# define sock_set_error _mangle(sock_set_error)
# define sock_recoverable _mangle(sock_recoverable)
# define sock_stalled _mangle(sock_stalled)
# define sock_valid_socket _mangle(sock_valid_socket)
# define sock_set_blocking _mangle(sock_set_blocking)
# define sock_set_nolinger _mangle(sock_set_nolinger)
# define sock_set_nodelay _mangle(sock_set_nodelay)
# define sock_set_keepalive _mangle(sock_set_keepalive)
# define sock_close _mangle(sock_close)
# define sock_connect_wto _mangle(sock_connect_wto)
# define sock_connect_wto_bind _mangle(sock_connect_wto_bind)
# define sock_connect_non_blocking _mangle(sock_connect_non_blocking)
# define sock_connected _mangle(sock_connected)
# define sock_write_bytes _mangle(sock_write_bytes)
# define sock_write _mangle(sock_write)
# define sock_write_fmt _mangle(sock_write_fmt)
# define sock_write_string _mangle(sock_write_string)
# define sock_writev _mangle(sock_writev)
# define sock_read_bytes _mangle(sock_read_bytes)
# define sock_read_line _mangle(sock_read_line)
# define sock_get_server_socket _mangle(sock_get_server_socket)
# define sock_listen _mangle(sock_listen)
# define sock_set_send_buffer _mangle(sock_set_send_buffer)
# define sock_accept _mangle(sock_accept)
#endif

/* Misc socket functions */
void sock_initialize(void);
void sock_shutdown(void);
char *sock_get_localip(char *buff, int len);
int sock_error(void);
int sock_recoverable(int error);
int sock_stalled(int error);
int sock_valid_socket(sock_t sock);
int sock_active (sock_t sock);
int sock_set_blocking(sock_t sock, int block);
int sock_set_nolinger(sock_t sock);
int sock_set_keepalive(sock_t sock);
int sock_set_nodelay(sock_t sock);
void sock_set_send_buffer (sock_t sock, int win_size);
void sock_set_error(int val);
int sock_close(sock_t  sock);

/* Connection related socket functions */
sock_t sock_connect_wto(const char *hostname, int port, int timeout);
sock_t sock_connect_wto_bind(const char *hostname, int port, const char *bnd, int timeout);
sock_t sock_connect_non_blocking(const char *host, unsigned port);
int sock_connected(sock_t sock, int timeout);

/* Socket write functions */
int sock_write_bytes(sock_t sock, const void *buff, size_t len);
int sock_write(sock_t sock, const char *fmt, ...);
int sock_write_fmt(sock_t sock, const char *fmt, va_list ap);
int sock_write_string(sock_t sock, const char *buff);
ssize_t sock_writev (sock_t sock, const struct iovec *iov, size_t count);


/* Socket read functions */
int sock_read_bytes(sock_t sock, char *buff, size_t len);
int sock_read_line(sock_t sock, char *string, const int len);

/* server socket functions */
sock_t sock_get_server_socket(int port, const char *sinterface);
int sock_listen(sock_t serversock, int backlog);
sock_t sock_accept(sock_t serversock, char *ip, size_t len);

#ifdef _WIN32
int inet_aton(const char *s, struct in_addr *a);
#endif

#endif  /* __SOCK_H */
