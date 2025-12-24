/* -*- c-basic-offset: 8; -*- */
/* format_ogg.h: Internal shout interface to Ogg codec handlers
 * $Id$
 *
 *  Copyright (C) 2004 the Icecast team <team@icecast.org>
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

#ifndef __LIBSHOUT_FORMAT_OGG_H__
#define __LIBSHOUT_FORMAT_OGG_H__

#ifdef HAVE_CONFIG_H
#   include <config.h>
#endif

#include <stdlib.h>

#include <ogg/ogg.h>

typedef struct _ogg_codec_tag {
    ogg_stream_state os;

    unsigned int    headers;
    uint64_t        senttime;

    void    *codec_data;
    int     (*read_page)(struct _ogg_codec_tag *codec, ogg_page *page);
    void    (*free_data)(void *codec_data);

    struct  _ogg_codec_tag *next;
} ogg_codec_t;

/* codec hooks */
int _shout_open_vorbis(ogg_codec_t *codec, ogg_page *page);

#ifdef HAVE_THEORA
int _shout_open_theora(ogg_codec_t *codec, ogg_page *page);
#endif

#ifdef HAVE_SPEEX
int _shout_open_speex(ogg_codec_t *codec, ogg_page *page);
#endif

int _shout_open_opus(ogg_codec_t *codec, ogg_page *page);

#endif
