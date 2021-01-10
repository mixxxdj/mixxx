/* -*- c-basic-offset: 4; -*- */
/* sock.c: General Socket Functions
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

#ifdef HAVE_CONFIG_H
 #include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>
#include <ctype.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#ifdef HAVE_POLL
#include <poll.h>
#endif
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif

#ifndef _WIN32
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <netdb.h>
#else
#include <winsock2.h>
#endif

#include "sock.h"
#include "resolver.h"

/* for older C libraries */
#ifndef AI_NUMERICSERV
# define AI_NUMERICSERV 0
#endif
#ifndef AI_ADDRCONFIG
# define AI_ADDRCONFIG 0
#endif

/* sock_initialize
**
** initializes the socket library.  you must call this
** before using the library!
*/
void sock_initialize(void)
{
#ifdef _WIN32
    WSADATA wsad;
    WSAStartup(0x0101, &wsad);
#endif

    resolver_initialize();
}

/* sock_shutdown
**
** shutdown the socket library.  remember to call this when you're
** through using the lib
*/
void sock_shutdown(void)
{
#ifdef _WIN32
    WSACleanup();
#endif

    resolver_shutdown();
}

/* sock_get_localip
**
** gets the local ip address for the machine
** the ip it returns *should* be on the internet.
** in any case, it's as close as we can hope to get
** unless someone has better ideas on how to do this
*/
char *sock_get_localip(char *buff, int len)
{
    char temp[1024];

    if (gethostname(temp, sizeof(temp)) != 0)
        return NULL;

    if (resolver_getip(temp, buff, len))
        return buff;

    return NULL;
}

/* sock_error
** 
** returns the last socket error
*/
int sock_error(void)
{
#ifdef _WIN32
    return WSAGetLastError();
#else
    return errno;
#endif
}

void sock_set_error(int val)
{
#ifdef _WIN32
     WSASetLastError (val);
#else
     errno = val;
#endif
}

/* sock_recoverable
**
** determines if the socket error is recoverable
** in terms of non blocking sockets
*/
int sock_recoverable(int error)
{
    switch (error)
    {
    case 0:
    case EAGAIN:
    case EINTR:
    case EINPROGRESS:
#if defined(EWOULDBLOCK) && EWOULDBLOCK != EAGAIN
    case EWOULDBLOCK:
#endif
#if defined (WSAEWOULDBLOCK) && WSAEWOULDBLOCK != EWOULDBLOCK
    case WSAEWOULDBLOCK:
#endif
#if defined (WSAEINPROGRESS) && WSAEINPROGRESS != EINPROGRESS
    case WSAEINPROGRESS:
#endif
#ifdef ERESTART
    case ERESTART:
#endif
        return 1;
    default:
        return 0;
    }
}

int sock_stalled (int error)
{
    switch (error)
    {
    case EAGAIN:
    case EINPROGRESS:
    case EALREADY:
#if defined(EWOULDBLOCK) && EWOULDBLOCK != EAGAIN
    case EWOULDBLOCK:
#endif
#if defined (WSAEWOULDBLOCK) && WSAEWOULDBLOCK != EWOULDBLOCK
    case WSAEWOULDBLOCK:
#endif
#if defined (WSAEINPROGRESS) && WSAEINPROGRESS != EINPROGRESS
    case WSAEINPROGRESS:
#endif
#ifdef ERESTART
    case ERESTART:
#endif
        return 1;
    default:
        return 0;
    }
}


static int sock_connect_pending (int error)
{
    return error == EINPROGRESS || error == EALREADY;
}

/* sock_valid_socket
**
** determines if a sock_t represents a valid socket
*/
int sock_valid_socket(sock_t sock)
{
    int ret;
    int optval;
    socklen_t optlen;

    optlen = sizeof(int);
    /* apparently on windows getsockopt.optval is a char * */
    ret = getsockopt(sock, SOL_SOCKET, SO_TYPE, (void*) &optval, &optlen);

    return (ret == 0);
}


/* determines if the passed socket is still connected */
int sock_active (sock_t sock)
{
    char c;
    int l;

    l = recv (sock, &c, 1, MSG_PEEK);
    if (l == 0)
        return 0;
    if (l == SOCK_ERROR && sock_recoverable (sock_error()))
        return 1;
    return 0;
}

/* inet_aton
**
** turns an ascii ip address into a binary representation
*/
#ifdef _WIN32
int inet_aton(const char *s, struct in_addr *a)
{
    int lsb, b2, b3, msb;

    if (sscanf(s, "%d.%d.%d.%d", &lsb, &b2, &b3, &msb) < 4) {
        return 0;
    }

    a->s_addr = inet_addr(s);
    
    return (a->s_addr != INADDR_NONE);
}
#endif /* _WIN32 */

