/* -*- c-basic-offset: 8; -*- */
/* shout.c: Implementation of public libshout interface shout.h
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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <shoutidjc/shout.h>

#include <common/net/sock.h>
#include <common/timing/timing.h>
#include <common/httpp/httpp.h>

#include "shout_private.h"
#include "util.h"

#ifdef _MSC_VER
#   ifndef va_copy
#       define va_copy(ap1, ap2) memcpy(&ap1, &ap2, sizeof(va_list))
#   endif
#   define vsnprintf      _vsnprintf
#   define inline         _inline
#endif

/* -- local prototypes -- */
static int send_queue(shout_t *self);
static int get_response(shout_t *self);
static int try_connect(shout_t *self);
static int try_write(shout_t *self, const void *data, size_t len);

static int create_request(shout_t *self);
static int parse_response(shout_t *self);

/* -- static data -- */
static int _initialized = 0;

/* -- public functions -- */

void shout_init(void)
{
    if (_initialized)
        return;

    sock_initialize();
    _initialized = 1;
}

void shout_shutdown(void)
{
    if (!_initialized)
        return;

    sock_shutdown();
    _initialized = 0;
}

shout_t *shout_new(void)
{
    shout_t *self;

    /* in case users haven't done this explicitly. Should we error
     * if not initialized instead? */
    shout_init();

    if (!(self = (shout_t*)calloc(1, sizeof(shout_t)))) {
        return NULL;
    }

    if (shout_set_host(self, LIBSHOUT_DEFAULT_HOST) != SHOUTERR_SUCCESS) {
        shout_free(self);
        return NULL;
    }

    if (shout_set_user(self, LIBSHOUT_DEFAULT_USER) != SHOUTERR_SUCCESS) {
        shout_free(self);
        return NULL;
    }

    if (shout_set_agent(self, LIBSHOUT_DEFAULT_USERAGENT) != SHOUTERR_SUCCESS) {
        shout_free(self);
        return NULL;
    }

    if (!(self->audio_info = _shout_util_dict_new())) {
        shout_free(self);
        return NULL;
    }

    if (!(self->meta = _shout_util_dict_new())) {
        shout_free(self);
        return NULL;
    }

    if (shout_set_meta(self, "name", "no name") != SHOUTERR_SUCCESS) {
        shout_free(self);
        return NULL;
    }

#ifdef HAVE_OPENSSL
    if (shout_set_allowed_ciphers(self, LIBSHOUT_DEFAULT_ALLOWED_CIPHERS) != SHOUTERR_SUCCESS) {
        shout_free(self);
        return NULL;
    }

    self->tls_mode      = SHOUT_TLS_AUTO;
    self->tls_mode_used = SHOUTERR_NOTLS;
#endif

    self->port      = LIBSHOUT_DEFAULT_PORT;
    self->format    = LIBSHOUT_DEFAULT_FORMAT;
    self->protocol  = LIBSHOUT_DEFAULT_PROTOCOL;

    return self;
}

void shout_free(shout_t *self)
{
    if (!self)
        return;

    if (self->state != SHOUT_STATE_UNCONNECTED)
        return;

    if (self->host)
        free(self->host);
    if (self->password)
        free(self->password);
    if (self->mount)
        free(self->mount);
    if (self->user)
        free(self->user);
    if (self->useragent)
        free(self->useragent);
    if (self->audio_info)
        _shout_util_dict_free (self->audio_info);
    if (self->meta)
        _shout_util_dict_free (self->meta);

#ifdef HAVE_OPENSSL
    if (self->ca_directory)
        free(self->ca_directory);
    if (self->ca_file)
        free(self->ca_file);
    if (self->allowed_ciphers)
        free(self->allowed_ciphers);
    if (self->client_certificate)
        free(self->client_certificate);
#endif

    free(self);
}

int shout_open(shout_t *self)
{
    /* sanity check */
    if (!self)
        return SHOUTERR_INSANE;
    if (self->state != SHOUT_STATE_UNCONNECTED)
        return SHOUTERR_CONNECTED;
    if (!self->host || !self->password || !self->port)
        return self->error = SHOUTERR_INSANE;
    if (self->format == SHOUT_FORMAT_OGG &&  (self->protocol != SHOUT_PROTOCOL_HTTP && self->protocol != SHOUT_PROTOCOL_ROARAUDIO))
        return self->error = SHOUTERR_UNSUPPORTED;

    return self->error = try_connect(self);
}


int shout_close(shout_t *self)
{
    if (!self)
        return SHOUTERR_INSANE;

    if (self->state == SHOUT_STATE_UNCONNECTED)
        return self->error = SHOUTERR_UNCONNECTED;

    if (self->state == SHOUT_STATE_CONNECTED && self->close)
        self->close(self);

#ifdef HAVE_OPENSSL
    if (self->tls)
        shout_tls_close(self->tls);
    self->tls = NULL;
#endif

    sock_close(self->socket);
    self->state = SHOUT_STATE_UNCONNECTED;
    self->starttime = 0;
    self->senttime = 0;
    shout_queue_free(&self->rqueue);
    shout_queue_free(&self->wqueue);

    return self->error = SHOUTERR_SUCCESS;
}

