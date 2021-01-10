/* -*- c-basic-offset: 8; -*- */
/* shout.c: Implementation of public libshout interface shout.h
 *
 *  Copyright (C) 2002-2004 the Icecast team <team@icecast.org>,
 *  Copyright (C) 2012-2022 Philipp Schafft <lion@lion.leolix.org>
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
static int try_connect(shout_t *self);

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
#endif

    self->port      = LIBSHOUT_DEFAULT_PORT;
    self->format    = LIBSHOUT_DEFAULT_FORMAT;
    self->usage     = LIBSHOUT_DEFAULT_USAGE;
    self->protocol  = LIBSHOUT_DEFAULT_PROTOCOL;

    return self;
}

void shout_free(shout_t *self)
{
    if (!self)
        return;

    if (!self->connection)
        return;

    if (self->host)
        free(self->host);
    if (self->password)
        free(self->password);
    if (self->content_language)
        free(self->content_language);
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
    if (self->connection)
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

    if (!self->connection)
        return self->error = SHOUTERR_UNCONNECTED;

    if (self->connection && self->connection->current_message_state == SHOUT_MSGSTATE_SENDING1 && self->close) {
        self->close(self);
        self->format_data = NULL;
        self->send = NULL;
        self->close = NULL;
    }

    shout_connection_unref(self->connection);
    self->connection = NULL;
    self->starttime = 0;
    self->senttime = 0;

    return self->error = SHOUTERR_SUCCESS;
}

int shout_send(shout_t *self, const unsigned char *data, size_t len)
{
    if (!self)
        return SHOUTERR_INSANE;

    if (!self->connection || self->connection->current_message_state != SHOUT_MSGSTATE_SENDING1)
        return self->error = SHOUTERR_UNCONNECTED;

    if (self->starttime <= 0)
        self->starttime = timing_get_time();

    if (!len) {
        int ret = shout_connection_iter(self->connection, self);
        if (ret == SHOUTERR_RETRY)
            return SHOUTERR_BUSY;
        return ret;
    }

    return self->send(self, data, len);
}

ssize_t shout_send_raw(shout_t *self, const unsigned char *data, size_t len)
{
    ssize_t ret;

    if (!self)
        return SHOUTERR_INSANE;

    if (!self->connection || self->connection->current_message_state != SHOUT_MSGSTATE_SENDING1)
        return SHOUTERR_UNCONNECTED;

    ret = shout_connection_send(self->connection, self, data, len);
    if (ret < 0)
       shout_connection_transfer_error(self->connection, self);
    return ret;
}

ssize_t shout_queuelen(shout_t *self)
{
    if (!self)
        return -1;

    return shout_connection_get_sendq(self->connection, self);
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
        return "Resource is busy";
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

    if (self->connection && self->connection->current_message_state == SHOUT_MSGSTATE_SENDING1)
        return SHOUTERR_CONNECTED;
    if (self->connection && self->connection->current_message_state != SHOUT_MSGSTATE_SENDING1) {
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

    if (self->connection)
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

    if (self->connection)
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

    if (self->connection)
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

    if (self->connection)
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

int shout_set_agent(shout_t *self, const char *agent)
{
    if (!self)
        return SHOUTERR_INSANE;

    if (self->connection)
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

int shout_set_content_language(shout_t *self, const char *content_language)
{
    const char *p;

    if (!self)
        return SHOUTERR_INSANE;

    if (!content_language) {
        if (self->content_language)
            free(self->content_language);
        return self->error = SHOUTERR_SUCCESS;
    }

    // we do at least a simple check for the correct format. This will at least detect unsafe chars.
    for (p = content_language; *p; p++) {
        if (*p == ' ' || *p == ',')
            continue;
        if ((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') || (*p >= '0' && *p <= '9'))
            continue;
        if (*p == '-')
            continue;

        return self->error = SHOUTERR_INSANE;
    }


    if (self->content_language)
        free(self->content_language);

    if ( !(self->content_language = _shout_util_strdup(content_language)) )
        return self->error = SHOUTERR_MALLOC;

    return self->error = SHOUTERR_SUCCESS;
}

const char *shout_get_content_language(shout_t *self)
{
    if (!self)
        return NULL;
    return self->content_language;
}

int shout_set_user(shout_t *self, const char *username)
{
    if (!self)
        return SHOUTERR_INSANE;

    if (self->connection)
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
    char c;

    if (!self || !name)
        return SHOUTERR_INSANE;

    if (self->connection)
        return self->error = SHOUTERR_CONNECTED;

    for (i = 0; (c = name[i]); i++) {
        if ((c < 'a' || c > 'z') && (c < '0' || c > '9'))
            return self->error = SHOUTERR_INSANE;
    }

    for (i = 0; (c = value[i]); i++) {
        if (c == '\r' || c == '\n')
            return self->error = SHOUTERR_INSANE;
    }

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

    if (self->connection)
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

static inline unsigned int remove_bits(unsigned int value, unsigned int to_remove)
{
    value |= to_remove;
    value -= to_remove;

    return value;
}

static inline int is_audio(unsigned int usage)
{
    if (!(usage & SHOUT_USAGE_AUDIO))
        return 0;

    if (remove_bits(usage, SHOUT_USAGE_AUDIO|SHOUT_USAGE_SUBTITLE))
        return 0;

    return 1;
}

static inline int is_video(unsigned int usage)
{
    if (!(usage & SHOUT_USAGE_VISUAL))
        return 0;

    if (remove_bits(usage, SHOUT_USAGE_VISUAL|SHOUT_USAGE_AUDIO|SHOUT_USAGE_SUBTITLE|SHOUT_USAGE_3D|SHOUT_USAGE_4D))
        return 0;

    return 1;
}

static const char *shout_get_mimetype(unsigned int format, unsigned int usage, const char *codecs)
{
    if (codecs)
        return NULL;

    switch (format) {
        case SHOUT_FORMAT_OGG:
            if (is_audio(usage)) {
                return "audio/ogg";
            } else if (is_video(usage)) {
                return "video/ogg";
            } else {
                return "application/ogg";
            }
        break;

        case SHOUT_FORMAT_MP3:
            /* MP3 *ONLY* support Audio. So all other values are outright invalid */
            if (usage == SHOUT_USAGE_AUDIO) {
                return "audio/mpeg";
            }
        break;
        case SHOUT_FORMAT_WEBM:
            if (is_audio(usage)) {
                return "audio/webm";
            } else if (is_video(usage)) {
                return "video/webm";
            }
        break;
        case SHOUT_FORMAT_MATROSKA:
            if (is_audio(usage)) {
                return "audio/x-matroska";
            } else if (is_video(usage) && (usage & SHOUT_USAGE_3D)) {
                return "video/x-matroska-3d";
            } else if (is_video(usage)) {
                return "video/x-matroska";
            }
        break;
        case SHOUT_FORMAT_TEXT:
            if (usage == SHOUT_USAGE_TEXT) {
                return "text/plain; charset=utf-8";
            }
        break;
        case SHOUT_FORMAT_AAC:
            if (usage == SHOUT_USAGE_AUDIO) {
                return "audio/aac";
            }
        break;
        case SHOUT_FORMAT_AACPLUS:
            if (usage == SHOUT_USAGE_AUDIO) {
                return "audio/aacp";
            }
        break;
    }

    return NULL;
}

