/* -*- c-basic-offset: 8; -*- */
/* mp3.c: libshout MP3 format handler
 * $Id$
 *
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

#include <shout/shout.h>
#include "shout_private.h"

/*
 * MP3 frame handling courtesy of Scott Manley - may he always be Manley.
 */

#define MPEG_MODE_MONO 3

/* -- local datatypes -- */
typedef struct {
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
	unsigned char header_bridge[3];
} mp3_data_t;

typedef struct {
	int syncword;
	int layer;
	int version;
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
	int stereo;
	int bitrate;
	unsigned int samplerate;
	unsigned int samples;
	unsigned int framesize;
} mp3_header_t;

/* -- const data -- */
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
	{ 11025, 8000, 8000, 0 }
};

/* -- static prototypes -- */
static int send_mp3(shout_t *self, const unsigned char *data, size_t len);
static void close_mp3(shout_t *self);

static void parse_header(mp3_header_t *mh, uint32_t header);
static int mp3_header(uint32_t head, mp3_header_t *mh);

int shout_open_mp3(shout_t *self)
{
	mp3_data_t *mp3_data;

	if (!(mp3_data = (mp3_data_t *)calloc(1, sizeof(mp3_data_t))))
		return SHOUTERR_MALLOC;
	self->format_data = mp3_data;

	self->send = send_mp3;
	self->close = close_mp3;

	return SHOUTERR_SUCCESS;
}

static int send_mp3(shout_t* self, const unsigned char* buff, size_t len)
{
	mp3_data_t* mp3_data = (mp3_data_t*) self->format_data;
	unsigned long pos;
	uint32_t head;
	int ret, count;
	int start, end, error, i;
	unsigned char *bridge_buff;
	mp3_header_t mh;

	bridge_buff = NULL;
	pos = 0;
	start = 0;
	error = 0;
	end = len - 1;
	memset(&mh, 0, sizeof(mh));

	/* finish the previous frame */
	if (mp3_data->frame_left > 0) {
		/* is the rest of the frame here? */
		if (mp3_data->frame_left <= len) {
			self->senttime += (int64_t)((double)mp3_data->frame_samples / (double)mp3_data->frame_samplerate * 1000000);
			mp3_data->frames++;
			pos += mp3_data->frame_left;
			mp3_data->frame_left = 0;
		} else {
			mp3_data->frame_left -= len;
			pos = len;
		}
	}

	/* header was over the boundary, so build a new build a new buffer */
	if (mp3_data->header_bridges) {
		bridge_buff = (unsigned char *)malloc(len + mp3_data->header_bridges);
		if (bridge_buff == NULL) {
			return self->error = SHOUTERR_MALLOC;
		}

		bridge_buff[0] = mp3_data->header_bridge[0];
		bridge_buff[1] = mp3_data->header_bridge[1];
		bridge_buff[2] = mp3_data->header_bridge[2];

		memcpy(&bridge_buff[mp3_data->header_bridges], buff, len);

		buff = bridge_buff;
		len += mp3_data->header_bridges;
		end = len - 1;

		mp3_data->header_bridges = 0;
	}

	/** this is the main loop
	*** we handle everything but the last 4 bytes...
	**/
	while ((pos + 4) <= len) {
		/* find mp3 header */
		head = (buff[pos] << 24) | 
			(buff[pos + 1] << 16) |
			(buff[pos + 2] << 8) |
			(buff[pos + 3]);

		/* is this a valid header? */
		if (mp3_header(head, &mh)) {
			if (error) {
				start = pos;
				end = len - 1;
				error = 0;
			}

			mp3_data->frame_samples = mh.samples;
			mp3_data->frame_samplerate = mh.samplerate;

			/* do we have a complete frame in this buffer? */
			if (len - pos >= mh.framesize) {
				self->senttime += (int64_t)((double)mp3_data->frame_samples / (double)mp3_data->frame_samplerate * 1000000);
				mp3_data->frames++;
				pos += mh.framesize;
			} else {
				mp3_data->frame_left = mh.framesize - (len - pos);
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
					ret = shout_send_raw(self, &buff[start], count);
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
	if ((pos > (len - 4)) && (pos < len)) {
		end = pos - 1;

		i = 0;
		while (pos < len) {
			mp3_data->header_bridge[i] = buff[pos];
			pos++;
			i++;
		} 
		mp3_data->header_bridges = i;
	}

	if (!error) {
		/* if there's no errors, lets send the frames */
		count = end - start + 1;
		if (count > 0)
			ret = shout_send_raw(self, &buff[start], count);
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

static void parse_header(mp3_header_t *mh, uint32_t header)
{
	mh->syncword = (header >> 20) & 0x0fff;
	mh->version = ((header >> 19) & 0x01) ? 0 : 1;
	if ((mh->syncword & 0x01) == 0)
		mh->version = 2;
	mh->layer = 3 - ((header >> 17) & 0x03);
	mh->error_protection = ((header >> 16) & 0x01) ? 0 : 1;
	mh->bitrate_index = (header >> 12) & 0x0F;
	mh->samplerate_index = (header >> 10) & 0x03;
	mh->padding = (header >> 9) & 0x01;
	mh->extension = (header >> 8) & 0x01;
	mh->mode = (header >> 6) & 0x03;
	mh->mode_ext = (header >> 4) & 0x03;
	mh->copyright = (header >> 3) & 0x01;
	mh->original = (header >> 2) & 0x01;
	mh->emphasis = header & 0x03;

	mh->stereo = (mh->mode == MPEG_MODE_MONO) ? 1 : 2;
	mh->bitrate = bitrate[mh->version][mh->layer][mh->bitrate_index];
	mh->samplerate = samplerate[mh->version][mh->samplerate_index];

	if (mh->version == 0)
		mh->samples = 1152;
	else
		mh->samples = 576;

	if(mh->samplerate)
		mh->framesize = (mh->samples * mh->bitrate * 1000 / mh->samplerate) / 8 + mh->padding;
}

/* mp3 frame parsing stuff */
static int mp3_header(uint32_t head, mp3_header_t *mh)
{
	/* fill out the header struct */
	parse_header(mh, head);

	/* check for syncword */
	if ((mh->syncword & 0x0ffe) != 0x0ffe)
		return 0;

	/* check that layer is valid */
	if (mh->layer == 0)
		return 0;

	/* make sure bitrate is sane */
	if (mh->bitrate == 0)
		return 0;

	/* make sure samplerate is sane */
	if (mh->samplerate == 0)
		return 0;

	return 1;
}

static void close_mp3(shout_t *self)
{
	mp3_data_t *mp3_data = (mp3_data_t *)self->format_data;

	free(mp3_data);
}
