/* icy.c: Implementation of public libshout interface shout.h
 *        ICY related functions only.
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
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdbool.h>
#include <string.h>

#include <shoutidjc/shout.h>

#include "shout_private.h"
#include "util.h"

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

static int shout_set_metadata_impl(shout_t *self, shout_metadata_t *metadata, bool is_utf8)
{
    shout_connection_t *connection;
    shout_http_plan_t plan;
    size_t param_len;
    char *param = NULL;
    char *encvalue;
    char *encpassword;
    char *encmount;
    const char *param_template;
    int ret;

    if (!self || !metadata)
        return SHOUTERR_INSANE;

    encvalue = _shout_util_dict_urlencode(metadata, '&');
    if (!encvalue)
        return self->error = SHOUTERR_MALLOC;

    memset(&plan, 0, sizeof(plan));

    plan.is_source = 0;

    switch (self->protocol) {
        case SHOUT_PROTOCOL_ICY:
            if (!(encpassword = _shout_util_url_encode(self->password))) {
                free(encvalue);
                return self->error = SHOUTERR_MALLOC;
            }

            param_template = "mode=updinfo&pass=%s&%s";
            param_len = strlen(param_template) + strlen(encvalue) + 1 + strlen(encpassword);
            param = malloc(param_len);
            if (!param) {
                free(encpassword);
                free(encvalue);
                return self->error = SHOUTERR_MALLOC;
            }
            snprintf(param, param_len, param_template, encpassword, encvalue);
            free(encpassword);

            plan.param = param;
            plan.fake_ua = 1;
            plan.auth = 0;
            plan.method = "GET";
            plan.resource = "/admin.cgi";
        break;
        case SHOUT_PROTOCOL_HTTP:
            if (!(encmount = _shout_util_url_encode(self->mount))) {
                free(encvalue);
                return self->error = SHOUTERR_MALLOC;
            }

            if (is_utf8) {
                param_template = "mode=updinfo&charset=UTF-8&mount=%s&%s";
            } else {
                param_template = "mode=updinfo&mount=%s&%s";
            }
            param_len = strlen(param_template) + strlen(encvalue) + 1 + strlen(encmount);
            param = malloc(param_len);
            if (!param) {
                free(encmount);
                free(encvalue);
                return self->error = SHOUTERR_MALLOC;
            }
            snprintf(param, param_len, param_template, encmount, encvalue);
            free(encmount);

            plan.param = param;
            plan.auth = 1;
            plan.resource = "/admin/metadata";
        break;
        case SHOUT_PROTOCOL_XAUDIOCAST:
            if (!(encmount = _shout_util_url_encode(self->mount))) {
                free(encvalue);
                return self->error = SHOUTERR_MALLOC;
            }
            if (!(encpassword = _shout_util_url_encode(self->password))) {
                free(encmount);
                free(encvalue);
                return self->error = SHOUTERR_MALLOC;
            }

            param_template = "mode=updinfo&pass=%s&mount=%s&%s";
            param_len = strlen(param_template) + strlen(encvalue) + 1 + strlen(encpassword) + strlen(self->mount);
            param = malloc(param_len);
            if (!param) {
                free(encpassword);
                free(encmount);
                free(encvalue);
                return self->error = SHOUTERR_MALLOC;
            }
            snprintf(param, param_len, param_template, encpassword, encmount, encvalue);
            free(encpassword);
            free(encmount);

            plan.param = param;
            plan.auth = 0;
            plan.method = "GET";
            plan.resource = "/admin.cgi";
        break;
        default:
            free(encvalue);
            return self->error = SHOUTERR_UNSUPPORTED;
        break;
    }

    free(encvalue);

    connection = shout_connection_new(self, shout_http_impl, &plan);
    if (!connection) {
        free(param);
        return self->error = SHOUTERR_MALLOC;
    }

    shout_connection_set_callback(self->connection, shout_cb_connection_callback, self);

#ifdef HAVE_OPENSSL
    shout_connection_select_tlsmode(connection, self->tls_mode);
#endif
    shout_connection_set_nonblocking(connection, SHOUT_BLOCKING_FULL);

    connection->target_message_state = SHOUT_MSGSTATE_PARSED_FINAL;

    shout_connection_connect(connection, self);

    do {
        ret = shout_connection_iter(connection, self);
    } while (ret == SHOUTERR_RETRY || ret == SHOUTERR_BUSY);

    shout_connection_unref(connection);

    free(param);

    return ret;
}

int shout_set_metadata(shout_t *self, shout_metadata_t *metadata)
{
    return shout_set_metadata_impl(self, metadata, false);
}

int shout_set_metadata_utf8(shout_t *self, shout_metadata_t *metadata)
{
    return shout_set_metadata_impl(self, metadata, true);
}