const char *shout_get_mimetype_from_self(shout_t *self)
{
    return shout_get_mimetype(self->format, self->usage, NULL);
}

int shout_set_content_format(shout_t *self, unsigned int format, unsigned int usage, const char *codecs)
{
    if (!self)
        return SHOUTERR_INSANE;

    if (self->connection)
        return self->error = SHOUTERR_CONNECTED;

    if (codecs) {
        return self->error = SHOUTERR_UNSUPPORTED;
    }

    if (!shout_get_mimetype(format, usage, codecs)) {
        return self->error = SHOUTERR_UNSUPPORTED;
    }

    self->format = format;
    self->usage  = usage;

    return self->error = SHOUTERR_SUCCESS;
}

int shout_get_content_format(shout_t *self, unsigned int *format, unsigned int *usage, const char **codecs)
{
    if (!self)
        return SHOUTERR_INSANE;

    if (format)
        *format = self->format;

    if (usage)
        *usage = self->usage;

    if (codecs)
        *codecs = NULL;

    return self->error = SHOUTERR_SUCCESS;
}

int shout_set_protocol(shout_t *self, unsigned int protocol)
{
    if (!self)
        return SHOUTERR_INSANE;

    if (self->connection)
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
    if (nonblocking == SHOUT_BLOCKING_DEFAULT)
        nonblocking = SHOUT_BLOCKING_FULL;

    if (!self || (nonblocking != SHOUT_BLOCKING_FULL && nonblocking != SHOUT_BLOCKING_NONE))
        return SHOUTERR_INSANE;

    if (self->connection)
        return self->error = SHOUTERR_CONNECTED;

    self->nonblocking = nonblocking;

    return SHOUTERR_SUCCESS;
}

