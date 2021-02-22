/* -*- c-basic-offset: 8; -*- */
/* opus.c: Ogg Opus data handlers for libshout
 *
 *  Copyright (C) 2005 the Icecast team <team@icecast.org>
 *  Copyright (C) 2011,2012 Xiph.Org Foundation
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
#include <inttypes.h>
#endif
#include <string.h>

#include <ogg/ogg.h>

#include "shout_private.h"
#include "format_ogg.h"

/* -- local data structures -- */
typedef struct {
    int      version;
    int      channels;          /* Number of channels: 1..255 */
    int      preskip;
    uint32_t input_sample_rate;
    int      gain;              /* in dB S7.8 should be zero whenever possible */
    int      channel_mapping;

    /* The rest is only used if channel_mapping != 0 */
    int      nb_streams;
    int      nb_coupled;
    unsigned char stream_map[255];
} OpusHeader;

typedef struct {
    OpusHeader  oh;
    int         skipped;
} opus_data_t;

typedef struct {
    const unsigned char *data;
    int                 maxlen;
    int                 pos;
} ROPacket;

/* -- local prototypes -- */
static int  read_opus_page(ogg_codec_t *codec, ogg_page *page);
static void free_opus_data(void *codec_data);
static int  opus_header_parse(const unsigned char *header, int len, OpusHeader *h);

/* -- header functions -- */

static int read_uint32(ROPacket *p, uint32_t *val)
{
    if (p->pos>p->maxlen-4)
        return 0;
    *val = (uint32_t)p->data[p->pos  ];
    *val |= (uint32_t)p->data[p->pos + 1] << 8;
    *val |= (uint32_t)p->data[p->pos + 2] << 16;
    *val |= (uint32_t)p->data[p->pos + 3] << 24;
    p->pos += 4;
    return 1;
}

static int read_uint16(ROPacket *p, uint16_t *val)
{
    if (p->pos>p->maxlen-2)
        return 0;
    *val = (uint16_t)p->data[p->pos  ];
    *val |= (uint16_t)p->data[p->pos + 1] << 8;
    p->pos += 2;
    return 1;
}

static int read_chars(ROPacket *p, unsigned char *str, int nb_chars)
{
    int i;
    if (p->pos>p->maxlen-nb_chars)
        return 0;
    for (i = 0; i < nb_chars; i++)
        str[i] = p->data[p->pos++];
    return 1;
}

static int opus_header_parse(const unsigned char *packet, int len, OpusHeader *h)
{
    int             i;
    char            str[9];
    ROPacket        p;
    unsigned char   ch;
    uint16_t        shortval;

    p.data      = packet;
    p.maxlen    = len;
    p.pos       = 0;

    str[8] = 0;

    if (len<19)
        return 0;

    read_chars(&p, (unsigned char*)str, 8);
    if (strcmp(str, "OpusHead")!=0)
        return 0;

    if (!read_chars(&p, &ch, 1))
        return 0;

    h->version = ch;
    if ((h->version&240) != 0) /* Only major version 0 supported. */
        return 0;

    if (!read_chars(&p, &ch, 1))
        return 0;

    h->channels = ch;
    if (h->channels == 0)
        return 0;

    if (!read_uint16(&p, &shortval))
        return 0;

    h->preskip = shortval;
    if (!read_uint32(&p, &h->input_sample_rate))
        return 0;

    if (!read_uint16(&p, &shortval))
        return 0;

    h->gain = (short)shortval;
    if (!read_chars(&p, &ch, 1))
        return 0;

    h->channel_mapping = ch;
    if (h->channel_mapping != 0) {
        if (!read_chars(&p, &ch, 1))
            return 0;

        h->nb_streams = ch;
        if (!read_chars(&p, &ch, 1))
            return 0;

        h->nb_coupled = ch;
        /* Multi-stream support */
        for (i=0; i < h->channels; i++) {
            if (!read_chars(&p, &h->stream_map[i], 1))
                return 0;
        }
    } else {
        h->nb_streams       = 1;
        h->nb_coupled       = h->channels > 1;
        h->stream_map[0]    = 0;
        h->stream_map[1]    = 1;
    }

    /* For version 0/1 we know there won't be any more data
     * so reject any that have data past the end.
     */
     if ((h->version==0 || h->version==1) && p.pos != len)
        return 0;
    return 1;
}