int shout_send(shout_t *self, const unsigned char *data, size_t len)
{
    if (!self)
        return SHOUTERR_INSANE;

    if (self->state != SHOUT_STATE_CONNECTED)
        return self->error = SHOUTERR_UNCONNECTED;

    if (self->starttime <= 0)
        self->starttime = timing_get_time();

    if (!len)
        return send_queue(self);

    return self->send(self, data, len);
}

ssize_t shout_send_raw(shout_t *self, const unsigned char *data, size_t len)
{
    ssize_t ret;

    if (!self)
        return SHOUTERR_INSANE;

    if (self->state != SHOUT_STATE_CONNECTED)
        return SHOUTERR_UNCONNECTED;

    self->error = SHOUTERR_SUCCESS;

    /* send immediately if possible (should be the common case) */
    if (len && !self->wqueue.len) {
        if ((ret = try_write(self, data, len)) < 0)
            return self->error;
        if (ret < (ssize_t)len) {
            self->error = shout_queue_data(&self->wqueue, data + ret, len - ret);
            if (self->error != SHOUTERR_SUCCESS)
                return self->error;
        }
        return len;
    }

    self->error = shout_queue_data(&self->wqueue, data, len);
    if (self->error != SHOUTERR_SUCCESS)
        return self->error;

    ret = send_queue(self);
    if (ret == SHOUTERR_SUCCESS || (len && ret == SHOUTERR_BUSY))
        return len;

    return ret;
}

ssize_t shout_queuelen(shout_t *self)
{
    if (!self)
        return SHOUTERR_INSANE;

    return (ssize_t)self->wqueue.len;
}


void shout_sync(shout_t *self)
{
    int64_t sleep;

    if (!self)
        return;

    if (self->senttime == 0)
        return;

    sleep = self->senttime / 1000 - (timing_get_time() - self->starttime);
    if (sleep > 0)
        timing_sleep((uint64_t)sleep);

}

int shout_delay(shout_t *self)
{

    if (!self)
        return 0;

    if (self->senttime == 0)
        return 0;

    return self->senttime / 1000 - (timing_get_time() - self->starttime);
}

shout_metadata_t *shout_metadata_new(void)
{
    return _shout_util_dict_new();
}

void shout_metadata_free(shout_metadata_t *self)
{
    if (!self)
        return;

    _shout_util_dict_free(self);
}

int shout_metadata_add(shout_metadata_t *self, const char *name, const char *value)
{
    if (!self || !name)
        return SHOUTERR_INSANE;

    return _shout_util_dict_set(self, name, value);
}

/* open second socket to server, send HTTP request to change metadata.
 * TODO: prettier error-handling.
 */
