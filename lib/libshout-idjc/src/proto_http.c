/* -*- c-basic-offset: 8; -*- */
/* proto_http.c: Implementation of protocol HTTP.
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

#ifdef HAVE_CONFIG_H
#   include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <shoutidjc/shout.h>
#include "shout_private.h"
#include "common/httpp/httpp.h"

typedef enum {
    STATE_CHALLENGE = 0,
    STATE_SOURCE,
    STATE_UPGRADE,
    STATE_POKE
} shout_http_protocol_state_t;

static char *shout_http_basic_authorization(shout_t *self)
{
    char *out, *in;
    int   len;

    if (!self || !self->user || !self->password)
        return NULL;

    len = strlen(self->user) + strlen(self->password) + 2;
    if (!(in = malloc(len)))
        return NULL;
    snprintf(in, len, "%s:%s", self->user, self->password);
    out = _shout_util_base64_encode(in);
    free(in);

    len = strlen(out) + 24;
    if (!(in = malloc(len))) {
        free(out);
        return NULL;
    }
    snprintf(in, len, "Authorization: Basic %s\r\n", out);
    free(out);

    return in;
}

static shout_connection_return_state_t shout_parse_http_select_next_state(shout_t *self, shout_connection_t *connection, int can_reuse, shout_http_protocol_state_t state)
{
    if (!can_reuse) {
        shout_connection_disconnect(connection);
        shout_connection_connect(connection, self);
    }
    connection->current_message_state = SHOUT_MSGSTATE_CREATING0;
    connection->target_message_state = SHOUT_MSGSTATE_SENDING1;
    connection->current_protocol_state = state;
    return SHOUT_RS_NOTNOW;
}

static shout_connection_return_state_t shout_create_http_request_source(shout_t *self, shout_connection_t *connection, int auth, int poke)
{
    char        *basic_auth;
    char        *ai;
    int          ret = SHOUTERR_MALLOC;
    util_dict   *dict;
    const char  *key, *val;
    const char  *mimetype;
    char        *mount = NULL;

    mimetype = shout_get_mimetype_from_self(self);
    if (!mimetype) {
        shout_connection_set_error(connection, SHOUTERR_INSANE);
        return SHOUT_RS_ERROR;
    }

    /* this is lazy code that relies on the only error from queue_* being
     * SHOUTERR_MALLOC
     */
    do {
        if (!(mount = _shout_util_url_encode_resource(self->mount)))
            break;
        if (connection->server_caps & LIBSHOUT_CAP_PUT) {
            if (shout_queue_printf(connection, "PUT %s HTTP/1.1\r\n", mount))
                break;
        } else {
            if (shout_queue_printf(connection, "SOURCE %s HTTP/1.0\r\n", mount))
                break;
        }
        if (self->password && auth) {
            if (! (basic_auth = shout_http_basic_authorization(self)))
                break;
            if (shout_queue_str(connection, basic_auth)) {
                free(basic_auth);
                break;
            }
            free(basic_auth);
        }
        if (shout_queue_printf(connection, "Host: %s:%i\r\n", self->host, self->port))
            break;
        if (self->useragent && shout_queue_printf(connection, "User-Agent: %s\r\n", self->useragent))
            break;
        if (shout_queue_printf(connection, "Content-Type: %s\r\n", mimetype))
            break;
        if (self->content_language && shout_queue_printf(connection, "Content-Language: %s\r\n", self->content_language))
            break;
        if (poke) {
            if (shout_queue_str(connection, "Content-Length: 0\r\nConnection: Keep-Alive\r\n"))
                break;
        } else if (connection->server_caps & LIBSHOUT_CAP_PUT) {
            if (shout_queue_printf(connection, "Expect: 100-continue\r\n", mount))
                break;
            /* Set timeout for 100-continue to 4s = 4000 ms. This is a little less than the default source_timeout/2. */
            shout_connection_set_wait_timeout(connection, self, 4000 /* [ms] */);
        }
        if (shout_queue_printf(connection, "ice-public: %d\r\n", self->public))
            break;

        _SHOUT_DICT_FOREACH(self->meta, dict, key, val) {
            if (val && shout_queue_printf(connection, "ice-%s: %s\r\n", key, val))
                break;
        }

        if ((ai = _shout_util_dict_urlencode(self->audio_info, ';'))) {
            if (shout_queue_printf(connection, "ice-audio-info: %s\r\n", ai)) {
                free(ai);
                break;
            }
            free(ai);
        }
        if (shout_queue_str(connection, "Prefer: return=minimal\r\n"))
            break;
        if (shout_queue_str(connection, "\r\n"))
            break;

        ret = SHOUTERR_SUCCESS;
    } while (0);

    if (mount)
        free(mount);

    shout_connection_set_error(connection, ret);
    return ret == SHOUTERR_SUCCESS ? SHOUT_RS_DONE : SHOUT_RS_ERROR;
}

