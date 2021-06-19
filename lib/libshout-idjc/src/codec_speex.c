/* -*- c-basic-offset: 8; -*- */
/* speex.c: Ogg Speex data handlers for libshout
 *
 *  Copyright (C) 2005 the Icecast team <team@icecast.org>
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

#include <speex/speex.h>
#include <speex/speex_header.h>

#include "shout_private.h"
#include "format_ogg.h"

/* -- local data structures -- */
typedef struct {
    SpeexHeader *sh;
} speex_data_t;

/* -- local prototypes -- */
static int  read_speex_page(ogg_codec_t *codec, ogg_page *page);
static void free_speex_data(void *codec_data);

/* -- speex functions -- */
int _shout_open_speex(ogg_codec_t *codec, ogg_page *page)
{
    speex_data_t   *speex_data = calloc(1, sizeof(speex_data_t));
    ogg_packet      packet;

    (void)          page;

	if (!speex_data)
        return SHOUTERR_MALLOC;

    ogg_stream_packetout(&codec->os, &packet);

    if ( !(speex_data->sh = speex_packet_to_header((char*)packet.packet, packet.bytes)) ) {
        free_speex_data(speex_data);
        return SHOUTERR_UNSUPPORTED;
    }

    codec->codec_data   = speex_data;
    codec->read_page    = read_speex_page;
    codec->free_data    = free_speex_data;

    return SHOUTERR_SUCCESS;
}

static int read_speex_page(ogg_codec_t *codec, ogg_page *page)
{
    ogg_packet      packet;
    speex_data_t   *speex_data = codec->codec_data;
    uint64_t        samples = 0;

    (void)          page;

    while (ogg_stream_packetout(&codec->os, &packet) > 0) {
        samples += speex_data->sh->frames_per_packet * speex_data->sh->frame_size;
    }

    codec->senttime += ((samples * 1000000) / speex_data->sh->rate);

    return SHOUTERR_SUCCESS;
}

static void free_speex_data(void *codec_data)
{
    speex_data_t *speex_data = (speex_data_t*)codec_data;

	if (speex_data->sh)
        free(speex_data->sh);

    free(speex_data);
}