int shout_set_metadata(shout_t *self, shout_metadata_t *metadata)
{
    int         error;
    sock_t      socket = -1;
    int         rv;
    char       *encvalue = NULL;
    const char *request_template;
    char       *request = NULL;
    size_t      request_len;
    char       *auth = NULL;
    char       *mount = NULL;
#ifdef HAVE_OPENSSL
    shout_tls_t *tls = NULL;
#endif

    if (!self || !metadata)
        return SHOUTERR_INSANE;

    if (!(encvalue = _shout_util_dict_urlencode(metadata, '&')))
        goto error_malloc;

    if (!(mount = _shout_util_url_encode(self->mount)))
        goto error_malloc;

    switch (self->protocol) {
    case SHOUT_PROTOCOL_ICY:
        request_template = "GET /admin.cgi?mode=updinfo&pass=%s&%s HTTP/1.0\r\nUser-Agent: %s (Mozilla compatible)\r\n\r\n";
        request_len = strlen(request_template) + strlen(self->password) + strlen(encvalue) + strlen(shout_get_agent(self)) + 1;
        if (!(request = malloc(request_len)))
            goto error_malloc;
        snprintf(request, request_len, request_template, self->password, encvalue, shout_get_agent(self));
        break;
    case SHOUT_PROTOCOL_HTTP:
        auth = shout_http_basic_authorization(self);

        request_template = "GET /admin/metadata?mode=updinfo&mount=%s&%s HTTP/1.0\r\nUser-Agent: %s\r\n%s\r\n";
        request_len = strlen(request_template) + strlen(mount) + strlen(encvalue) + strlen(shout_get_agent(self)) + 1;
        if (auth)
            request_len += strlen(auth);
        if (!(request = malloc(request_len)))
            goto error_malloc;
        snprintf(request, request_len, request_template, mount, encvalue, shout_get_agent(self), auth ? auth : "");
        break;
    default:
        request_template = "GET /admin.cgi?mode=updinfo&pass=%s&mount=%s&%s HTTP/1.0\r\nUser-Agent: %s\r\n\r\n";
        request_len = strlen(request_template) + strlen(self->password) + strlen(mount) + strlen(encvalue) + strlen(shout_get_agent(self)) + 1;
        if (!(request = malloc(request_len)))
            goto error_malloc;
        snprintf(request, request_len, request_template, self->password, mount, encvalue, shout_get_agent(self));
        break;
    }

    free(encvalue);
    encvalue = NULL;

    free(mount);
    mount = NULL;

    if (auth)
        free(auth);
    auth = NULL;

    if ((socket = sock_connect(self->host, self->port)) <= 0)
        return SHOUTERR_NOCONNECT;

#ifdef HAVE_OPENSSL
    switch (self->tls_mode_used) {
        case SHOUT_TLS_DISABLED:
            /* nothing to do */
        break;

        case SHOUT_TLS_RFC2817:
            /* Use TLS via HTTP Upgrade:-header [RFC2817]. */
            do {
                /* use a subscope to avoid more function level variables */
                char    upgrade[512];
                size_t  len;

                /* send upgrade request */
                snprintf(upgrade, sizeof(upgrade),
                    "GET / HTTP/1.1\r\nConnection: Upgrade\r\nUpgrade: TLS/1.0\r\nHost: %s:%i\r\n\r\n",
                    self->host, self->port);

                upgrade[sizeof(upgrade) - 1] = 0;
                len = strlen(upgrade);
                if (len == (sizeof(upgrade) - 1))
                    goto error_malloc;

                rv = sock_write_bytes(socket, upgrade, len);
                if (len != (size_t)rv)
                    goto error_socket;

                /* read status line */
                if (!sock_read_line(socket, upgrade, sizeof(upgrade)))
                    goto error_socket;
                if (strncmp(upgrade, "HTTP/1.1 101 ", 13) != 0)
                    goto error_socket;

                /* read headers */
                len = 0;
                do {
                    if (!sock_read_line(socket, upgrade, sizeof(upgrade)))
                        goto error_socket;
                    if (upgrade[0] == 0)
                        break;
                    if (!strncasecmp(upgrade, "Content-Length: ", 16) == 0)
                        len = atoi(upgrade + 16);
                } while (1);

                /* read body */
                while (len) {
                    rv = sock_read_bytes(socket, upgrade, len > sizeof(upgrade) ? sizeof(upgrade) : len);
                    if (rv < 1)
                        goto error_socket;
                    len -= rv;
                }
            } while (0);
        /* fall thru */

        case SHOUT_TLS_RFC2818:
            /* Use TLS for transport layer like HTTPS [RFC2818] does. */
            tls = shout_tls_new(self, socket);
            if (!tls)
                goto error_malloc;
            error = shout_tls_try_connect(tls);
            if (error != SHOUTERR_SUCCESS)
                goto error;
        break;

        default:
            /* Bad mode or auto detection not completed. */
            error = SHOUTERR_INSANE;
            goto error;
        break;
    }
#endif

#ifdef HAVE_OPENSSL
    if (tls) {
        rv = shout_tls_write(tls, request, strlen(request));
    } else {
        rv = sock_write(socket, "%s", request);
    }
#else
    rv = sock_write(socket, "%s", request);
#endif

    if (!rv)
        goto error_socket;

    error = SHOUTERR_SUCCESS;
    goto error;

error_socket:
    error = SHOUTERR_SOCKET;
    goto error;
error_malloc:
    error = SHOUTERR_MALLOC;
    goto error;
error:
#ifdef HAVE_OPENSSL
    if (tls)
        shout_tls_close(tls);
#endif
    if (socket != -1)
        sock_close(socket);
    if (encvalue)
        free(encvalue);
    if (request)
        free(request);
    if (auth)
        free(auth);
    if (mount)
        free(mount);
    return error;
}

/* getters/setters */
const char *shout_version(int *major, int *minor, int *patch)
{
    if (major)
        *major = LIBSHOUT_MAJOR;
    if (minor)
        *minor = LIBSHOUT_MINOR;
    if (patch)
        *patch = LIBSHOUT_MICRO;

    return VERSION;
}

int shout_get_errno(shout_t *self)
{
    return self->error;
}

const char *shout_get_error(shout_t *self)
{
    if (!self)
        return "Invalid shout_t";

    switch (self->error) {
    case SHOUTERR_SUCCESS:
        return "No error";
    case SHOUTERR_INSANE:
        return "Nonsensical arguments";
    case SHOUTERR_NOCONNECT:
        return "Couldn't connect";
    case SHOUTERR_NOLOGIN:
        return "Login failed";
    case SHOUTERR_SOCKET:
        return "Socket error";
    case SHOUTERR_MALLOC:
        return "Out of memory";
    case SHOUTERR_CONNECTED:
        return "Cannot set parameter while connected";
    case SHOUTERR_UNCONNECTED:
        return "Not connected";
    case SHOUTERR_BUSY:
        return "Socket is busy";
    case SHOUTERR_UNSUPPORTED:
        return "This libshout doesn't support the requested option";
    case SHOUTERR_NOTLS:
        return "TLS requested but not supported by peer";
    case SHOUTERR_TLSBADCERT:
        return "TLS connection can not be established because of bad certificate";
    case SHOUTERR_RETRY:
        return "Please retry current operation.";
    default:
        return "Unknown error";
    }
}

/* Returns:
 *   SHOUTERR_CONNECTED if the connection is open,
 *   SHOUTERR_UNCONNECTED if it has not yet been opened,
 *   or an error from try_connect, including SHOUTERR_BUSY
 */
int shout_get_connected(shout_t *self)
{
    int rc;

    if (!self)
        return SHOUTERR_INSANE;

    if (self->state == SHOUT_STATE_CONNECTED)
        return SHOUTERR_CONNECTED;
    if (self->state != SHOUT_STATE_UNCONNECTED) {
        if ((rc = try_connect(self)) == SHOUTERR_SUCCESS)
            return SHOUTERR_CONNECTED;
        return rc;
    }

    return SHOUTERR_UNCONNECTED;
}

