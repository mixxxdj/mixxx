/* -*- c-basic-offset: 8; -*- */
/* shout.h: Private libshout data structures and declarations
 *
 *  Copyright (C) 2002-2004 the Icecast team <team@icecast.org>,
 *  Copyright (C) 2012-2019 Philipp "ph3-der-loewe" Schafft <lion@lion.leolix.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * $Id$
 */

#ifndef __LIBSHOUT_SHOUT_PRIVATE_H__
#define __LIBSHOUT_SHOUT_PRIVATE_H__

#ifdef HAVE_CONFIG_H
#   include "config.h"
#endif

#include <shoutidjc/shout.h>
#include <common/net/sock.h>
#include <common/timing/timing.h>
#include "util.h"

#include <sys/types.h>

#ifdef HAVE_STDINT_H
#   include <stdint.h>
#elif defined (HAVE_INTTYPES_H)
#   include <inttypes.h>
#endif

#ifdef HAVE_OPENSSL
#   include <openssl/ssl.h>
#endif

#define LIBSHOUT_DEFAULT_HOST       "localhost"
#define LIBSHOUT_DEFAULT_PORT       8000
#define LIBSHOUT_DEFAULT_FORMAT     SHOUT_FORMAT_OGG
#define LIBSHOUT_DEFAULT_USAGE      SHOUT_USAGE_UNKNOWN
#define LIBSHOUT_DEFAULT_PROTOCOL   SHOUT_PROTOCOL_HTTP
#define LIBSHOUT_DEFAULT_USER       "source"
#define LIBSHOUT_DEFAULT_USERAGENT  "libshout/" VERSION
#define LIBSHOUT_DEFAULT_ALLOWED_CIPHERS "ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-AES256-GCM-SHA384:DHE-RSA-AES128-GCM-SHA256:DHE-DSS-AES128-GCM-SHA256:kEDH+AESGCM:ECDHE-RSA-AES128-SHA256:ECDHE-ECDSA-AES128-SHA256:ECDHE-RSA-AES128-SHA:ECDHE-ECDSA-AES128-SHA:ECDHE-RSA-AES256-SHA384:ECDHE-ECDSA-AES256-SHA384:ECDHE-RSA-AES256-SHA:ECDHE-ECDSA-AES256-SHA:DHE-RSA-AES128-SHA256:DHE-RSA-AES128-SHA:DHE-DSS-AES128-SHA256:DHE-RSA-AES256-SHA256:DHE-DSS-AES256-SHA:DHE-RSA-AES256-SHA:AES128-GCM-SHA256:AES256-GCM-SHA384:AES128-SHA256:AES256-SHA256:AES128-SHA:AES256-SHA:AES:CAMELLIA:DES-CBC3-SHA:!aNULL:!eNULL:!EXPORT:!DES:!RC4:!MD5:!PSK:!aECDH:!EDH-DSS-DES-CBC3-SHA:!EDH-RSA-DES-CBC3-SHA:!KRB5-DES-CBC3-SHA" /* Mozilla's 'Intermediate' list as of 2015-04-19 */

/* server capabilities.
   0x000000XXUL -> Methods.
   0x0000XX00UL -> HTTP Options
   0x000X0000UL -> TLS Related
   0xX0000000UL -> State related
   0x0XX00000UL -> Reserved
 */
#define LIBSHOUT_CAP_SOURCE      0x00000001UL
#define LIBSHOUT_CAP_PUT         0x00000002UL
#define LIBSHOUT_CAP_GET         0x00000004UL
#define LIBSHOUT_CAP_POST        0x00000008UL
#define LIBSHOUT_CAP_OPTIONS     0x00000010UL
#define LIBSHOUT_CAP_CHUNKED     0x00000100UL
#define LIBSHOUT_CAP_100CONTINUE 0x00000200UL
#define LIBSHOUT_CAP_UPGRADETLS  0x00010000UL
#define LIBSHOUT_CAP_REQAUTH     0x20000000UL /* requires authentication */
#define LIBSHOUT_CAP_CHALLENGED  0x40000000UL
#define LIBSHOUT_CAP_GOTCAPS     0x80000000UL

#define LIBSHOUT_MAX_RETRY       3

#define SHOUT_BUFSIZE 4096

typedef struct _shout_tls shout_tls_t;

typedef struct _shout_buf {
    unsigned char   data[SHOUT_BUFSIZE];
    unsigned int    len;
    unsigned int    pos;

    struct  _shout_buf *prev;
    struct  _shout_buf *next;
} shout_buf_t;

typedef struct {
    shout_buf_t     *head;
    size_t           len;
} shout_queue_t;

