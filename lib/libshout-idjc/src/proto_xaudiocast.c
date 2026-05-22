/* -*- c-basic-offset: 8; -*- */
/* proto_xaudiocast.c: Implementation of protocol xaudiocast.
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

#include <stdlib.h>
#include <string.h>

#include <shoutidjc/shout.h>
#include "shout_private.h"

shout_connection_return_state_t shout_create_xaudiocast_request(shout_t *self, shout_connection_t *connection)
{
    const char  *bitrate;
    const char  *val;
    char        *mount = NULL;
    int          ret;

    bitrate = shout_get_audio_info(self, SHOUT_AI_BITRATE);
    if (!bitrate)
        bitrate = "0";

    ret = SHOUTERR_MALLOC;
    do {
        if (!(mount = _shout_util_url_encode_resource(self->mount)))
            break;
        if (shout_queue_printf(connection, "SOURCE %s %s\n", self->password, mount))
            break;
        if (shout_queue_printf(connection, "x-audiocast-name: %s\n", shout_get_meta(self, "name")))
            break;
        val = shout_get_meta(self, "url");
        if (shout_queue_printf(connection, "x-audiocast-url: %s\n", val ? val : "http://www.icecast.org/"))
            break;
        val = shout_get_meta(self, "genre");
        if (shout_queue_printf(connection, "x-audiocast-genre: %s\n", val ? val : "icecast"))
            break;
        if (shout_queue_printf(connection, "x-audiocast-bitrate: %s\n", bitrate))
            break;
        if (shout_queue_printf(connection, "x-audiocast-public: %i\n", self->public))
            break;
        val = shout_get_meta(self, "description");
        if (shout_queue_printf(connection, "x-audiocast-description: %s\n", val ? val : "Broadcasting with the icecast streaming media server!"))
            break;
        if (self->dumpfile && shout_queue_printf(connection, "x-audiocast-dumpfile: %s\n", self->dumpfile))
            break;
        if (shout_queue_str(connection, "\n"))
            break;

        ret = SHOUTERR_SUCCESS;
    } while (0);

    if (mount)
        free(mount);

    shout_connection_set_error(connection, ret);
    return ret == SHOUTERR_SUCCESS ? SHOUT_RS_DONE : SHOUT_RS_ERROR;
}

shout_connection_return_state_t shout_get_xaudiocast_response(shout_t *self, shout_connection_t *connection)
{
    shout_buf_t *queue = connection->rqueue.head;
    size_t i;

    if (!connection->rqueue.len)
        return SHOUT_RS_DONE;

    do {
        for (i = 0; i < queue->len; i++) {
            if (queue->data[i] == '\n') {
                /* got response */
                return SHOUT_RS_DONE;
            }
        }
    } while ((queue = queue->next));

    /* need more data */
    return SHOUT_RS_NOTNOW;
}

shout_connection_return_state_t shout_parse_xaudiocast_response(shout_t *self, shout_connection_t *connection)
{
    char *response = NULL;

    if (connection->rqueue.len) {
        if (shout_queue_collect(connection->rqueue.head, &response) <= 0) {
            shout_connection_set_error(connection, SHOUTERR_MALLOC);
            return SHOUT_RS_ERROR;
        }
    }
    shout_queue_free(&connection->rqueue);

    if (!response || !strstr(response, "OK")) {
        free(response);

        /* check to see if that is a response to a POKE. */
        if (!(connection->server_caps & LIBSHOUT_CAP_GOTCAPS)) {
            connection->server_caps |= LIBSHOUT_CAP_GOTCAPS;
            shout_connection_disconnect(connection);
            shout_connection_connect(connection, self);
            connection->current_message_state = SHOUT_MSGSTATE_CREATING0;
            connection->target_message_state = SHOUT_MSGSTATE_SENDING1;
            return SHOUT_RS_NOTNOW;
        } else {
            shout_connection_set_error(connection, SHOUTERR_NOLOGIN);
            return SHOUT_RS_ERROR;
        }
    }
    free(response);

    connection->server_caps |= LIBSHOUT_CAP_GOTCAPS;
    connection->current_message_state = SHOUT_MSGSTATE_SENDING1;
    connection->target_message_state = SHOUT_MSGSTATE_WAITING1;
    return SHOUT_RS_DONE;
}

static const shout_protocol_impl_t shout_xaudiocast_impl_real = {
    .msg_create = shout_create_xaudiocast_request,
    .msg_get = shout_get_xaudiocast_response,
    .msg_parse = shout_parse_xaudiocast_response
};
const shout_protocol_impl_t *shout_xaudiocast_impl = &shout_xaudiocast_impl_real;
