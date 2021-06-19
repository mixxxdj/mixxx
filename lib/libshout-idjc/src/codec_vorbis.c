/* -*- c-basic-offset: 8; -*- */
/* vorbis.c: Ogg Vorbis data handlers for libshout
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

#ifdef HAVE_INTTYPES_H
#   include <inttypes.h>
#endif
#include <stdlib.h>

#include <vorbis/codec.h>

#include "shout_private.h"
#include "format_ogg.h"

/* -- local data structures -- */
typedef struct {
    vorbis_info     vi;
    vorbis_comment  vc;
    int             prevW;
} vorbis_data_t;

/* -- local prototypes -- */
static int  read_vorbis_page(ogg_codec_t *codec, ogg_page *page);
static void free_vorbis_data(void *codec_data);
static int  vorbis_blocksize(vorbis_data_t *vd, ogg_packet *p);

/* -- vorbis functions -- */
int _shout_open_vorbis(ogg_codec_t *codec, ogg_page *page)
{
    vorbis_data_t *vorbis_data = calloc(1, sizeof(vorbis_data_t));
    ogg_packet packet;

    (void)page;

	if (!vorbis_data)
        return SHOUTERR_MALLOC;

    vorbis_info_init(&vorbis_data->vi);
    vorbis_comment_init(&vorbis_data->vc);

    ogg_stream_packetout(&codec->os, &packet);

    if (vorbis_synthesis_headerin(&vorbis_data->vi, &vorbis_data->vc, &packet) < 0) {
        free_vorbis_data(vorbis_data);
        return SHOUTERR_UNSUPPORTED;
    }

    codec->codec_data   = vorbis_data;
    codec->read_page    = read_vorbis_page;
    codec->free_data    = free_vorbis_data;

    return SHOUTERR_SUCCESS;
}

static int read_vorbis_page(ogg_codec_t *codec, ogg_page *page)
{
    ogg_packet      packet;
    vorbis_data_t  *vorbis_data = codec->codec_data;
    uint64_t        samples = 0;
    (void)          page;

    if (codec->headers < 3) {
        while (ogg_stream_packetout(&codec->os, &packet) > 0) {
			if (vorbis_synthesis_headerin(&vorbis_data->vi, &vorbis_data->vc, &packet) < 0)
                return SHOUTERR_INSANE;
            codec->headers++;
        }

        return SHOUTERR_SUCCESS;
    }

    while (ogg_stream_packetout(&codec->os, &packet) > 0) {
        samples += vorbis_blocksize(vorbis_data, &packet);
    }

    codec->senttime += ((samples * 1000000) / vorbis_data->vi.rate);

    return SHOUTERR_SUCCESS;
}

static void free_vorbis_data(void *codec_data)
{
    vorbis_data_t *vorbis_data = (vorbis_data_t*)codec_data;

    vorbis_info_clear(&vorbis_data->vi);
    vorbis_comment_clear(&vorbis_data->vc);
    free(vorbis_data);
}

static int vorbis_blocksize(vorbis_data_t *vd, ogg_packet *p)
{
    int this = vorbis_packet_blocksize(&vd->vi, p);
    int ret  = (this + vd->prevW) / 4;

    if (!vd->prevW) {
        vd->prevW = this;
        return 0;
    }

    vd->prevW = this;
    return ret;
}
