/* -*- c-basic-offset: 8; -*- */
/* shout.c: Implementation of public libshout interface shout.h
 *
 *  Copyright (C) 2021-2021 Philipp Schafft <lion@lion.leolix.org>
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
#   include <config.h>
#endif

#include <shoutidjc/shout.h>
#include "shout_private.h"

static void close_text(shout_t *self)
{
    /* no-op */
}

static int send_text(shout_t *self, const unsigned char *data, size_t len)
{
    ssize_t ret = shout_send_raw(self, data, len);

    if (ret != (ssize_t)len) {
        self->error = SHOUTERR_SOCKET;
    }

    return self->error;
}

int shout_open_text(shout_t *self)
{
    /* configure shout state */
    self->format_data = NULL;

    self->send = send_text;
    self->close = close_text;

    return SHOUTERR_SUCCESS;
}