static shout_connection_return_state_t shout_create_http_request_generic(shout_t *self, shout_connection_t *connection, const char *method, const char *res, const char *param, int fake_ua, const char *upgrade, int auth)
{
    int          ret = SHOUTERR_MALLOC;
    int          is_post = 0;
    char        *basic_auth;

    if (method) {
        is_post = strcmp(method, "POST") == 0;
    } else {
        if (connection->server_caps & LIBSHOUT_CAP_POST) {
            method = "POST";
            is_post = 1;
        } else {
            method = "GET";
            is_post = 0;
        }
    }

    /* this is lazy code that relies on the only error from queue_* being
     * SHOUTERR_MALLOC
     */
    do {
        ret = SHOUTERR_SUCCESS;

        if (!param || is_post) {
            if (shout_queue_printf(connection, "%s %s HTTP/1.1\r\n", method, res))
                break;
        } else {
            if (shout_queue_printf(connection, "%s %s?%s HTTP/1.1\r\n", method, res, param))
                break;
        }

        /* Send Host:-header as this one may be used to select cert! */
        if (shout_queue_printf(connection, "Host: %s:%i\r\n", self->host, self->port))
            break;

        if (fake_ua) {
            /* Thank you Nullsoft for your broken software. */
            if (self->useragent && shout_queue_printf(connection, "User-Agent: %s (Mozilla compatible)\r\n", self->useragent))
                break;
        } else {
            if (self->useragent && shout_queue_printf(connection, "User-Agent: %s\r\n", self->useragent))
                break;
        }

        if (self->password && auth) {
            if (! (basic_auth = shout_http_basic_authorization(self)))
                break;
            if (shout_queue_str(connection, basic_auth)) {
                free(basic_auth);
                break;
            }
            free(basic_auth);
        }

        if (upgrade) {
            if (shout_queue_printf(connection, "Connection: Upgrade\r\nUpgrade: %s\r\n", upgrade))
                break;
        }

        if (param && is_post) {
            if (shout_queue_printf(connection, "Content-Type: application/x-www-form-urlencoded\r\nContent-Length: %llu\r\n", (long long unsigned int)strlen(param)))
                break;
        }

        /* End of request */
        if (shout_queue_str(connection, "\r\n"))
            break;
        if (param && is_post) {
            if (shout_queue_str(connection, param))
                break;
        }
    } while (0);

    shout_connection_set_error(connection, ret);
    return ret == SHOUTERR_SUCCESS ? SHOUT_RS_DONE : SHOUT_RS_ERROR;
}