int shout_set_host(shout_t *self, const char *host)
{
    if (!self)
        return SHOUTERR_INSANE;

    if (self->state != SHOUT_STATE_UNCONNECTED)
        return self->error = SHOUTERR_CONNECTED;

    if (self->host)
        free(self->host);

    if ( !(self->host = _shout_util_strdup(host)) )
        return self->error = SHOUTERR_MALLOC;

    return self->error = SHOUTERR_SUCCESS;
}

const char *shout_get_host(shout_t *self)
{
    if (!self)
        return NULL;

    return self->host;
}

int shout_set_port(shout_t *self, unsigned short port)
{
    if (!self)
        return SHOUTERR_INSANE;

    if (self->state != SHOUT_STATE_UNCONNECTED)
        return self->error = SHOUTERR_CONNECTED;

    self->port = port;

    return self->error = SHOUTERR_SUCCESS;
}

unsigned short shout_get_port(shout_t *self)
{
    if (!self)
        return 0;

    return self->port;
}

int shout_set_password(shout_t *self, const char *password)
{
    if (!self)
        return SHOUTERR_INSANE;

    if (self->state != SHOUT_STATE_UNCONNECTED)
        return self->error = SHOUTERR_CONNECTED;

    if (self->password)
        free(self->password);

    if ( !(self->password = _shout_util_strdup(password)) )
        return self->error = SHOUTERR_MALLOC;

    return self->error = SHOUTERR_SUCCESS;
}

const char* shout_get_password(shout_t *self)
{
    if (!self)
        return NULL;

    return self->password;
}

int shout_set_mount(shout_t *self, const char *mount)
{
    size_t len;

    if (!self || !mount)
        return SHOUTERR_INSANE;

    if (self->state != SHOUT_STATE_UNCONNECTED)
        return self->error = SHOUTERR_CONNECTED;

    if (self->mount)
        free(self->mount);

    len = strlen(mount) + 1;
    if (mount[0] != '/')
        len++;

    if ( !(self->mount = malloc(len)) )
        return self->error = SHOUTERR_MALLOC;

    snprintf(self->mount, len, "%s%s", mount[0] == '/' ? "" : "/", mount);

    return self->error = SHOUTERR_SUCCESS;
}

const char *shout_get_mount(shout_t *self)
{
    if (!self)
        return NULL;

    return self->mount;
}

int shout_set_name(shout_t *self, const char *name)
{
    return shout_set_meta(self, "name", name);
}

const char *shout_get_name(shout_t *self)
{
    return shout_get_meta(self, "name");
}

int shout_set_url(shout_t *self, const char *url)
{
    return shout_set_meta(self, "url", url);
}

const char *shout_get_url(shout_t *self)
{
    return shout_get_meta(self, "url");
}

int shout_set_genre(shout_t *self, const char *genre)
{
    return shout_set_meta(self, "genre", genre);
}

const char *shout_get_genre(shout_t *self)
{
    return shout_get_meta(self, "genre");
}

int shout_set_agent(shout_t *self, const char *agent)
{
    if (!self)
        return SHOUTERR_INSANE;

    if (self->state != SHOUT_STATE_UNCONNECTED)
        return self->error = SHOUTERR_CONNECTED;

    if (self->useragent)
        free(self->useragent);

    if ( !(self->useragent = _shout_util_strdup(agent)) )
        return self->error = SHOUTERR_MALLOC;

    return self->error = SHOUTERR_SUCCESS;
}

const char *shout_get_agent(shout_t *self)
{
    if (!self)
        return NULL;

    return self->useragent;
}


int shout_set_user(shout_t *self, const char *username)
{
    if (!self)
        return SHOUTERR_INSANE;

    if (self->state != SHOUT_STATE_UNCONNECTED)
        return self->error = SHOUTERR_CONNECTED;

    if (self->user)
        free(self->user);

    if ( !(self->user = _shout_util_strdup(username)) )
        return self->error = SHOUTERR_MALLOC;

    return self->error = SHOUTERR_SUCCESS;
}

const char *shout_get_user(shout_t *self)
{
    if (!self)
        return NULL;

    return self->user;
}

int shout_set_description(shout_t *self, const char *description)
{
    return shout_set_meta(self, "description", description);
}

const char *shout_get_description(shout_t *self)
{
    return shout_get_meta(self, "description");
}

int shout_set_dumpfile(shout_t *self, const char *dumpfile)
{
    if (!self)
        return SHOUTERR_INSANE;

    if (self->state != SHOUT_STATE_UNCONNECTED)
        return SHOUTERR_CONNECTED;

    if (self->dumpfile)
        free(self->dumpfile);

    if ( !(self->dumpfile = _shout_util_strdup(dumpfile)) )
        return self->error = SHOUTERR_MALLOC;

    return self->error = SHOUTERR_SUCCESS;
}

const char *shout_get_dumpfile(shout_t *self)
{
    if (!self)
        return NULL;

    return self->dumpfile;
}

int shout_set_audio_info(shout_t *self, const char *name, const char *value)
{
    if (!self)
        return SHOUTERR_INSANE;

    return self->error = _shout_util_dict_set(self->audio_info, name, value);
}