/* sock_set_blocking
 *
 * set the sock blocking or nonblocking
 * 1 for blocking
 * 0 for nonblocking
 */
int sock_set_blocking(sock_t sock, int block)
{
#ifdef _WIN32
#ifdef __MINGW32__
    u_long varblock = 1;
#else
    int varblock = 1;
#endif
#endif

    if ((!sock_valid_socket(sock)) || (block < 0) || (block > 1))
        return SOCK_ERROR;

#ifdef _WIN32
    if (block) varblock = 0;
    return ioctlsocket(sock, FIONBIO, &varblock);
#else
    return fcntl(sock, F_SETFL, (block) ? 0 : O_NONBLOCK);
#endif
}

int sock_set_nolinger(sock_t sock)
{
    struct linger lin = { 0, 0 };
    return setsockopt(sock, SOL_SOCKET, SO_LINGER, (void *)&lin, 
            sizeof(struct linger));
}

int sock_set_nodelay(sock_t sock)
{
    int nodelay = 1;

    return setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (void *)&nodelay,
            sizeof(int));
}

int sock_set_keepalive(sock_t sock)
{
    int keepalive = 1;
    return setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, (void *)&keepalive, 
            sizeof(int));
}

/* sock_close
**
** close the socket
*/
int sock_close(sock_t sock)
{
#ifdef _WIN32
    return closesocket(sock);
#else
    return close(sock);
#endif
}

/* sock_writev
 *
 * write multiple buffers at once, return bytes actually written
 */
#ifdef HAVE_WRITEV

ssize_t sock_writev (sock_t sock, const struct iovec *iov, size_t count)
{
    return writev (sock, iov, count);
}

#else

ssize_t sock_writev (sock_t sock, const struct iovec *iov, size_t count)
{
    int i = count, accum = 0, ret;
    const struct iovec *v = iov;

    while (i)
    {
        if (v->iov_base && v->iov_len)
        {
            ret = sock_write_bytes (sock, v->iov_base, v->iov_len);
            if (ret == -1 && accum==0)
                return -1;
            if (ret == -1)
                ret = 0;
            accum += ret;
            if (ret < (int)v->iov_len)
                break;
        }
        v++;
        i--;
    }
    return accum;
}

#endif

/* sock_write_bytes
**
** write bytes to the socket
** this function will _NOT_ block
*/
int sock_write_bytes(sock_t sock, const void *buff, size_t len)
{
    /* sanity check */
    if (!buff) {
        return SOCK_ERROR;
    } else if (len <= 0) {
        return SOCK_ERROR;
    } /*else if (!sock_valid_socket(sock)) {
        return SOCK_ERROR;
    } */

    return send(sock, buff, len, 0);
}

/* sock_write_string
**
** writes a string to a socket
** This function must only be called with a blocking socket.
*/
int sock_write_string(sock_t sock, const char *buff)
{
    return (sock_write_bytes(sock, buff, strlen(buff)) > 0);
}

/* sock_write
**
** write a formatted string to the socket
** this function must only be called with a blocking socket.
** will truncate the string if it's greater than 1024 chars.
*/
int sock_write(sock_t sock, const char *fmt, ...)
{
    int rc;
    va_list ap;

    va_start (ap, fmt);
    rc = sock_write_fmt (sock, fmt, ap);
    va_end (ap);

    return rc;
}

#ifdef HAVE_OLD_VSNPRINTF
int sock_write_fmt(sock_t sock, const char *fmt, va_list ap)
{
    va_list ap_local;
    unsigned int len = 1024;
    char *buff = NULL;
    int ret;

    /* don't go infinite, but stop at some huge limit */
    while (len < 2*1024*1024)
    {
        char *tmp = realloc (buff, len);
        ret = -1;
        if (tmp == NULL)
            break;
        buff = tmp;
        va_copy (ap_local, ap);
        ret = vsnprintf (buff, len, fmt, ap_local);
        if (ret > 0)
        {
            ret = sock_write_bytes (sock, buff, ret);
            break;
        }
        len += 8192;
    }
    free (buff);
    return ret;
}
#else
int sock_write_fmt(sock_t sock, const char *fmt, va_list ap)
{
    char buffer [1024], *buff = buffer;
    int len;
    int rc = SOCK_ERROR;
    va_list ap_retry;

    va_copy (ap_retry, ap);

    len = vsnprintf (buff, sizeof (buffer), fmt, ap);

    if (len > 0)
    {
        if ((size_t)len < sizeof (buffer))   /* common case */
            rc = sock_write_bytes(sock, buff, (size_t)len);
        else
        {
            /* truncated */
            buff = malloc (++len);
            if (buff)
            {
                len = vsnprintf (buff, len, fmt, ap_retry);
                if (len > 0)
                    rc = sock_write_bytes (sock, buff, len);
                free (buff);
            }
        }
    }
    va_end (ap_retry);

    return rc;
}
#endif