/* From libopus, src/opus_decode.c */
static int packet_get_samples_per_frame(const unsigned char *data, int32_t Fs)
{
    int audiosize;

    if (data[0] & 0x80) {
        audiosize = ((data[0] >> 3) & 0x3);
        audiosize = (Fs << audiosize) / 400;
    } else if ((data[0]&0x60) == 0x60) {
        audiosize = (data[0] & 0x08) ? Fs / 50 : Fs / 100;
    } else {
        audiosize = ((data[0] >> 3) & 0x3);
        if (audiosize == 3) {
            audiosize = Fs * 60 / 1000;
        } else {
            audiosize = (Fs << audiosize) / 100;
        }
    }
    return audiosize;
}

/* From libopus, src/opus_decode.c */
static int packet_get_nb_frames(const unsigned char packet[], int32_t len)
{
    int count;

    if (len < 1)
        return -1;

    count = packet[0] & 0x3;
    if (count==0) {
        return 1;
    } else if (count!=3) {
        return 2;
    } else if (len<2) {
        return -4;
    } else {
        return packet[1] & 0x3F;
    }
}

/* -- opus functions -- */
int _shout_open_opus(ogg_codec_t *codec, ogg_page *page)
{
    opus_data_t     *opus_data = calloc(1, sizeof(opus_data_t));
    ogg_packet      packet;

    (void)          page;

   if (!opus_data)
        return SHOUTERR_MALLOC;

    ogg_stream_packetout(&codec->os, &packet);

    if (!opus_header_parse(packet.packet, packet.bytes, &opus_data->oh)) {
        free_opus_data(opus_data);
        return SHOUTERR_UNSUPPORTED;
    }
    opus_data->skipped = 0;

    codec->codec_data   = opus_data;
    codec->read_page    = read_opus_page;
    codec->free_data    = free_opus_data;

    return SHOUTERR_SUCCESS;
}

static int read_opus_page(ogg_codec_t *codec, ogg_page *page)
{
    ogg_packet      packet;
    opus_data_t    *opus_data = codec->codec_data;

    (void)          page;

    /* We use the strategy of counting the packet times and ignoring
     * the granpos. This has the advantage of needing less code to
     * sanely handle non-zero starttimes and slightly saner behavior
     * on files with holes.
     */
    while (ogg_stream_packetout(&codec->os, &packet) > 0) {
        if (packet.bytes > 0 && (packet.bytes < 2 || memcmp(packet.packet, "Op", 2) != 0)) {
            int32_t spf;
            spf = packet_get_samples_per_frame(packet.packet, 48000);
            if (spf > 0) {
                int32_t spp;
                spp = packet_get_nb_frames(packet.packet, packet.bytes);
                if (spp > 0) {
                    int needskip;
                    needskip = opus_data->oh.preskip - opus_data->skipped;
                    spp *= spf;
                    /* Opus files can begin with some frames which are
                     * just there to prime the decoder and are not played
                     * these should just be sent as fast as we get them.
                     */
                    if (needskip > 0) {
                        int skip;
                        skip = spp < needskip ? spp : needskip;
                        spp -= skip;
                        opus_data->skipped += skip;
                    }
                    codec->senttime += ((spp * 1000000ULL) / 48000ULL);
                }
            } else if (packet.bytes >= 19 && memcmp(packet.packet, "OpusHead", 8) == 0) {
                /* We appear to be chaining, reset skip to burst the pregap. */
                if (opus_header_parse(packet.packet,packet.bytes,&opus_data->oh))
                    opus_data->skipped = 0;
            }
        }
    }
    return SHOUTERR_SUCCESS;
}

static void free_opus_data(void *codec_data)
{
    free(codec_data);
}