const char *shout_get_audio_info(shout_t *self, const char *name)
{
    if (!self)
        return NULL;

    return _shout_util_dict_get(self->audio_info, name);
}

int shout_set_meta(shout_t *self, const char *name, const char *value)
{
    size_t i;

    if (!self || !name)
        return SHOUTERR_INSANE;

    if (self->state != SHOUT_STATE_UNCONNECTED)
        return self->error = SHOUTERR_CONNECTED;

    for (i = 0; name[i]; i++)
        if ((name[i] < 'a' || name[i] > 'z') && (name[i] < '0' || name[i] > '9'))
            return self->error = SHOUTERR_INSANE;

    return self->error = _shout_util_dict_set(self->meta, name, value);
}

const char *shout_get_meta(shout_t *self, const char *name)
{
    if (!self)
        return NULL;

    return _shout_util_dict_get(self->meta, name);
}

int shout_set_public(shout_t *self, unsigned int public)
{
    if (!self || (public != 0 && public != 1))
        return SHOUTERR_INSANE;

    if (self->state != SHOUT_STATE_UNCONNECTED)
        return self->error = SHOUTERR_CONNECTED;

    self->public = public;

    return self->error = SHOUTERR_SUCCESS;
}

unsigned int shout_get_public(shout_t *self)
{
    if (!self)
        return 0;

    return self->public;
}

int shout_set_format(shout_t *self, unsigned int format)
{
    if (!self)
        return SHOUTERR_INSANE;

    if (self->state != SHOUT_STATE_UNCONNECTED)
        return self->error = SHOUTERR_CONNECTED;

    if (format != SHOUT_FORMAT_OGG && format != SHOUT_FORMAT_MP3 &&
        format != SHOUT_FORMAT_WEBM && format != SHOUT_FORMAT_WEBMAUDIO &&
        format != SHOUT_FORMAT_AAC && format != SHOUT_FORMAT_AACPLUS) {
        return self->error = SHOUTERR_UNSUPPORTED;
    }

    self->format = format;

    return self->error = SHOUTERR_SUCCESS;
}

unsigned int shout_get_format(shout_t* self)
{
    if (!self)
        return 0;

    return self->format;
}

int shout_set_mimetype(shout_t *self, const char *mimetype)
{
    if (!self)
        return self->error = SHOUTERR_INSANE;

    if (self->state != SHOUT_STATE_UNCONNECTED)
		return self->error = SHOUTERR_CONNECTED;

    if (self->mimetype)
        free(self->mimetype);
        
    if (!mimetype)
        self->mimetype = NULL;
    else
        if (! (self->mimetype = _shout_util_strdup (mimetype)))
            return self->error = SHOUTERR_MALLOC;
        
    return self->error = SHOUTERR_SUCCESS;
}

const char *shout_get_mimetype(shout_t *self)
{        
    const char *mimetype;
    
    if (!self)
        return NULL;
        
    if (self->mimetype)
        return self->mimetype;
        
    switch (self->format) {
    case SHOUT_FORMAT_OGG:
        mimetype = "application/ogg";
        break;
    case SHOUT_FORMAT_MP3:
        mimetype = "audio/mpeg";
        break;
    case SHOUT_FORMAT_WEBM:
        mimetype = "video/webm";
        break;
    case SHOUT_FORMAT_WEBMAUDIO:
        mimetype = "audio/webm";
        break;
    case SHOUT_FORMAT_AAC:
        mimetype = "audio/aac";
        break;
    case SHOUT_FORMAT_AACPLUS:
        mimetype = "audio/aacp";
        break;
    default:
    mimetype = NULL;
    }
    return mimetype;
}

int shout_set_protocol(shout_t *self, unsigned int protocol)
{
    if (!self)
        return SHOUTERR_INSANE;

    if (self->state != SHOUT_STATE_UNCONNECTED)
        return self->error = SHOUTERR_CONNECTED;

    if (protocol != SHOUT_PROTOCOL_HTTP &&
        protocol != SHOUT_PROTOCOL_XAUDIOCAST &&
        protocol != SHOUT_PROTOCOL_ICY &&
        protocol != SHOUT_PROTOCOL_ROARAUDIO) {
        return self->error = SHOUTERR_UNSUPPORTED;
    }

    self->protocol = protocol;

    return self->error = SHOUTERR_SUCCESS;
}

unsigned int shout_get_protocol(shout_t *self)
{
    if (!self)
        return 0;

    return self->protocol;
}

int shout_set_nonblocking(shout_t *self, unsigned int nonblocking)
{
    if (!self || (nonblocking != 0 && nonblocking != 1))
        return SHOUTERR_INSANE;

    if (self->state != SHOUT_STATE_UNCONNECTED)
        return self->error = SHOUTERR_CONNECTED;

    self->nonblocking = nonblocking;

    return SHOUTERR_SUCCESS;
}

unsigned int shout_get_nonblocking(shout_t *self)
{
    if (!self)
        return 0;

    return self->nonblocking;
}