unsigned int shout_get_nonblocking(shout_t *self)
{
    if (!self)
        return SHOUT_BLOCKING_FULL;

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

    if (self->connection)
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

    if (self->connection)
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

    if (self->connection)
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

    if (self->connection)
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

    if (mode != SHOUT_TLS_DISABLED &&
        mode != SHOUT_TLS_AUTO)
        return self->error = SHOUTERR_UNSUPPORTED;

    return SHOUTERR_SUCCESS;
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

int shout_control(shout_t *self, shout_control_t control, ...)
{
    int ret = SHOUTERR_INSANE;
    va_list ap;

    if (!self)
        return SHOUTERR_INSANE;

    va_start(ap, control);

    switch (control) {
#ifdef HAVE_OPENSSL
        case SHOUT_CONTROL_GET_SERVER_CERTIFICATE_AS_PEM:
        case SHOUT_CONTROL_GET_SERVER_CERTIFICATE_CHAIN_AS_PEM:
            if (self->connection->tls) {
                void **vpp = va_arg(ap, void **);
                if (vpp) {
                    ret = shout_connection_control(self->connection, control, vpp);
                } else {
                    ret = SHOUTERR_INSANE;
                }
            } else {
                ret = SHOUTERR_BUSY;
            }
        break;
#else
        case SHOUT_CONTROL_GET_SERVER_CERTIFICATE_AS_PEM:
        case SHOUT_CONTROL_GET_SERVER_CERTIFICATE_CHAIN_AS_PEM:
            ret = SHOUTERR_UNSUPPORTED;
        break;
#endif
        case SHOUT_CONTROL__MIN:
        case SHOUT_CONTROL__MAX:
            ret = SHOUTERR_INSANE;
        break;
    }

    va_end(ap);

    return self->error = ret;
}
int shout_set_callback(shout_t *self, shout_callback_t callback, void *userdata)
{
    if (!self)
        return SHOUTERR_INSANE;

    self->callback = callback;
    self->callback_userdata = userdata;

    return self->error = SHOUTERR_SUCCESS;
}

/* -- static function definitions -- */
static int shout_call_callback(shout_t *self, shout_event_t event, ...)
{
    va_list ap;
    int ret;

    if (!self->callback)
        return SHOUT_CALLBACK_PASS;

    va_start(ap, event);
    ret = self->callback(self, event, self->callback_userdata, ap);
    va_end(ap);

    return ret;
}

int shout_cb_connection_callback(shout_connection_t *con, shout_event_t event, void *userdata, va_list ap)
{
    shout_t *self = userdata;

    /* Avoid going up if not needed */
    if (!self->callback)
        return SHOUT_CALLBACK_PASS;

    switch (event) {
        case SHOUT_EVENT_TLS_CHECK_PEER_CERTIFICATE:
            return shout_call_callback(self, event, con);
        break;
        case SHOUT_EVENT__MIN:
        case SHOUT_EVENT__MAX:
            return SHOUTERR_INSANE;
        break;
    }

    return SHOUT_CALLBACK_PASS;
}

static int try_connect(shout_t *self)
{
    int ret;

    if (!self->connection) {
        const shout_protocol_impl_t *impl = NULL;

        switch (shout_get_protocol(self)) {
            case SHOUT_PROTOCOL_HTTP:
                impl = shout_http_impl;
                memset(&(self->source_plan.http), 0, sizeof(self->source_plan.http));
                self->source_plan.http.is_source = 1;
                self->source_plan.http.auth = 1;
                self->source_plan.http.resource = self->mount;
            break;
            case SHOUT_PROTOCOL_XAUDIOCAST:
                impl = shout_xaudiocast_impl;
            break;
            case SHOUT_PROTOCOL_ICY:
                impl = shout_icy_impl;
            break;
            case SHOUT_PROTOCOL_ROARAUDIO:
                impl = shout_roaraudio_impl;
            break;
        }

        self->connection = shout_connection_new(self, impl, &(self->source_plan));
        if (!self->connection)
            return self->error = SHOUTERR_MALLOC;

        shout_connection_set_callback(self->connection, shout_cb_connection_callback, self);

#ifdef HAVE_OPENSSL
        shout_connection_select_tlsmode(self->connection, self->tls_mode);
#endif
        self->connection->target_message_state = SHOUT_MSGSTATE_SENDING1;
        shout_connection_connect(self->connection, self);
    }

    ret = shout_connection_iter(self->connection, self);
    if (ret == SHOUTERR_RETRY)
        ret = SHOUTERR_BUSY;
    self->error = ret;

    if (self->connection->current_message_state == SHOUT_MSGSTATE_SENDING1 && !self->send) {
        int rc;
        switch (self->format) {
            case SHOUT_FORMAT_OGG:
                rc = self->error = shout_open_ogg(self);
                break;
            case SHOUT_FORMAT_MP3:
                rc = self->error = shout_open_mp3(self);
                break;
            case SHOUT_FORMAT_WEBM:
            case SHOUT_FORMAT_MATROSKA:
                rc = self->error = shout_open_webm(self);
                break;
            case SHOUT_FORMAT_TEXT:
                rc = self->error = shout_open_text(self);
                break;
            case SHOUT_FORMAT_AAC:
            case SHOUT_FORMAT_AACPLUS:
                rc = self->error = shout_open_adts(self);
                break;
            default:
                rc = SHOUTERR_INSANE;
                break;
        }
        if (rc != SHOUTERR_SUCCESS) {
            return ret;
        }
    }

    return ret;
}