int sock_read_bytes(sock_t sock, char *buff, size_t len)
{

    /*if (!sock_valid_socket(sock)) return 0; */
    if (!buff) return 0;
    if (len <= 0) return 0;

    return recv(sock, buff, len, 0);
}

/* sock_read_line
**
** Read one line of at max len bytes from sock into buff.
** If ok, return 1 and nullterminate buff. Otherwize return 0.
** Terminating \n is not put into the buffer.
**
** this function will probably not work on sockets in nonblocking mode
*/
int sock_read_line(sock_t sock, char *buff, const int len)
{
    char c = '\0';
    int read_bytes, pos;
  
    /*if (!sock_valid_socket(sock)) {
        return 0;
    } else*/ if (!buff) {
        return 0;
    } else if (len <= 0) {
        return 0;
    }

    pos = 0;
    read_bytes = recv(sock, &c, 1, 0);

    if (read_bytes < 0) {
        return 0;
    }

    while ((c != '\n') && (pos < len) && (read_bytes == 1)) {
        if (c != '\r')
            buff[pos++] = c;
        read_bytes = recv(sock, &c, 1, 0);
    }
    
    if (read_bytes == 1) {
        buff[pos] = '\0';
        return 1;
    } else {
        return 0;
    }
}

/* see if a connection has been established. If timeout is < 0 then wait
 * indefinitely, else wait for the stated number of seconds.
 * return SOCK_TIMEOUT for timeout
 * return SOCK_ERROR for failure
 * return 0 for try again, interrupted
 * return 1 for ok 
 */
#ifdef HAVE_POLL
int sock_connected (sock_t sock, int timeout)
{
    struct pollfd check;
    int val = SOCK_ERROR;
    socklen_t size = sizeof val;

    check.fd = sock;
    check.events = POLLOUT;
    switch (poll (&check, 1, timeout*1000))
    {
        case 0: return SOCK_TIMEOUT;
        default:
            /* on windows getsockopt.val is defined as char* */
            if (getsockopt(sock, SOL_SOCKET, SO_ERROR, (void*) &val, &size) == 0)
            {
                if (val == 0)
                    return 1;
                sock_set_error (val);
            }
            /* fall through */
        case -1:
            if (sock_recoverable (sock_error()))
                return 0;
            return SOCK_ERROR;
    }                                           
}

#else

int sock_connected (sock_t sock, int timeout)
{
    fd_set wfds;
    int val = SOCK_ERROR;
    socklen_t size = sizeof val;
    struct timeval tv, *timeval = NULL;

    /* make a timeout of <0 be indefinite */
    if (timeout >= 0)
    {
        tv.tv_sec = timeout;
        tv.tv_usec = 0;
        timeval = &tv;
    }

    FD_ZERO(&wfds);
    FD_SET(sock, &wfds);

    switch (select(sock + 1, NULL, &wfds, NULL, timeval))
    {
        case 0:
            return SOCK_TIMEOUT;
        default:
            /* on windows getsockopt.val is defined as char* */
            if (getsockopt(sock, SOL_SOCKET, SO_ERROR, (void*) &val, &size) == 0)
            {
                if (val == 0)
                    return 1;
                sock_set_error (val);
            }
            /* fall through */
        case -1:
            if (sock_recoverable (sock_error()))
                return 0;
            return SOCK_ERROR;
    }
}
#endif

sock_t sock_connect_wto (const char *hostname, int port, int timeout)
{
    return sock_connect_wto_bind(hostname, port, NULL, timeout);
}

#ifdef HAVE_GETADDRINFO

