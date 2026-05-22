/* -*- c-basic-offset: 8; -*- */
/* mpeg.c: mpeg format handlers for libshout
 *
 *  Copyright (C) 2012, 2016 Stephen Fairchild <s-fairchild@users.sourceforge.net>
 *  Copyright (C) 2002-2003 the Icecast team <team@icecast.org>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <shoutidjc/shout.h>
#include "shout_private.h"

/* -- header sizes -- */
#define MPEG1_AUDIO_HEADER_SIZE (4)
#define MAX_ADTS_HEADER_SIZE (9)
#define MIN_ADTS_HEADER_SIZE (7)  /* Excluding optional checksum */

/* -- header structures -- */
typedef struct {
	unsigned int samplerate;
	unsigned int samples;
	unsigned int framesize; /* physical size in bytes */
} header_digest_t;

typedef struct {
	int syncword;   /* always 0xFFF */
	int version;	/* 0 for mpeg4, 1 for mpeg 2 */
	int layer;	  	/* always 0 */
	int protection_absent;
	int profile;	/* the mpeg audio object type minus 1 */
	int samplerate_index;
	int private_stream;
	int channel_configuration;
	int originality;
	int home;
	int copyrighted_stream;
	int copyright_start;
	int buffer_fullness;
	int extra_frames; /* number of additional aac frames (RDBs) */
} adts_header_info_t;

typedef struct {
	int syncword;
	int layer;
	int version;
	int versionbits;
	int error_protection;
	int bitrate_index;
	int samplerate_index;
	int padding;
	int extension;
	int mode;
	int mode_ext;
	int copyright;
	int original;
	int emphasis;
	int bitrate;
	int slotsize;
} mpeg1_audio_header_info_t;

/* -- local state -- */
typedef struct {
	/* running count of number of frames processed */
	unsigned int frames;
	/* the number of samples for the current frame */
	int frame_samples;
	/* the samplerate of the current frame */
	int frame_samplerate;
	/* how many bytes for the rest of this frame */
	unsigned int frame_left;
	/* is the header bridged?? */
	int header_bridges;
	/* put part of header here if it spans a boundary */
	unsigned char *header_bridge;
	/* the size of the parsed part of the header */
	int header_size;
	/* the reader to use */
	int (*header_read)(const uint8_t *window, header_digest_t *mh);
} mpeg_data_t;

/* -- static prototypes -- */
static int open_mpeg(shout_t *self, size_t header_size, int (*reader)(const uint8_t *, header_digest_t *));
static int send_mpeg(shout_t *self, const unsigned char *buff, size_t len);
static void close_mpeg(shout_t *self);
static void parse_adts_header(adts_header_info_t *h, header_digest_t *mh, const uint8_t *window);
static int adts_header(const uint8_t *window, header_digest_t *mh);
static void parse_mpeg1_audio_header(mpeg1_audio_header_info_t *h, header_digest_t *mh, const uint8_t *window);
static int mpeg1_audio_header(const uint8_t *window, header_digest_t *mh);

int shout_open_adts(shout_t *self)
{
	return open_mpeg(self, MIN_ADTS_HEADER_SIZE, adts_header);
}

int shout_open_mp3(shout_t *self)
{
	return open_mpeg(self, MPEG1_AUDIO_HEADER_SIZE, mpeg1_audio_header);
}

static int open_mpeg(shout_t *self, size_t header_size, int (*reader)(const uint8_t *, header_digest_t *))
{
	mpeg_data_t *mpeg_data;

	if (!(mpeg_data = (mpeg_data_t *)calloc(1, sizeof(mpeg_data_t))))
		return SHOUTERR_MALLOC;
	if (!(mpeg_data->header_bridge = (unsigned char *)malloc(header_size - 1)))
		return SHOUTERR_MALLOC;

	self->format_data = mpeg_data;

	self->send = send_mpeg;
	self->close = close_mpeg;

	mpeg_data->header_size = header_size;
	mpeg_data->header_read = reader;

	return SHOUTERR_SUCCESS;
}

