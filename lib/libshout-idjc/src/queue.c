/* -*- c-basic-offset: 8; -*- */
/* queue.c: Implementation data queue logic.
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
#include <stdio.h>

#include <shoutidjc/shout.h>
#include "shout_private.h"

/* queue data in pages of SHOUT_BUFSIZE bytes */
int shout_queue_data(shout_queue_t *queue, const unsigned char *data, size_t len)
{
    shout_buf_t *buf;
    size_t       plen;

	if (!len)
        return SHOUTERR_SUCCESS;

    if (!queue->len) {
        queue->head = calloc(1, sizeof(shout_buf_t));
		if (!queue->head)
            return SHOUTERR_MALLOC;
    }

    for (buf = queue->head; buf->next; buf = buf->next) ;

    /* Maybe any added data should be freed if we hit a malloc error?
     * Otherwise it'd be impossible to tell where to start requeueing.
     * (As if anyone ever tried to recover from a malloc error.) */
    while (len > 0) {
        if (buf->len == SHOUT_BUFSIZE) {
            buf->next = calloc(1, sizeof(shout_buf_t));
			if (!buf->next)
                return SHOUTERR_MALLOC;
            buf->next->prev = buf;
            buf = buf->next;
        }

        plen = len > SHOUT_BUFSIZE - buf->len ? SHOUT_BUFSIZE - buf->len : len;
        memcpy(buf->data + buf->len, data, plen);
        buf->len += plen;
        data += plen;
        len -= plen;
        queue->len += plen;
    }
    return SHOUTERR_SUCCESS;
}

int shout_queue_str(shout_t *self, const char *str)
{
    return shout_queue_data(&self->wqueue, (const unsigned char*)str, strlen(str));
}

/* this should be shared with sock_write. Create libicecommon. */
int shout_queue_printf(shout_t *self, const char *fmt, ...)
{
    char        buffer[1024];
    char       *buf;
    va_list     ap, ap_retry;
    int         len;

    buf = buffer;

    va_start(ap, fmt);
    va_copy(ap_retry, ap);

    len = vsnprintf(buf, sizeof(buffer), fmt, ap);

    self->error = SHOUTERR_SUCCESS;
    if (len > 0) {
		if ((size_t)len < sizeof(buffer)) {
            shout_queue_data(&self->wqueue, (unsigned char*)buf, len);
        } else {
            buf = malloc(++len);
            if (buf) {
                len = vsnprintf(buf, len, fmt, ap_retry);
                shout_queue_data(&self->wqueue, (unsigned char*)buf, len);
                free(buf);
			} else {
                self->error = SHOUTERR_MALLOC;
            }
        }
    }

    va_end(ap_retry);
    va_end(ap);

    return self->error;
}

void shout_queue_free(shout_queue_t *queue)
{
    shout_buf_t *prev;

    while (queue->head) {
        prev = queue->head;
        queue->head = queue->head->next;
        free(prev);
    }
    queue->len = 0;
}

/* collect nodes of a queue into a single buffer */
ssize_t shout_queue_collect(shout_buf_t *queue, char **buf)
{
    shout_buf_t *node;
    size_t       pos = 0;
    size_t       len = 0;

    for (node = queue; node; node = node->next)
        len += node->len;

    if (!(*buf = malloc(len)))
        return SHOUTERR_MALLOC;

    for (node = queue; node; node = node->next) {
        memcpy(*buf + pos, node->data, node->len);
        pos += node->len;
    }

    return len;
}
