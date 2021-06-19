/* -*- c-basic-offset: 8; -*- */
/* proto_http.c: Implementation of protocol HTTP.
 *
 *  Copyright (C) 2002-2004 the Icecast team <team@icecast.org>,
 *  Copyright (C) 2012-2015 Philipp "ph3-der-loewe" Schafft <lion@lion.leolix.org>
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

char *shout_http_basic_authorization(shout_t *self)
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

int shout_create_http_request(shout_t *self)
{
    char        *auth;
    char        *ai;
    int          ret = SHOUTERR_MALLOC;
    util_dict   *dict;
    const char  *key, *val;
    const char  *mimetype;
    char        *mount = NULL;

    if (!(mimetype = shout_get_mimetype(self)))
        return SHOUTERR_INSANE;
        
    /* this is lazy code that relies on the only error from queue_* being
     * SHOUTERR_MALLOC
     */
    do {
        if (!(mount = _shout_util_url_encode_resource(self->mount)))
            break;
        if (shout_queue_printf(self, "SOURCE %s HTTP/1.0\r\n", mount))
            break;
        if (self->password && (self->server_caps & LIBSHOUT_CAP_GOTCAPS)) {
            if (! (auth = shout_http_basic_authorization(self)))
                break;
            if (shout_queue_str(self, auth)) {
                free(auth);
                break;
            }
            free(auth);
        }
        if (self->useragent && shout_queue_printf(self, "Host: %s:%i\r\n", self->host, self->port))
            break;
        if (self->useragent && shout_queue_printf(self, "User-Agent: %s\r\n", self->useragent))
            break;
        if (shout_queue_printf(self, "Content-Type: %s\r\n", mimetype))
            break;
        if (shout_queue_printf(self, "ice-public: %d\r\n", self->public))
            break;

        _SHOUT_DICT_FOREACH(self->meta, dict, key, val) {
            if (val && shout_queue_printf(self, "ice-%s: %s\r\n", key, val))
                break;
        }

        if ((ai = _shout_util_dict_urlencode(self->audio_info, ';'))) {
            if (shout_queue_printf(self, "ice-audio-info: %s\r\n", ai)) {
                free(ai);
                break;
            }
            free(ai);
        }
        if (shout_queue_str(self, "\r\n"))
            break;

        ret = SHOUTERR_SUCCESS;
    } while (0);

    if (mount)
        free(mount);

    return ret;
}

int shout_create_http_request_upgrade(shout_t *self, const char *proto)
{
    do {
        if (shout_queue_str(self, "GET / HTTP/1.1\r\nConnection: Upgrade\r\n"))
            break;
        if (shout_queue_printf(self, "Upgrade: %s\r\n", proto))
            break;
        /* Send Host:-header as this one may be used to select cert! */
        if (shout_queue_printf(self, "Host: %s:%i\r\n", self->host, self->port))
            break;
        if (shout_queue_str(self, "\r\n"))
            break;
        return SHOUTERR_SUCCESS;
    } while (0);

    return SHOUTERR_MALLOC;
}

int shout_get_http_response(shout_t *self)
{
    int          blen;
    char        *pc;
    shout_buf_t *queue;
    int          newlines = 0;

    /* work from the back looking for \r?\n\r?\n. Anything else means more
     * is coming.
     */
    for (queue = self->rqueue.head; queue->next; queue = queue->next) ;
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

        if (newlines == 2)
            return SHOUTERR_SUCCESS;

        blen--;
        pc--;

        if (!blen && queue->prev) {
            queue = queue->prev;
            pc = (char*)queue->data + queue->len - 1;
            blen = queue->len;
        }
    }

    return SHOUTERR_BUSY;
}

static inline void parse_http_response_caps(shout_t *self, const char *header, const char *str) {
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
                self->server_caps |= LIBSHOUT_CAP_SOURCE;
            } else if (strcasecmp(buf, "PUT") == 0) {
                self->server_caps |= LIBSHOUT_CAP_PUT;
            } else if (strcasecmp(buf, "POST") == 0) {
                self->server_caps |= LIBSHOUT_CAP_POST;
            } else if (strcasecmp(buf, "GET") == 0) {
                self->server_caps |= LIBSHOUT_CAP_GET;
            }
        } else if (strcmp(header, "Accept-Encoding") == 0) {
            if (strcasecmp(buf, "chunked") == 0) {
                self->server_caps |= LIBSHOUT_CAP_CHUNKED;
            }
        } else if (strcmp(header, "Upgrade") == 0) {
            if (strcasecmp(buf, "TLS/1.0") == 0) {
                self->server_caps |= LIBSHOUT_CAP_UPGRADETLS;
            }
        } else {
            return;             /* unknown header */
        }

        str += len + 1;
    } while (end);

    return;
}

static inline int eat_body(shout_t *self, size_t len, const char *buf, size_t buflen)
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
        got = shout_conn_read(self, buffer, len > sizeof(buffer) ? sizeof(buffer) : len);
        if (got == -1 && shout_conn_recoverable(self)) {
            continue;
        } else if (got == -1) {
            return -1;
        }

        len -= got;
    }

    return 0;
}

int shout_parse_http_response(shout_t *self)
{
    http_parser_t   *parser;
    char            *header = NULL;
    ssize_t          hlen;
    int              code;
    const char      *retcode;
    int              ret;
    char            *mount;

    /* all this copying! */
    hlen = shout_queue_collect(self->rqueue.head, &header);
    if (hlen <= 0)
        return SHOUTERR_MALLOC;
    shout_queue_free(&self->rqueue);

    parser = httpp_create_parser();
    httpp_initialize(parser, NULL);

    if (!(mount = _shout_util_url_encode(self->mount))) {
        httpp_destroy(parser);
        free(header);
        return SHOUTERR_MALLOC;
    }

    ret = httpp_parse_response(parser, header, hlen, mount);
    free(mount);

    if (ret) {
        /* TODO: Headers to Handle:
         * Allow:, Accept-Encoding:, Warning:, Upgrade:
         */
        parse_http_response_caps(self, "Allow", httpp_getvar(parser, "allow"));
        parse_http_response_caps(self, "Accept-Encoding", httpp_getvar(parser, "accept-encoding"));
        parse_http_response_caps(self, "Upgrade", httpp_getvar(parser, "upgrade"));
        self->server_caps |= LIBSHOUT_CAP_GOTCAPS;
        retcode = httpp_getvar(parser, HTTPP_VAR_ERROR_CODE);
        code = atoi(retcode);
        if (code >= 200 && code < 300) {
            httpp_destroy(parser);
            free(header);
            return SHOUTERR_SUCCESS;
        } else if (code == 401 || code == 405 || code == 426 || code == 101) {
            const char *content_length = httpp_getvar(parser, "content-length");
            if (content_length) {
                if (eat_body(self, atoi(content_length), header, hlen) == -1)
                    goto failure;
            }
#ifdef HAVE_OPENSSL
            switch (code) {
                case 426:
                    self->tls_mode = SHOUT_TLS_RFC2817;
                break;

                case 101:
                    self->upgrade_to_tls = 1;
                break;
            }
#endif
            self->retry++;
            if (self->retry > LIBSHOUT_MAX_RETRY)
                self->retry = 0;

            goto retry;
        } else {
            self->retry = 0;
        }
    }

failure:
    self->retry = 0;
retry:
    free(header);
    httpp_destroy(parser);
    return self->error = SHOUTERR_NOLOGIN;
}