typedef enum {
    SHOUT_SOCKSTATE_UNCONNECTED = 0,
    SHOUT_SOCKSTATE_CONNECTING,
    SHOUT_SOCKSTATE_CONNECTED,
    SHOUT_SOCKSTATE_TLS_CONNECTING,
    SHOUT_SOCKSTATE_TLS_CONNECTED,
    SHOUT_SOCKSTATE_TLS_VERIFIED
} shout_connect_socket_state_t;

typedef enum {
    SHOUT_MSGSTATE_IDLE = 0,
    SHOUT_MSGSTATE_CREATING0,
    SHOUT_MSGSTATE_SENDING0,
    SHOUT_MSGSTATE_WAITING0,
    SHOUT_MSGSTATE_RECEIVING0,
    SHOUT_MSGSTATE_RECEIVED0,
    SHOUT_MSGSTATE_PARSED_INFORMATIONAL0,
    SHOUT_MSGSTATE_CREATING1,
    SHOUT_MSGSTATE_SENDING1,
    SHOUT_MSGSTATE_WAITING1,
    SHOUT_MSGSTATE_RECEIVING1,
    SHOUT_MSGSTATE_RECEIVED1,
    SHOUT_MSGSTATE_PARSED_INFORMATIONAL1,
    SHOUT_MSGSTATE_PARSED_FINAL
} shout_connect_message_state_t;

typedef enum {
    SHOUT_RS_DONE,
    SHOUT_RS_TIMEOUT,
    SHOUT_RS_NOTNOW,
    SHOUT_RS_ERROR
} shout_connection_return_state_t;

typedef union shout_protocol_extra_tag {
    int si;
    void *vp;
} shout_protocol_extra_t;

typedef struct {
    int is_source;
    int fake_ua;
    int auth;
    const char *method;
    const char *resource;
    const char *param;
} shout_http_plan_t;

typedef struct shout_connection_tag shout_connection_t;

typedef struct {
    shout_connection_return_state_t (*msg_create)(shout_t *self, shout_connection_t *connection);
    shout_connection_return_state_t (*msg_get)(shout_t *self, shout_connection_t *connection);
    shout_connection_return_state_t (*msg_parse)(shout_t *self, shout_connection_t *connection);
    shout_connection_return_state_t (*protocol_iter)(shout_t *self, shout_connection_t *connection);
} shout_protocol_impl_t;

typedef int (*shout_connection_callback_t)(shout_connection_t *con, shout_event_t event, void *userdata, va_list ap);

struct shout_connection_tag {
    size_t                          refc;

    int                             selected_tls_mode;
    shout_connect_socket_state_t    target_socket_state;
    shout_connect_socket_state_t    current_socket_state;
    shout_connect_message_state_t   target_message_state;
    shout_connect_message_state_t   current_message_state;
    int                             target_protocol_state;
    int                             current_protocol_state;
    shout_protocol_extra_t          protocol_extra;

    const shout_protocol_impl_t *impl;
    const void *plan;

    int (*any_timeout)(shout_t *self, shout_connection_t *connection);
    int (*destory)(shout_connection_t *connection);

    int                             nonblocking;

    shout_connection_callback_t callback;
    void        *callback_userdata;

#ifdef HAVE_OPENSSL
    shout_tls_t   *tls;
#endif
    sock_t         socket;
    shout_queue_t  rqueue;
    shout_queue_t  wqueue;

    uint64_t       wait_timeout;
    /* maybe we want to convert this to general flag vector later */
    int            wait_timeout_happened;

    /* server capabilities (LIBSHOUT_CAP_*) */
    uint32_t server_caps;

    int error;
};

struct shout {
    /* hostname or IP of icecast server */
    char *host;
    /* port of the icecast server */
    int port;
    /* login password for the server */
    char *password;
    /* server protocol to use */
    unsigned int protocol;
    /* type of data being sent */
    unsigned int format;
    unsigned int usage;
    /* audio encoding parameters */
    util_dict *audio_info;
    /* content-language */
    char *content_language;

    /* user-agent to use when doing HTTP login */
    char *useragent;
    /* mountpoint for this stream */
    char *mount;
    /* all the meta data about the stream */
    util_dict *meta;
    /* icecast 1.x dumpfile */
    char *dumpfile;
    /* username to use for HTTP auth. */
    char *user;
    /* is this stream private? */
    int public;
    /* mimetype override */
    char *mimetype;

    shout_callback_t callback;
    void *callback_userdata;

    /* TLS options */
#ifdef HAVE_OPENSSL
    int          tls_mode;
    char        *ca_directory;
    char        *ca_file;
    char        *allowed_ciphers;
    char        *client_certificate;
#endif

    union {
        shout_http_plan_t http;
    } source_plan;