static shout_connection_return_state_t shout_create_http_request(shout_t *self, shout_connection_t *connection)
{
    const shout_http_plan_t *plan = connection->plan;

    if (!plan) {
        shout_connection_set_error(connection, SHOUTERR_INSANE);
        return SHOUT_RS_ERROR;
    }

#ifdef HAVE_OPENSSL
    if (!connection->tls) {
        /* Why not try Upgrade? */
        if ((connection->selected_tls_mode == SHOUT_TLS_AUTO || connection->selected_tls_mode == SHOUT_TLS_AUTO_NO_PLAIN) &&
                !(connection->server_caps & LIBSHOUT_CAP_GOTCAPS) &&
                connection->current_protocol_state == STATE_CHALLENGE) {
            connection->current_protocol_state = STATE_UPGRADE;
        }

        if (connection->selected_tls_mode == SHOUT_TLS_RFC2817) {
            connection->current_protocol_state = STATE_UPGRADE;
        }
    }
#endif

    switch ((shout_http_protocol_state_t)connection->current_protocol_state) {
        case STATE_CHALLENGE:
            connection->server_caps |= LIBSHOUT_CAP_CHALLENGED;
            if (plan->is_source) {
                return shout_create_http_request_source(self, connection, 0, 1);
            } else {
                return shout_create_http_request_generic(self, connection, plan->method, plan->resource, plan->param, plan->fake_ua, NULL, 0);
            }
        break;
        case STATE_SOURCE:
            /* Just an extra layer of safety */
            switch (connection->selected_tls_mode) {
                case SHOUT_TLS_AUTO_NO_PLAIN:
                case SHOUT_TLS_RFC2817:
                case SHOUT_TLS_RFC2818:
#ifdef HAVE_OPENSSL
                    if (!connection->tls) {
                        /* TLS requested but for some reason not established. NOT sending credentials. */
                        shout_connection_set_error(connection, SHOUTERR_INSANE);
                        return SHOUT_RS_ERROR;
                    }
#else
                    shout_connection_set_error(connection, SHOUTERR_UNSUPPORTED);
                    return SHOUT_RS_ERROR;
#endif
                break;
            }

            if (plan->is_source) {
                return shout_create_http_request_source(self, connection, 1, 0);
            } else {
                return shout_create_http_request_generic(self, connection, plan->method, plan->resource, plan->param, plan->fake_ua, NULL, plan->auth);
            }
        break;
        case STATE_UPGRADE:
            return shout_create_http_request_generic(self, connection, "OPTIONS", "*", NULL, 0, "TLS/1.0, HTTP/1.1", 0);
        break;
        case STATE_POKE:
            return shout_create_http_request_generic(self, connection, "GET", "/admin/!POKE", NULL, 0, NULL, 0);
        break;
        default:
            shout_connection_set_error(connection, SHOUTERR_INSANE);
            return SHOUT_RS_ERROR;
        break;
    }
}

static shout_connection_return_state_t shout_get_http_response(shout_t *self, shout_connection_t *connection)
{
    int          blen;
    char        *pc;
    shout_buf_t *queue;
    int          newlines = 0;

    if (!connection->rqueue.len) {
#ifdef HAVE_OPENSSL
        if (!connection->tls && (connection->selected_tls_mode == SHOUT_TLS_AUTO || connection->selected_tls_mode == SHOUT_TLS_AUTO_NO_PLAIN)) {
            if (connection->current_protocol_state == STATE_POKE) {
                shout_connection_select_tlsmode(connection, SHOUT_TLS_RFC2818);
                return shout_parse_http_select_next_state(self, connection, 0, STATE_CHALLENGE);
            } else {
                return shout_parse_http_select_next_state(self, connection, 0, STATE_POKE);
            }
        }
#endif
        shout_connection_set_error(connection, SHOUTERR_SOCKET);
        return SHOUT_RS_ERROR;
    }

    /* work from the back looking for \r?\n\r?\n. Anything else means more
     * is coming.
     */
    for (queue = connection->rqueue.head; queue->next; queue = queue->next) ;
    pc = (char*)queue->data + queue->len - 1;
    blen = queue->len;
    while (blen) {
        if (*pc == '\n') {
            newlines++;
        } else if (*pc != '\r') {
            /* we may have to scan the entire queue if we got a response with
             * data after the head line (this can happen with eg 401)
             */
            newlines = 0;
        }

        if (newlines == 2) {
            return SHOUT_RS_DONE;
        }

        blen--;
        pc--;

        if (!blen && queue->prev) {
            queue = queue->prev;
            pc = (char*)queue->data + queue->len - 1;
            blen = queue->len;
        }
    }

    return SHOUT_RS_NOTNOW;
}

static inline void parse_http_response_caps(shout_t *self, shout_connection_t *connection, const char *header, const char *str) {
    const char *end;
    size_t      len;
    char        buf[64];

    if (!self || !header || !str)
        return;

    do {
        for (; *str == ' '; str++) ;
        end = strstr(str, ",");
        if (end) {
            len = end - str;
        } else {
            len = strlen(str);
        }

        if (len > (sizeof(buf) - 1))
            return;
        memcpy(buf, str, len);
        buf[len] = 0;

        if (strcmp(header, "Allow") == 0) {
            if (strcasecmp(buf, "SOURCE") == 0) {
                connection->server_caps |= LIBSHOUT_CAP_SOURCE;
            } else if (strcasecmp(buf, "PUT") == 0) {
                connection->server_caps |= LIBSHOUT_CAP_PUT;
            } else if (strcasecmp(buf, "POST") == 0) {
                connection->server_caps |= LIBSHOUT_CAP_POST;
            } else if (strcasecmp(buf, "GET") == 0) {
                connection->server_caps |= LIBSHOUT_CAP_GET;
            } else if (strcasecmp(buf, "OPTIONS") == 0) {
                connection->server_caps |= LIBSHOUT_CAP_OPTIONS;
            }
        } else if (strcmp(header, "Accept-Encoding") == 0) {
            if (strcasecmp(buf, "chunked") == 0) {
                connection->server_caps |= LIBSHOUT_CAP_CHUNKED;
            }
        } else if (strcmp(header, "Upgrade") == 0) {
            if (strcasecmp(buf, "TLS/1.0") == 0) {
                connection->server_caps |= LIBSHOUT_CAP_UPGRADETLS;
            }
        } else {
            return;             /* unknown header */
        }

        str += len + 1;
    } while (end);

    return;
}

