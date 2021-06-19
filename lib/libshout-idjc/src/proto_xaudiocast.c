/* -*- c-basic-offset: 8; -*- */
/* proto_xaudiocast.c: Implementation of protocol xaudiocast.
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

#include <stdlib.h>
#include <string.h>

#include <shoutidjc/shout.h>
#include "shout_private.h"

int shout_create_xaudiocast_request(shout_t *self)
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
        if (shout_queue_printf(self, "SOURCE %s %s\n", self->password, mount))
            break;
        if (shout_queue_printf(self, "x-audiocast-name: %s\n", shout_get_meta(self, "name")))
            break;
        val = shout_get_meta(self, "url");
        if (shout_queue_printf(self, "x-audiocast-url: %s\n", val ? val : "http://www.icecast.org/"))
            break;
        val = shout_get_meta(self, "genre");
        if (shout_queue_printf(self, "x-audiocast-genre: %s\n", val ? val : "icecast"))
            break;
        if (shout_queue_printf(self, "x-audiocast-bitrate: %s\n", bitrate))
            break;
        if (shout_queue_printf(self, "x-audiocast-public: %i\n", self->public))
            break;
        val = shout_get_meta(self, "description");
        if (shout_queue_printf(self, "x-audiocast-description: %s\n", val ? val : "Broadcasting with the icecast streaming media server!"))
            break;
        if (self->dumpfile && shout_queue_printf(self, "x-audiocast-dumpfile: %s\n", self->dumpfile))
            break;
        if (shout_queue_str(self, "\n"))
            break;

        ret = SHOUTERR_SUCCESS;
    } while (0);

    if (mount)
        free(mount);

    return ret;
}

int shout_parse_xaudiocast_response(shout_t *self)
{
    char *response;

    if (shout_queue_collect(self->rqueue.head, &response) <= 0)
        return SHOUTERR_MALLOC;
    shout_queue_free(&self->rqueue);

    if (!strstr(response, "OK")) {
        free(response);
        return SHOUTERR_NOLOGIN;
    }
    free(response);

    return SHOUTERR_SUCCESS;
}