    /* socket the connection is on */
    shout_connection_t *connection;
    int             nonblocking;

    void *format_data;
    int (*send)(shout_t* self, const unsigned char* buff, size_t len);
    void (*close)(shout_t* self);

    /* start of this period's timeclock */
    uint64_t starttime;
    /* amount of data we've sent (in microseconds) */
    uint64_t senttime;

    int error;
};

/* helper functions */
const char *shout_get_mimetype_from_self(shout_t *self);
int shout_cb_connection_callback(shout_connection_t *con, shout_event_t event, void *userdata, va_list ap);

int     shout_queue_data(shout_queue_t *queue, const unsigned char *data, size_t len);
int     shout_queue_str(shout_connection_t *self, const char *str);
int     shout_queue_printf(shout_connection_t *self, const char *fmt, ...);
void    shout_queue_free(shout_queue_t *queue);
ssize_t shout_queue_collect(shout_buf_t *queue, char **buf);

/* transports */
ssize_t shout_conn_read(shout_t *self, void *buf, size_t len);
ssize_t shout_conn_write(shout_t *self, const void *buf, size_t len);
int     shout_conn_recoverable(shout_t *self);

/* connection */
ssize_t shout_connection__read(shout_connection_t *con, shout_t *shout, void *buf, size_t len);
int shout_connection__recoverable(shout_connection_t *con, shout_t *shout);
shout_connection_t *shout_connection_new(shout_t *self, const shout_protocol_impl_t *impl, const void *plan);
int                 shout_connection_ref(shout_connection_t *con);
int                 shout_connection_unref(shout_connection_t *con);
int                 shout_connection_iter(shout_connection_t *con, shout_t *shout);
int                 shout_connection_select_tlsmode(shout_connection_t *con, int tlsmode);
int                 shout_connection_set_nonblocking(shout_connection_t *con, unsigned int nonblocking);
int                 shout_connection_set_wait_timeout(shout_connection_t *con, shout_t *shout, uint64_t timeout /* [ms] */);
int                 shout_connection_get_wait_timeout_happened(shout_connection_t *con, shout_t *shout); /* returns SHOUTERR_* or > 0 for true */
int                 shout_connection_connect(shout_connection_t *con, shout_t *shout);
int                 shout_connection_disconnect(shout_connection_t *con);
ssize_t             shout_connection_send(shout_connection_t *con, shout_t *shout, const void *buf, size_t len);
ssize_t             shout_connection_get_sendq(shout_connection_t *con, shout_t *shout);
int                 shout_connection_starttls(shout_connection_t *con, shout_t *shout);
int                 shout_connection_set_error(shout_connection_t *con, int error);
int                 shout_connection_get_error(shout_connection_t *con);
int                 shout_connection_transfer_error(shout_connection_t *con, shout_t *shout);
int                 shout_connection_control(shout_connection_t *con, shout_control_t control, ...);
int                 shout_connection_set_callback(shout_connection_t *con, shout_connection_callback_t callback, void *userdata);

#ifdef HAVE_OPENSSL
typedef int (*shout_tls_callback_t)(shout_tls_t *tls, shout_event_t event, void *userdata, va_list ap);

shout_tls_t *shout_tls_new(shout_t *self, sock_t socket);
int          shout_tls_try_connect(shout_tls_t *tls);
int          shout_tls_close(shout_tls_t *tls);
ssize_t      shout_tls_read(shout_tls_t *tls, void *buf, size_t len);
ssize_t      shout_tls_write(shout_tls_t *tls, const void *buf, size_t len);
int          shout_tls_recoverable(shout_tls_t *tls);
int          shout_tls_get_peer_certificate(shout_tls_t *tls, char **buf);
int          shout_tls_get_peer_certificate_chain(shout_tls_t *tls, char **buf);
int          shout_tls_set_callback(shout_tls_t *tls, shout_tls_callback_t callback, void *userdata);
#endif

/* protocols */
extern const shout_protocol_impl_t *shout_http_impl;
extern const shout_protocol_impl_t *shout_xaudiocast_impl;
extern const shout_protocol_impl_t *shout_icy_impl;
extern const shout_protocol_impl_t *shout_roaraudio_impl;

shout_connection_return_state_t shout_get_xaudiocast_response(shout_t *self, shout_connection_t *connection);
shout_connection_return_state_t shout_parse_xaudiocast_response(shout_t *self, shout_connection_t *connection);

/* containers */
int shout_open_ogg(shout_t *self);
int shout_open_mp3(shout_t *self);
int shout_open_webm(shout_t *self);
int shout_open_text(shout_t *self);
int shout_open_adts(shout_t *self);

#endif /* __LIBSHOUT_SHOUT_PRIVATE_H__ */