static inline int eat_body(shout_t *self, shout_connection_t *connection, size_t len, const char *buf, size_t buflen)
{
    const char  *p;
    size_t       header_len = 0;
    char         buffer[256];
    ssize_t      got;

    if (!len)
        return 0;

    for (p = buf; p < (buf + buflen - 3); p++) {
        if (p[0] == '\r' && p[1] == '\n' && p[2] == '\r' && p[3] == '\n') {
            header_len = p - buf + 4;
            break;
        } else if (p[0] == '\n' && p[1] == '\n') {
            header_len = p - buf + 2;
            break;
        }
    }
    if (!header_len && buflen >= 3 && buf[buflen - 2] == '\n' && buf[buflen - 3] == '\n') {
        header_len = buflen - 1;
    } else if (!header_len && buflen >= 2 && buf[buflen - 1] == '\n' && buf[buflen - 2] == '\n') {
        header_len = buflen;
    }

    if ((buflen - header_len) > len)
        return -1;

    len -= buflen - header_len;

    while (len) {
        got = shout_connection__read(connection, self, buffer, len > sizeof(buffer) ? sizeof(buffer) : len);
        if (got == -1 && shout_connection__recoverable(connection, self)) {
            continue;
        } else if (got == -1) {
            return -1;
        }

        len -= got;
    }

    return 0;
}