sock_t sock_connect_non_blocking (const char *hostname, unsigned port)
{
    int sock = SOCK_ERROR;
    struct addrinfo *ai, *head, hints;
    char service[8];

    memset (&hints, 0, sizeof (hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    snprintf (service, sizeof (service), "%u", port);

    if (getaddrinfo (hostname, service, &hints, &head))
        return SOCK_ERROR;

    ai = head;
    while (ai)
    {
        if ((sock = socket (ai->ai_family, ai->ai_socktype, ai->ai_protocol)) 
                > -1)
        {
            sock_set_blocking (sock, 0);
            if (connect(sock, ai->ai_addr, ai->ai_addrlen) < 0 && 
                    !sock_connect_pending(sock_error()))
            {
                sock_close (sock);
                sock = SOCK_ERROR;
            }
            else
                break;
        }
        ai = ai->ai_next;
    }
    if (head) freeaddrinfo (head);
    
    return sock;
}

/* issue a connect, but return after the timeout (seconds) is reached. If
 * timeout is 0 or less then we will wait until the OS gives up on the connect
 * The socket is returned
 */
sock_t sock_connect_wto_bind (const char *hostname, int port, const char *bnd, int timeout)
{
    sock_t sock = SOCK_ERROR;
    struct addrinfo *ai, *head, *b_head=NULL, hints;
    char service[8];

    memset (&hints, 0, sizeof (hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    snprintf (service, sizeof (service), "%u", port);

    if (getaddrinfo (hostname, service, &hints, &head))
        return SOCK_ERROR;

    ai = head;
    while (ai)
    {
        if ((sock = socket (ai->ai_family, ai->ai_socktype, ai->ai_protocol)) >= 0)
        {
            if (timeout > 0)
                sock_set_blocking (sock, 0);

            if (bnd)
            {
                struct addrinfo b_hints;
                memset (&b_hints, 0, sizeof(b_hints));
                b_hints.ai_family = ai->ai_family;
                b_hints.ai_socktype = ai->ai_socktype;
                b_hints.ai_protocol = ai->ai_protocol;
                if (getaddrinfo (bnd, NULL, &b_hints, &b_head) ||
                        bind (sock, b_head->ai_addr, b_head->ai_addrlen) < 0)
                {
                    sock_close (sock);
                    sock = SOCK_ERROR;
                    break;
                }
            }

            if (connect (sock, ai->ai_addr, ai->ai_addrlen) == 0)
                break;

            /* loop as the connect maybe async */
            while (sock != SOCK_ERROR)
            {
                if (sock_recoverable (sock_error()))
                {
                    int connected = sock_connected (sock, timeout);
                    if (connected == 0)  /* try again, interrupted */
                        continue;
                    if (connected == 1) /* connected */
                    {
                        if (timeout >= 0)
                            sock_set_blocking(sock, 1);
                        break;
                    }
                }
                sock_close (sock);
                sock = SOCK_ERROR;
            }
            if (sock != SOCK_ERROR)
                break;
        }
        ai = ai->ai_next;
    }
    if (b_head)
        freeaddrinfo (b_head);
    freeaddrinfo (head);

    return sock;
}


sock_t sock_get_server_socket (int port, const char *sinterface)
{
    struct sockaddr_storage sa;
    struct addrinfo hints, *res, *ai;
    char service [12];
    int sock;

    if (port < 0)
        return SOCK_ERROR;

    memset (&sa, 0, sizeof(sa));
    memset (&hints, 0, sizeof(hints));

    hints.ai_family = AF_UNSPEC;
    hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG | AI_NUMERICSERV | AI_NUMERICHOST;
    hints.ai_socktype = SOCK_STREAM;
    snprintf (service, sizeof (service), "%d", port);

    if (getaddrinfo (sinterface, service, &hints, &res))
        return SOCK_ERROR;
    ai = res;
    do
    {
        int on = 1;
        sock = socket (ai->ai_family, ai->ai_socktype, ai->ai_protocol);
        if (sock < 0)
            continue;

        setsockopt (sock, SOL_SOCKET, SO_REUSEADDR, (const void *)&on, sizeof(on));
        on = 0;
#ifdef IPV6_V6ONLY
        setsockopt (sock, IPPROTO_IPV6, IPV6_V6ONLY, &on, sizeof on);
#endif

        if (bind (sock, ai->ai_addr, ai->ai_addrlen) < 0)
        {
            sock_close (sock);
            continue;
        }
        freeaddrinfo (res);
        return sock;

    } while ((ai = ai->ai_next));

    freeaddrinfo (res);
    return SOCK_ERROR;
}


#else


int sock_try_connection (sock_t sock, const char *hostname, unsigned int port)
{
    struct sockaddr_in sin, server;
    char ip[MAX_ADDR_LEN];

    if (!hostname || !hostname[0] || port == 0)
        return -1;

    memset(&sin, 0, sizeof(struct sockaddr_in));
    memset(&server, 0, sizeof(struct sockaddr_in));

    if (!resolver_getip(hostname, ip, MAX_ADDR_LEN))
    {
        sock_close (sock);
        return -1;
    }

    if (inet_aton(ip, (struct in_addr *)&sin.sin_addr) == 0)
    {
        sock_close(sock);
        return -1;
    }

    memcpy(&server.sin_addr, &sin.sin_addr, sizeof(struct sockaddr_in));

    server.sin_family = AF_INET;
    server.sin_port = htons((short)port);

    return connect(sock, (struct sockaddr *)&server, sizeof(server));
}

sock_t sock_connect_non_blocking (const char *hostname, unsigned port)
{
    sock_t sock;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == SOCK_ERROR)
        return SOCK_ERROR;

    sock_set_blocking (sock, 0);
    sock_try_connection (sock, hostname, port);
    
    return sock;
}

sock_t sock_connect_wto_bind (const char *hostname, int port, const char *bnd, int timeout)
{
    sock_t sock;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == SOCK_ERROR)
        return SOCK_ERROR;

    if (bnd)
    {
        struct sockaddr_in sa;

        memset(&sa, 0, sizeof(sa));
        sa.sin_family = AF_INET;

        if (inet_aton (bnd, &sa.sin_addr) == 0 ||
            bind (sock, (struct sockaddr *)&sa, sizeof(sa)) < 0)
        {
            sock_close (sock);
            return SOCK_ERROR;
        }
    }

    if (timeout)
    {
        sock_set_blocking (sock, 0);
        if (sock_try_connection (sock, hostname, port) < 0)
        {
            int ret = sock_connected (sock, timeout);
            if (ret <= 0)
            {
                sock_close (sock);
                return SOCK_ERROR;
            }
        }
        sock_set_blocking(sock, 1);
    }
    else
    {
        if (sock_try_connection (sock, hostname, port) < 0)
        {
            sock_close (sock);
            sock = SOCK_ERROR;
        }
    }
    return sock;
}


/* sock_get_server_socket
**
** create a socket for incoming requests on a specified port and
** interface.  if interface is null, listen on all interfaces.
** returns the socket, or SOCK_ERROR on failure
*/
sock_t sock_get_server_socket(int port, const char *sinterface)
{
    struct sockaddr_in sa;
    int error, opt;
    sock_t sock;
    char ip[MAX_ADDR_LEN];

    if (port < 0)
        return SOCK_ERROR;

    /* defaults */
    memset(&sa, 0, sizeof(sa));

    /* set the interface to bind to if specified */
    if (sinterface != NULL) {
        if (!resolver_getip(sinterface, ip, sizeof (ip)))
            return SOCK_ERROR;

        if (!inet_aton(ip, &sa.sin_addr)) {
            return SOCK_ERROR;
        } else {
            sa.sin_family = AF_INET;
            sa.sin_port = htons((short)port);
        }
    } else {
        sa.sin_addr.s_addr = INADDR_ANY;
        sa.sin_family = AF_INET;
        sa.sin_port = htons((short)port);
    }

    /* get a socket */
    sock = socket (AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
        return SOCK_ERROR;

    /* reuse it if we can */
    opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const void *)&opt, sizeof(int));

    /* bind socket to port */
    error = bind(sock, (struct sockaddr *)&sa, sizeof (struct sockaddr_in));
    if (error == -1)
        return SOCK_ERROR;

    return sock;
}

#endif

void sock_set_send_buffer (sock_t sock, int win_size)
{
    setsockopt (sock, SOL_SOCKET, SO_SNDBUF, (char *) &win_size, sizeof(win_size));
}

int sock_listen(sock_t serversock, int backlog)
{
    if (!sock_valid_socket(serversock))
        return 0;

    if (backlog <= 0)
        backlog = 10;

    return (listen(serversock, backlog) == 0);
}

sock_t sock_accept(sock_t serversock, char *ip, size_t len)
{
#ifdef HAVE_GETNAMEINFO
    struct sockaddr_storage sa;
#else    
    struct sockaddr_in sa;
#endif
    sock_t ret;
    socklen_t slen;

    if (ip == NULL || len == 0 || !sock_valid_socket(serversock))
        return SOCK_ERROR;

    slen = sizeof(sa);
    ret = accept(serversock, (struct sockaddr *)&sa, &slen);

    if (ret != SOCK_ERROR)
    {
#ifdef HAVE_GETNAMEINFO
        if (getnameinfo ((struct sockaddr *)&sa, slen, ip, len, NULL, 0, NI_NUMERICHOST))
            snprintf (ip, len, "unknown");
#else
        /* inet_ntoa is not reentrant, we should protect this */
        strncpy(ip, inet_ntoa(sa.sin_addr), len);
#endif
        sock_set_nolinger(ret);
        sock_set_keepalive(ret);
    }

    return ret;
}