static int send_mpeg(shout_t *self, const unsigned char *buff, size_t len)
{
	mpeg_data_t* mpeg_data = (mpeg_data_t*) self->format_data;
	unsigned long pos;
	int ret, count;
	int start, end, error, i;
	unsigned char *bridge_buff;
	header_digest_t mh;

	bridge_buff = NULL;
	pos = 0;
	start = 0;
	error = 0;
	end = len - 1;

	/* finish the previous frame */
	if (mpeg_data->frame_left > 0) {
		/* is the rest of the frame here? */
		if (mpeg_data->frame_left <= len) {
			self->senttime += (int64_t)((double)mpeg_data->frame_samples / (double)mpeg_data->frame_samplerate * 1000000.0);
			mpeg_data->frames++;
			pos += mpeg_data->frame_left;
			mpeg_data->frame_left = 0;
		} else {
			mpeg_data->frame_left -= len;
			pos = len;
		}
	}

	/* header was over the boundary, so build a new build a new buffer */
	if (mpeg_data->header_bridges) {
		bridge_buff = (unsigned char *)malloc(len + mpeg_data->header_bridges);
		if (bridge_buff == NULL) {
			return self->error = SHOUTERR_MALLOC;
		}

		memcpy(bridge_buff, mpeg_data->header_bridge, mpeg_data->header_bridges);
		memcpy(&bridge_buff[mpeg_data->header_bridges], buff, len);

		buff = bridge_buff;
		len += mpeg_data->header_bridges;
		end = len - 1;

		mpeg_data->header_bridges = 0;
	}

	/** this is the main loop
	*** we handle everything except the last few bytes...
	**/
	while ((pos + mpeg_data->header_size) <= len) {
		/* is this a valid header? */
		if (mpeg_data->header_read(&buff[pos], &mh)) {
			if (error) {
				start = pos;
				end = len - 1;
				error = 0;
			}

			mpeg_data->frame_samples = mh.samples;
			mpeg_data->frame_samplerate = mh.samplerate;

			/* do we have a complete frame in this buffer? */
			if (len - pos >= mh.framesize) {
				self->senttime += (int64_t)((double)mpeg_data->frame_samples / (double)mpeg_data->frame_samplerate * 1000000.0);
				mpeg_data->frames++;
				pos += mh.framesize;
			} else {
				mpeg_data->frame_left = mh.framesize - (len - pos);
				pos = len;
			}
		} else {
			/* there was an error
			** so we send all the valid data up to this point
			*/

			if (!error) {
				error = 1;
				end = pos - 1;
				count = end - start + 1;
				if (count > 0)
					ret = (int)shout_send_raw(self, (unsigned char *)&buff[start], count);
				else
					ret = 0;

				if (ret != count) {
					if (bridge_buff != NULL)
						free(bridge_buff);
					return self->error = SHOUTERR_SOCKET;
				}
			}
			pos++;
		}
	}

	/* catch the tail if there is one */
	if ((pos > (len - mpeg_data->header_size)) && (pos < len)) {
		end = pos - 1;

		i = 0;
		while (pos < len) {
			mpeg_data->header_bridge[i] = buff[pos];
			pos++;
			i++;
		}
		mpeg_data->header_bridges = i;
	}

	if (!error) {
		/* if there are no errors, send the frames */
		count = end - start + 1;
		if (count > 0)
			ret = (int)shout_send_raw(self, (unsigned char *)&buff[start], count);
		else
			ret = 0;

		if (bridge_buff != NULL)
			free(bridge_buff);

		if (ret == count) {
			return self->error = SHOUTERR_SUCCESS;
		} else {
			return self->error = SHOUTERR_SOCKET;
		}
	}

	if (bridge_buff != NULL)
		free(bridge_buff);

	return self->error = SHOUTERR_SUCCESS;
}

static void close_mpeg(shout_t *self)
{
	mpeg_data_t *mpeg_data = (mpeg_data_t *)self->format_data;

	free(mpeg_data->header_bridge);
	free(mpeg_data);
}

/* -- adts/aac frame parsing stuff -- */
static void parse_adts_header(adts_header_info_t *h, header_digest_t *mh, const uint8_t *window)
{
	static const unsigned int samplerate[16] = {
		96000, 88200, 64000, 48000, 44100, 32000, 24000,
		22050, 16000, 12000, 11025, 8000, 7350, 0, 0, 0
	};

	h->syncword = (int)window[0] << 4 | window[1] >> 4;
	h->layer = window[1] >> 1 & 0x3;
	h->protection_absent = window[1] & 0x1;
	h->samplerate_index = window[2] >> 2 & 0xf;
	h->extra_frames = window[6] & 0x3;

	mh->samplerate = samplerate[h->samplerate_index];
	mh->samples = 1024 * (1 + h->extra_frames);
	mh->framesize = ((int)window[3] << 11 | (int)window[4] << 3 | window[5] >> 5) & 0x1fff;

	/* -- unused values -- */
	#if 0
	h->version = window[1] >> 3 & 0x1;
	h->profile = window[2] >> 6;
	h->private_stream = window[2] >> 1 & 0x1;
	h->channel_configuration = ((int)window[2] << 2 | window[3] >> 6) & 0x7;
	h->originality = window[3] >> 5 & 0x1;
	h->home = window[3] >> 4 & 0x1;
	h->copyrighted_stream = window[3] >> 3 & 0x1;
	h->copyright_start = window[3] >> 2 & 0x1;
	h->buffer_fullness = ((int)window[5] << 6 | window[6] >> 2) & 0x7ff;
	#endif
}