static shout_connection_return_state_t shout_parse_http_response(shout_t *self, shout_connection_t *connection)
{
    http_parser_t   *parser;
    char            *header = NULL;
    ssize_t          hlen;
    int              code;
    const char      *retcode;
    int              ret;
    char            *mount;
    int              consider_retry = 0;
    int              can_reuse = 0;
#ifdef HAVE_STRCASESTR
    const char      *tmp;
#endif

    /* all this copying! */
    hlen = shout_queue_collect(connection->rqueue.head, &header);
    if (hlen <= 0) {
        if (connection->current_protocol_state == STATE_SOURCE && shout_connection_get_wait_timeout_happened(connection, self) > 0) {
            connection->current_message_state = SHOUT_MSGSTATE_SENDING1;
            connection->target_message_state = SHOUT_MSGSTATE_WAITING1;
            return SHOUT_RS_DONE;
        } else {
            shout_connection_set_error(connection, SHOUTERR_MALLOC);
            return SHOUT_RS_ERROR;
        }
    }
    shout_queue_free(&connection->rqueue);

    parser = httpp_create_parser();
    httpp_initialize(parser, NULL);

    if (!(mount = _shout_util_url_encode(self->mount))) {
        httpp_destroy(parser);
        free(header);
        shout_connection_set_error(connection, SHOUTERR_MALLOC);
        return SHOUT_RS_ERROR;
    }

    ret = httpp_parse_response(parser, header, hlen, mount);
    free(mount);

    if (ret) {
        /* TODO: Headers to Handle:
         * Allow:, Accept-Encoding:, Warning:, Upgrade:
         */
        parse_http_response_caps(self, connection, "Allow", httpp_getvar(parser, "allow"));
        parse_http_response_caps(self, connection, "Accept-Encoding", httpp_getvar(parser, "accept-encoding"));
        parse_http_response_caps(self, connection, "Upgrade", httpp_getvar(parser, "upgrade"));
        connection->server_caps |= LIBSHOUT_CAP_GOTCAPS;
        retcode = httpp_getvar(parser, HTTPP_VAR_ERROR_CODE);
        code = atoi(retcode);

#ifdef HAVE_STRCASESTR
        tmp = httpp_getvar(parser, HTTPP_VAR_VERSION);
        if (tmp && strcmp(tmp, "1.1") == 0) {
            can_reuse = 1;
        }
        tmp = httpp_getvar(parser, "connection");
        if (tmp && strcasestr(tmp, "keep-alive")) {
            can_reuse = 1;
        }
        if (tmp && strcasestr(tmp, "close")) {
            can_reuse = 0;
        }
#else
        /* get a real OS */
        can_reuse = 0;
#endif

        if ((code == 100 || (code >= 200 && code < 300)) && connection->current_protocol_state == STATE_SOURCE) {
            httpp_destroy(parser);
            free(header);
            connection->current_message_state = SHOUT_MSGSTATE_SENDING1;
            connection->target_message_state = SHOUT_MSGSTATE_WAITING1;
            return SHOUT_RS_DONE;
        } else if ((code >= 200 && code < 300) || code == 400 || code == 401 || code == 405 || code == 426 || code == 101) {
            const char *content_length = httpp_getvar(parser, "content-length");
            if (content_length) {
                if (eat_body(self, connection, atoi(content_length), header, hlen) == -1) {
                    can_reuse = 0;
                    goto failure;
                }
            }
#ifdef HAVE_OPENSSL
            switch (code) {
                case 400:
                    if (connection->current_protocol_state != STATE_UPGRADE && connection->current_protocol_state != STATE_POKE) {
                        free(header);
                        httpp_destroy(parser);
                        shout_connection_set_error(connection, SHOUTERR_NOLOGIN);
                        return SHOUT_RS_ERROR;
                    }
                    if (connection->selected_tls_mode == SHOUT_TLS_AUTO_NO_PLAIN) {
                        can_reuse = 0;
                        shout_connection_select_tlsmode(connection, SHOUT_TLS_RFC2818);
                    }
                break;

                case 426:
                    if (connection->tls) {
                        free(header);
                        httpp_destroy(parser);
                        shout_connection_set_error(connection, SHOUTERR_NOLOGIN);
                        return SHOUT_RS_ERROR;
                    } else if (connection->selected_tls_mode == SHOUT_TLS_DISABLED) {
                        free(header);
                        httpp_destroy(parser);
                        shout_connection_set_error(connection, SHOUTERR_NOCONNECT);
                        return SHOUT_RS_ERROR;
                    } else {
                        /* Reset challenge state here as we do not know if it's the same inside TLS */
                        connection->server_caps |= LIBSHOUT_CAP_CHALLENGED;
                        connection->server_caps -= LIBSHOUT_CAP_CHALLENGED;
                        shout_connection_select_tlsmode(connection, SHOUT_TLS_RFC2817);
                        free(header);
                        httpp_destroy(parser);
                        return shout_parse_http_select_next_state(self, connection, can_reuse, STATE_UPGRADE);
                    }
                break;

                case 101:
                    shout_connection_select_tlsmode(connection, SHOUT_TLS_RFC2817);
                    shout_connection_starttls(connection, self);
                break;
            }
#endif
            consider_retry = 1;
        }

        if (code >= 100 && code < 200) {
            connection->current_message_state = SHOUT_MSGSTATE_WAITING0;
            return SHOUT_RS_NOTNOW;
        }
    }

failure:
    free(header);
    httpp_destroy(parser);

    if (consider_retry) {
        switch ((shout_http_protocol_state_t)connection->current_protocol_state) {
            case STATE_CHALLENGE:
                return shout_parse_http_select_next_state(self, connection, can_reuse, STATE_SOURCE);
            break;
            case STATE_UPGRADE:
            case STATE_POKE:
                if (connection->server_caps & LIBSHOUT_CAP_CHALLENGED) {
                    return shout_parse_http_select_next_state(self, connection, can_reuse, STATE_SOURCE);
                } else {
                    return shout_parse_http_select_next_state(self, connection, can_reuse, STATE_CHALLENGE);
                }
            break;
            case STATE_SOURCE:
                /* no-op */
            break;
        }
    }
    shout_connection_set_error(connection, SHOUTERR_NOLOGIN);
    return SHOUT_RS_ERROR;
}

static const shout_protocol_impl_t shout_http_impl_real = {
    .msg_create = shout_create_http_request,
    .msg_get = shout_get_http_response,
    .msg_parse = shout_parse_http_response
};
const shout_protocol_impl_t * shout_http_impl = &shout_http_impl_real;