/* TLS functions */
#ifdef HAVE_OPENSSL
int shout_set_tls(shout_t *self, int mode)
{
    if (!self)
        return SHOUTERR_INSANE;

    if (mode != SHOUT_TLS_DISABLED &&
        mode != SHOUT_TLS_AUTO &&
        mode != SHOUT_TLS_AUTO_NO_PLAIN &&
        mode != SHOUT_TLS_RFC2818 &&
        mode != SHOUT_TLS_RFC2817)
        return self->error = SHOUTERR_UNSUPPORTED;

    self->tls_mode = mode;
    return SHOUTERR_SUCCESS;
}
int shout_get_tls(shout_t *self)
{
    if (!self)
        return SHOUTERR_INSANE;

    return self->tls_mode;
}

int shout_set_ca_directory(shout_t *self, const char *directory)
{
    if (!self)
        return SHOUTERR_INSANE;

    if (self->state != SHOUT_STATE_UNCONNECTED)
        return self->error = SHOUTERR_CONNECTED;

    if (self->ca_directory)
        free(self->ca_directory);

    if (!(self->ca_directory = _shout_util_strdup(directory)))
        return self->error = SHOUTERR_MALLOC;

    return self->error = SHOUTERR_SUCCESS;
}

const char *shout_get_ca_directory(shout_t *self)
{
    if (!self)
        return NULL;

    return self->ca_directory;
}

int shout_set_ca_file(shout_t *self, const char *file)
{
    if (!self)
        return SHOUTERR_INSANE;

    if (self->state != SHOUT_STATE_UNCONNECTED)
        return self->error = SHOUTERR_CONNECTED;

    if (self->ca_file)
        free(self->ca_file);

    if (!(self->ca_file = _shout_util_strdup(file)))
        return self->error = SHOUTERR_MALLOC;

    return self->error = SHOUTERR_SUCCESS;
}

const char *shout_get_ca_file(shout_t *self)
{
    if (!self)
        return NULL;

    return self->ca_file;
}

int shout_set_allowed_ciphers(shout_t *self, const char *ciphers)
{
    if (!self)
        return SHOUTERR_INSANE;

    if (self->state != SHOUT_STATE_UNCONNECTED)
        return self->error = SHOUTERR_CONNECTED;

    if (self->allowed_ciphers)
        free(self->allowed_ciphers);

    if (!(self->allowed_ciphers = _shout_util_strdup(ciphers)))
        return self->error = SHOUTERR_MALLOC;

    return self->error = SHOUTERR_SUCCESS;
}

const char *shout_get_allowed_ciphers(shout_t *self)
{
    if (!self)
        return NULL;

    return self->allowed_ciphers;
}

int shout_set_client_certificate(shout_t *self, const char *certificate)
{
    if (!self)
        return SHOUTERR_INSANE;

    if (self->state != SHOUT_STATE_UNCONNECTED)
        return self->error = SHOUTERR_CONNECTED;

    if (self->client_certificate)
        free(self->client_certificate);

    if (!(self->client_certificate = _shout_util_strdup(certificate)))
        return self->error = SHOUTERR_MALLOC;

    return self->error = SHOUTERR_SUCCESS;
}

const char *shout_get_client_certificate(shout_t *self)
{
    if (!self)
        return NULL;

    return self->client_certificate;
}
#else
int shout_set_tls(shout_t *self, int mode)
{
    if (!self)
        return SHOUTERR_INSANE;

    if (mode == SHOUT_TLS_DISABLED)
        return SHOUTERR_SUCCESS;

    return self->error = SHOUTERR_UNSUPPORTED;
}

int shout_get_tls(shout_t *self)
{
    return SHOUT_TLS_DISABLED;
}

int shout_set_ca_directory(shout_t *self, const char *directory)
{
    if (!self)
        return SHOUTERR_INSANE;

    return self->error = SHOUTERR_UNSUPPORTED;
}

const char *shout_get_ca_directory(shout_t *self)
{
    return NULL;
}

int shout_set_ca_file(shout_t *self, const char *file)
{
    if (!self)
        return SHOUTERR_INSANE;

    return self->error = SHOUTERR_UNSUPPORTED;
}

const char *shout_get_ca_file(shout_t *self)
{
    return NULL;
}

int shout_set_allowed_ciphers(shout_t *self, const char *ciphers)
{
    if (!self)
        return SHOUTERR_INSANE;

    return self->error = SHOUTERR_UNSUPPORTED;
}
const char *shout_get_allowed_ciphers(shout_t *self)
{
    return NULL;
}

int shout_set_client_certificate(shout_t *self, const char *certificate)
{
    if (!self)
        return SHOUTERR_INSANE;
    return self->error = SHOUTERR_UNSUPPORTED;
}

const char *shout_get_client_certificate(shout_t *self)
{
    return NULL;
}
#endif

/* -- static function definitions -- */
static int get_response(shout_t *self)
{
    char buf[1024];
    int rc;

    rc = shout_conn_read(self, buf, sizeof(buf));

    if (rc < 0 && shout_conn_recoverable(self))
        return SHOUTERR_BUSY;

    if (rc <= 0)
        return SHOUTERR_SOCKET;

    if ((rc = shout_queue_data(&self->rqueue, (unsigned char*)buf, rc)))
        return rc;

    if (self->protocol == SHOUT_PROTOCOL_ROARAUDIO)
        return shout_get_roaraudio_response(self);

    return shout_get_http_response(self);
}

