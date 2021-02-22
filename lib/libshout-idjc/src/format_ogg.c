/* -*- c-basic-offset: 8; -*- */
/* ogg.c: Generic ogg data handler
 * $Id$
 *
 *  Copyright (C) 2002-2004 the Icecast team <team@icecast.org>
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

#include <stdlib.h>
#include <string.h>

#ifdef HAVE_INTTYPES_H
#   include <inttypes.h>
#endif

#include <ogg/ogg.h>

#include <shoutidjc/shout.h>
#include "shout_private.h"
#include "format_ogg.h"

/* -- local datatypes -- */
typedef struct {
    ogg_sync_state  oy;
    ogg_codec_t    *codecs;
    char            bos;
} ogg_data_t;

/* -- static prototypes -- */
static int  send_ogg(shout_t *self, const unsigned char *data, size_t len);
static void close_ogg(shout_t *self);
static int  open_codec(ogg_codec_t *codec, ogg_page *page);
static void free_codec(ogg_codec_t *codec);
static void free_codecs(ogg_data_t *ogg_data);
static int  send_page(shout_t *self, ogg_page *page);

typedef int (*codec_open_t)(ogg_codec_t *codec, ogg_page *page);

static codec_open_t codecs[] = {
    _shout_open_vorbis,
#ifdef HAVE_THEORA
    _shout_open_theora,
#endif
    _shout_open_opus,
#ifdef HAVE_SPEEX
    _shout_open_speex,
#endif
    NULL
};

int shout_open_ogg(shout_t *self)
{
    ogg_data_t *ogg_data;

    if (!(ogg_data = (ogg_data_t *)calloc(1, sizeof(ogg_data_t)))) {
        return self->error = SHOUTERR_MALLOC;
    }
    self->format_data = ogg_data;

    ogg_sync_init(&ogg_data->oy);
    ogg_data->bos = 1;

    self->send  = send_ogg;
    self->close = close_ogg;

    return SHOUTERR_SUCCESS;
}

static int send_ogg(shout_t *self, const unsigned char *data, size_t len)
{
    ogg_data_t  *ogg_data = (ogg_data_t*)self->format_data;
    ogg_codec_t *codec;
    char        *buffer;
    ogg_page     page;

    buffer = ogg_sync_buffer(&ogg_data->oy, len);
    memcpy(buffer, data, len);
    ogg_sync_wrote(&ogg_data->oy, len);

    while (ogg_sync_pageout(&ogg_data->oy, &page) == 1) {
        if (ogg_page_bos(&page)) {
            if (!ogg_data->bos) {
                free_codecs(ogg_data);
                ogg_data->bos = 1;
            }

            codec = calloc(1, sizeof(ogg_codec_t));
            if (! codec) {
                return self->error = SHOUTERR_MALLOC;
            }

            if ((self->error = open_codec(codec, &page)) != SHOUTERR_SUCCESS) {
                return self->error;
            }

            codec->headers = 1;
            codec->senttime = self->senttime;
            codec->next = ogg_data->codecs;
            ogg_data->codecs = codec;
        } else {
            ogg_data->bos = 0;

            codec = ogg_data->codecs;
            while (codec) {
                if (ogg_page_serialno(&page) == codec->os.serialno) {
                    if (codec->read_page) {
                        ogg_stream_pagein(&codec->os, &page);
                        codec->read_page(codec, &page);

                        if (self->senttime < codec->senttime) {
                            self->senttime = codec->senttime;
                        }
                    }

                    break;
                }
                codec = codec->next;
            }
        }

        if ((self->error = send_page(self, &page)) != SHOUTERR_SUCCESS) {
            return self->error;
        }
    }

    return self->error = SHOUTERR_SUCCESS;
}

static void close_ogg(shout_t *self)
{
    ogg_data_t *ogg_data = (ogg_data_t*)self->format_data;
    free_codecs(ogg_data);
    ogg_sync_clear(&ogg_data->oy);
    free(ogg_data);
}

static int open_codec(ogg_codec_t *codec, ogg_page *page)
{
    codec_open_t    this_codec;
    int             i = 0;

    while ((this_codec = codecs[i])) {
        ogg_stream_init(&codec->os, ogg_page_serialno(page));
        ogg_stream_pagein(&codec->os, page);

        if (this_codec(codec, page) == SHOUTERR_SUCCESS) {
            return SHOUTERR_SUCCESS;
        }

        ogg_stream_clear(&codec->os);
        i++;
    }

    /* if no handler is found, we currently just fall back to untimed send_raw */
    return SHOUTERR_SUCCESS;
}

static void free_codecs(ogg_data_t *ogg_data)
{
    ogg_codec_t *codec, *next;

    if (ogg_data == NULL) {
        return;
    }

    codec = ogg_data->codecs;
    while (codec) {
        next = codec->next;
        free_codec(codec);
        codec = next;
    }
    ogg_data->codecs = NULL;
}

static void free_codec(ogg_codec_t *codec)
{
    if (codec->free_data) {
        codec->free_data(codec->codec_data);
    }
    ogg_stream_clear(&codec->os);
    free(codec);
}

static int send_page(shout_t *self, ogg_page *page)
{
    int ret;

    ret = shout_send_raw(self, page->header, page->header_len);
    if (ret != page->header_len) {
        return self->error = SHOUTERR_SOCKET;
    }

    ret = shout_send_raw(self, page->body, page->body_len);
    if (ret != page->body_len) {
        return self->error = SHOUTERR_SOCKET;
    }

    return SHOUTERR_SUCCESS;
}