static int adts_header(const uint8_t *window, header_digest_t *mh)
{
	adts_header_info_t h;

	/* fill out the header structs */
	parse_adts_header(&h, mh, window);

	/* check for syncword */
	if (h.syncword != 0xfff)
		return 0;

	/* check that layer is valid */
	if (h.layer != 0)
		return 0;

	/* make sure sample rate is sane */
	if (mh->samplerate == 0)
		return 0;

	/* make sure frame length is sane */
	if (mh->framesize < (h.protection_absent ? MIN_ADTS_HEADER_SIZE : MAX_ADTS_HEADER_SIZE))
		return 0;

	return 1;
}

/* -- mp3 frame parsing stuff -- */
static void parse_mpeg1_audio_header(mpeg1_audio_header_info_t *h, header_digest_t *mh, const uint8_t *window)
{
	static const unsigned int bitrate[3][3][16] =
	{
		{
			{ 0, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448, 0 },
			{ 0, 32, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 384, 0 },
			{ 0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 0 }
		}, {
			{ 0, 32, 48, 56, 64, 80, 96, 112, 128, 144, 160, 176, 192, 224, 256, 0 },
			{ 0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160, 0 },
			{ 0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160, 0 }
		}, {
			{ 0, 32, 48, 56, 64, 80, 96, 112, 128, 144, 160, 176, 192, 224, 256, 0 },
			{ 0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160, 0 },
			{ 0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160, 0 }
		}
	};

	static const unsigned int samplerate[3][4] =
	{
		{ 44100, 48000, 32000, 0 },
		{ 22050, 24000, 16000, 0 },
		{ 11025, 12000, 8000, 0 }
	};

	static const unsigned int samples[3][4] =
	{
		{ 384, 1152, 1152, 0 },
		{ 384, 1152, 576, 0 },
		{ 384, 1152, 576, 0 }
	};
	
	static const unsigned int slotsize[4] = 
	{
		4, 1, 1, 0
	};

	uint32_t head = (window[0] << 24) |
			(window[1] << 16) |
			(window[2] << 8) |
			(window[3]);

	h->syncword = (head >> 20) & 0xffe;
	h->versionbits = (head >> 19) & 0x3;
	h->version = (unsigned int[4]){2, 0, 1, 0}[h->versionbits];
	h->layer = 3 - ((head >> 17) & 0x3);
	h->bitrate_index = (head >> 12) & 0xf;
	h->samplerate_index = (head >> 10) & 0x3;
	h->padding = (head >> 9) & 0x1;
	h->mode = (head >> 6) & 0x3;
	h->bitrate = bitrate[h->version][h->layer][h->bitrate_index];
	h->slotsize = slotsize[h->layer];
	mh->samplerate = samplerate[h->version][h->samplerate_index];
	mh->samples = samples[h->version][h->layer];

	if (mh->samplerate && h->slotsize)
		mh->framesize = (mh->samples * h->bitrate * 1000 /
				(8 * mh->samplerate * h->slotsize)) * h->slotsize + h->padding;

	/* -- unused values -- */
	#if 0
	h->extension = (head >> 8) & 0x1;
	h->error_protection = ((head >> 16) & 0x1) ? 0 : 1;
	h->mode_ext = (head >> 4) & 0x3;
	h->copyright = (head >> 3) & 0x1;
	h->original = (head >> 2) & 0x1;
	h->emphasis = head & 0x3;
	#endif
}

static int mpeg1_audio_header(const uint8_t *window, header_digest_t *mh)
{
	mpeg1_audio_header_info_t h;

	/* fill out the header struct */
	parse_mpeg1_audio_header(&h, mh, window);

	/* check for syncword */
	if (h.syncword != 0xffe)
		return 0;

	/* check version is not reserved */
	if (h.versionbits == 1)
		return 0;

	/* check that layer is sane */
	if (h.layer == 0)
		return 0;

	/* check that bitrate is sane */
	if (h.bitrate == 0)
		return 0;

	/* check that samplerate is sane */
	if (mh->samplerate == 0)
		return 0;

	/* version 2.5 only permitted with mp3 */
	if (h.version == 2 && h.layer != 2)
		return 0;

	/* mp2 v1 has banned modes at certain bitrates */
	if (h.layer == 1 && h.version == 0) {
		if (h.mode != 3 && h.bitrate <= 80 && h.bitrate != 64)
			return 0;
		if (h.mode == 3 && h.bitrate >= 224)
			return 0;
	}

	return 1;
}