static int try_connect(shout_t *self)
{
    int rc;
    int port;

retry:
    /* the breaks between cases are omitted intentionally */
    switch (self->state) {
        case SHOUT_STATE_UNCONNECTED:
            port = self->port;
            if (shout_get_protocol(self) == SHOUT_PROTOCOL_ICY)
                port++;

            if (shout_get_nonblocking(self)) {
                if ((self->socket = sock_connect_non_blocking(self->host, port)) < 0)
                    return self->error = SHOUTERR_NOCONNECT;
                self->state = SHOUT_STATE_CONNECT_PENDING;
            } else {
                if ((self->socket = sock_connect(self->host, port)) < 0)
                    return self->error = SHOUTERR_NOCONNECT;
                self->state = SHOUT_STATE_CONNECT_PENDING;
            }
        /* fall through */

        case SHOUT_STATE_CONNECT_PENDING:
            if (shout_get_nonblocking(self)) {
                if ((rc = sock_connected(self->socket, 0)) < 1) {
                    if (rc == SOCK_ERROR) {
                        rc = SHOUTERR_SOCKET;
                        goto failure;
                    } else {
                        return SHOUTERR_BUSY;
                    }
                }
            }
            self->state = SHOUT_STATE_TLS_PENDING;
        /* fall through */

        case SHOUT_STATE_TLS_PENDING:
#ifdef HAVE_OPENSSL
            if (self->tls_mode_used < 0) {
                switch (self->tls_mode) {
                    case SHOUT_TLS_DISABLED:
                    case SHOUT_TLS_RFC2818:
                    case SHOUT_TLS_RFC2817:
                        self->tls_mode_used = self->tls_mode;
                    break;
                    case SHOUT_TLS_AUTO:
                    case SHOUT_TLS_AUTO_NO_PLAIN:
                        if (self->server_caps & LIBSHOUT_CAP_GOTCAPS) {
                            /* We had a probe allready, otherwise just poke the server. */
                            if (self->server_caps & LIBSHOUT_CAP_UPGRADETLS) {
                                self->tls_mode_used = SHOUT_TLS_RFC2817;
                            } else {
                                if (self->tls_mode == SHOUT_TLS_AUTO_NO_PLAIN) {
                                    self->tls_mode_used = SHOUTERR_NOTLS;
                                    return SHOUTERR_NOTLS;
                                }
                                self->tls_mode_used = SHOUT_TLS_DISABLED;
                            }
                            self->state = SHOUT_STATE_TLS_PENDING;
                            goto retry;
                        }
                    break;
                    default:
                        rc = SHOUTERR_INSANE;
                        goto failure;
                    break;
                }
            }

            if (self->tls_mode_used == SHOUT_TLS_RFC2818 || self->upgrade_to_tls) {
                if (!self->tls) {
                    self->tls = shout_tls_new(self, self->socket);
                    if (!self->tls) /* just guessing that it's a malloc error */
                        return SHOUTERR_MALLOC;
                }
                if ((rc = shout_tls_try_connect(self->tls)) != SHOUTERR_SUCCESS) {
                    if (rc == SHOUTERR_BUSY)
                        return SHOUTERR_BUSY;
                    goto failure;
                }
            } else if (self->tls_mode_used == SHOUT_TLS_RFC2817) {
                if ((rc = shout_create_http_request_upgrade(self, "TLS/1.0")) != SHOUTERR_SUCCESS) {
                    if (rc == SHOUTERR_BUSY)
                        return SHOUTERR_BUSY;
                    goto failure;
                }
                self->state = SHOUT_STATE_REQ_PENDING;
                goto retry;
            }
#endif
            self->state = SHOUT_STATE_REQ_CREATION;

        case SHOUT_STATE_REQ_CREATION:
            if ((rc = create_request(self)) != SHOUTERR_SUCCESS)
                goto failure;
            self->state = SHOUT_STATE_REQ_PENDING;
        /* fall through */

        case SHOUT_STATE_REQ_PENDING:
            do {
                rc = send_queue(self);
            } while (!shout_get_nonblocking(self) && rc == SHOUTERR_BUSY);

            if (rc == SHOUTERR_BUSY)
                return rc;

            if (rc == SHOUTERR_SOCKET && self->retry) {
                self->state = SHOUT_STATE_RECONNECT;
                goto retry;
            }

            if (rc != SHOUTERR_SUCCESS)
                goto failure;
            self->state = SHOUT_STATE_RESP_PENDING;
        /* fall through */

        case SHOUT_STATE_RESP_PENDING:
            do {
                rc = get_response(self);
            } while (!shout_get_nonblocking(self) && rc == SHOUTERR_BUSY);

            if (rc == SHOUTERR_BUSY)
                return rc;

            if (rc == SHOUTERR_SOCKET && self->retry) {
                self->state = SHOUT_STATE_RECONNECT;
                goto retry;
#ifdef HAVE_OPENSSL
            } else if (rc == SHOUTERR_SOCKET && !(self->server_caps & LIBSHOUT_CAP_GOTCAPS) &&
                (self->tls_mode == SHOUT_TLS_AUTO || self->tls_mode == SHOUT_TLS_AUTO_NO_PLAIN)) {
                self->state = SHOUT_STATE_RECONNECT;
                self->tls_mode_used = SHOUT_TLS_RFC2818;
                goto retry;
#endif
            }

            if (rc != SHOUTERR_SUCCESS)
                goto failure;

            if ((rc = parse_response(self)) != SHOUTERR_SUCCESS) {
                if (rc == SHOUTERR_RETRY)
                    goto retry;

                if (self->retry) {
                    self->state = SHOUT_STATE_TLS_PENDING;
                    goto retry;
                }
                goto failure;
            }

            switch (self->format) {
                case SHOUT_FORMAT_OGG:
                    if ((rc = self->error = shout_open_ogg(self)) != SHOUTERR_SUCCESS)
                        goto failure;
                break;

                case SHOUT_FORMAT_MP3:
                    if ((rc = self->error = shout_open_mp3(self)) != SHOUTERR_SUCCESS)
                        goto failure;
                break;

                case SHOUT_FORMAT_WEBM:
                case SHOUT_FORMAT_WEBMAUDIO:
                    if ((rc = self->error = shout_open_webm(self)) != SHOUTERR_SUCCESS)
                        goto failure;
                break;
                
                case SHOUT_FORMAT_AAC:
                case SHOUT_FORMAT_AACPLUS:
                    if ((rc = self->error = shout_open_adts(self)) != SHOUTERR_SUCCESS)
                        goto failure;
                break;

                default:
                    rc = SHOUTERR_INSANE;
                    goto failure;
            }
        /* fall through */

        case SHOUT_STATE_CONNECTED:
            self->state = SHOUT_STATE_CONNECTED;
        break;
        /* special case, no fallthru to this */

        case SHOUT_STATE_RECONNECT:
            sock_close(self->socket);
            self->state = SHOUT_STATE_UNCONNECTED;
            goto retry;
        break;
    }

    return SHOUTERR_SUCCESS;

failure:
    shout_close(self);
    return rc;
}

static int try_write(shout_t *self, const void *data_p, size_t len)
{
    int             ret;
    size_t          pos = 0;
    unsigned char  *data = (unsigned char*)data_p;

    /* loop until whole buffer is written (unless it would block) */
    do {
        ret = shout_conn_write(self, data + pos, len - pos);
        if (ret > 0)
            pos += ret;
    } while (pos < len && ret >= 0);

    if (ret < 0) {
        if (shout_conn_recoverable(self)) {
            self->error = SHOUTERR_BUSY;
            return pos;
        }
        self->error = SHOUTERR_SOCKET;
        return ret;
    }
    return pos;
}

ssize_t shout_conn_read(shout_t *self, void *buf, size_t len)
{
#ifdef HAVE_OPENSSL
    if (self->tls)
        return shout_tls_read(self->tls, buf, len);
#endif
    return sock_read_bytes(self->socket, buf, len);
}

ssize_t shout_conn_write(shout_t *self, const void *buf, size_t len)
{
#ifdef HAVE_OPENSSL
    if (self->tls)
        return shout_tls_write(self->tls, buf, len);
#endif
    return sock_write_bytes(self->socket, buf, len);
}
int shout_conn_recoverable(shout_t *self)
{
#ifdef HAVE_OPENSSL
    if (self->tls)
        return shout_tls_recoverable(self->tls);
#endif
    return sock_recoverable(sock_error());
}

static int send_queue(shout_t *self)
{
    shout_buf_t *buf;
    int          ret;

    if (!self->wqueue.len)
        return SHOUTERR_SUCCESS;

    buf = self->wqueue.head;
    while (buf) {
        ret = try_write(self, buf->data + buf->pos, buf->len - buf->pos);
        if (ret < 0)
            return self->error;

        buf->pos += ret;
        self->wqueue.len -= ret;
        if (buf->pos == buf->len) {
            self->wqueue.head = buf->next;
            free(buf);
            buf = self->wqueue.head;
            if (buf)
                buf->prev = NULL;
        } else {
            /* incomplete write */
            return SHOUTERR_BUSY;
        }
    }
    return self->error = SHOUTERR_SUCCESS;
}

static int create_request(shout_t *self)
{
    if (self->protocol == SHOUT_PROTOCOL_HTTP) {
        return shout_create_http_request(self);
    } else if (self->protocol == SHOUT_PROTOCOL_XAUDIOCAST) {
        return shout_create_xaudiocast_request(self);
    } else if (self->protocol == SHOUT_PROTOCOL_ICY) {
        return shout_create_icy_request(self);
    } else if (self->protocol == SHOUT_PROTOCOL_ROARAUDIO) {
        return shout_create_roaraudio_request(self);
    }

    return self->error = SHOUTERR_UNSUPPORTED;
}

static int parse_response(shout_t *self)
{
    if (self->protocol == SHOUT_PROTOCOL_HTTP) {
        return shout_parse_http_response(self);
    } else if (self->protocol == SHOUT_PROTOCOL_XAUDIOCAST ||
        self->protocol == SHOUT_PROTOCOL_ICY) {
        return shout_parse_xaudiocast_response(self);
    } else if (self->protocol == SHOUT_PROTOCOL_ROARAUDIO) {
        return shout_parse_roaraudio_response(self);
    }

    return self->error = SHOUTERR_UNSUPPORTED;
}
