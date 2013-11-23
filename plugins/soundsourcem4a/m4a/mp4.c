/*
 * Copyright 2006 dnk <dnk@bjum.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#include "ip.h"
#include "xmalloc.h"
#include "debug.h"
#include "file.h"

#ifdef __MP4V2__
    #include <mp4v2/mp4v2.h>
#else
    #include <mp4.h>
#endif
#include <faad.h>

#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

struct mp4_private {
	char *overflow_buf;
	int overflow_buf_len;

	unsigned char channels;
	unsigned long sample_rate;

	faacDecHandle decoder;		/* typedef void * */

	struct {
		MP4FileHandle handle;	/* typedef void * */

		MP4TrackId track;
		MP4SampleId sample;
		MP4SampleId num_samples;
	} mp4;
};


static MP4TrackId mp4_get_track(MP4FileHandle *handle)
{
	MP4TrackId num_tracks;
	const char *track_type;
	uint8_t obj_type;
	MP4TrackId i;

	num_tracks = MP4GetNumberOfTracks(handle, NULL, 0);

	for (i = 1; i <= num_tracks; i++) {
		track_type = MP4GetTrackType(handle, i);
		if (!track_type)
			continue;

		if (!MP4_IS_AUDIO_TRACK_TYPE(track_type))
			continue;

		/* MP4GetTrackAudioType */
		obj_type = MP4GetTrackEsdsObjectTypeId(handle, i);
		if (obj_type == MP4_INVALID_AUDIO_TYPE)
			continue;

		if (obj_type == MP4_MPEG4_AUDIO_TYPE) {
			obj_type = MP4GetTrackAudioMpeg4Type(handle, i);

			if (MP4_IS_MPEG4_AAC_AUDIO_TYPE(obj_type))
				return i;
		} else {
			if (MP4_IS_AAC_AUDIO_TYPE(obj_type))
				return i;
		}
	}

	return MP4_INVALID_TRACK_ID;
}

static int mp4_open(struct input_plugin_data *ip_data)
{
	struct mp4_private *priv;
	faacDecConfigurationPtr neaac_cfg;
	unsigned char *buf;
	unsigned int buf_size;


	/* http://sourceforge.net/forum/message.php?msg_id=3578887 */
	if (ip_data->remote)
		return -IP_ERROR_FUNCTION_NOT_SUPPORTED;

	/* init private struct */
	priv = xnew0(struct mp4_private, 1);
	ip_data->private = priv;

	priv->decoder = faacDecOpen();

	/* set decoder config */
	neaac_cfg = faacDecGetCurrentConfiguration(priv->decoder);
	neaac_cfg->outputFormat = FAAD_FMT_16BIT;	/* force 16 bit audio */
	neaac_cfg->downMatrix = 1;			/* 5.1 -> stereo */
	faacDecSetConfiguration(priv->decoder, neaac_cfg);

	/* open mpeg-4 file */
	priv->mp4.handle = MP4Read(ip_data->filename, 0);
	if (!priv->mp4.handle) {
		d_print("MP4Read failed\n");
		goto out;
	}

	/* find aac audio track */
	priv->mp4.track = mp4_get_track(priv->mp4.handle);
	if (priv->mp4.track == MP4_INVALID_TRACK_ID) {
		d_print("MP4FindTrackId failed\n");
		goto out;
	}

	priv->mp4.num_samples = MP4GetTrackNumberOfSamples(priv->mp4.handle, priv->mp4.track);

	priv->mp4.sample = 1;

	buf = NULL;
	buf_size = 0;
	if (!MP4GetTrackESConfiguration(priv->mp4.handle, priv->mp4.track, &buf, &buf_size)) {
		/* failed to get mpeg-4 audio config... this is ok.
		 * faacDecInit2() will simply use default values instead.
		 */
		buf = NULL;
		buf_size = 0;
	}

	/* init decoder according to mpeg-4 audio config */
	if (faacDecInit2(priv->decoder, buf, buf_size, &priv->sample_rate, &priv->channels) < 0) {
		free(buf);
		goto out;
	}

	free(buf);

	d_print("sample rate %luhz, channels %u\n", priv->sample_rate, priv->channels);

	ip_data->sf = sf_rate(priv->sample_rate) | sf_channels(priv->channels) | sf_bits(16) | sf_signed(1);
#if defined(WORDS_BIGENDIAN)
	ip_data->sf |= sf_bigendian(1);
#endif

	return 0;

out:
	if (priv->mp4.handle)
		MP4Close(priv->mp4.handle);
	if (priv->decoder)
		faacDecClose(priv->decoder);
	free(priv);
	return -IP_ERROR_FILE_FORMAT;
}

static int mp4_close(struct input_plugin_data *ip_data)
{
	struct mp4_private *priv;

	priv = ip_data->private;

	if (priv->mp4.handle)
		MP4Close(priv->mp4.handle);

	if (priv->decoder)
		faacDecClose(priv->decoder);

	free(priv);
	ip_data->private = NULL;

	return 0;
}

/* returns -1 on fatal errors
 * returns -2 on non-fatal errors
 * 0 on eof
 * number of bytes put in 'buffer' on success */
static int decode_one_frame(struct input_plugin_data *ip_data, void *buffer, int count)
{
	struct mp4_private *priv;
	unsigned char *aac_data = NULL;
	unsigned int aac_data_len = 0;
	faacDecFrameInfo frame_info;
	char *sample_buf;
	int bytes;

	priv = ip_data->private;

	BUG_ON(priv->overflow_buf_len);

	if (priv->mp4.sample > priv->mp4.num_samples)
		return 0; /* EOF */

	if (MP4ReadSample(priv->mp4.handle, priv->mp4.track, priv->mp4.sample,
		&aac_data, &aac_data_len, NULL, NULL, NULL, NULL) == 0) {
		d_print("error reading mp4 sample %d\n", priv->mp4.sample);
		errno = EINVAL;
		return -1;
	}

	priv->mp4.sample++;

	if (!aac_data) {
		d_print("aac_data == NULL\n");
		errno = EINVAL;
		return -1;
	}

	sample_buf = faacDecDecode(priv->decoder, &frame_info, aac_data, aac_data_len);

	free(aac_data);

	if (!sample_buf || frame_info.bytesconsumed <= 0) {
		d_print("fatal error: %s\n", faacDecGetErrorMessage(frame_info.error));
		errno = EINVAL;
		return -1;
	}

	if (frame_info.error != 0) {
		d_print("frame error: %s\n", faacDecGetErrorMessage(frame_info.error));
		return -2;
	}

	if (frame_info.samples <= 0)
		return -2;

	if (frame_info.channels != priv->channels || frame_info.samplerate != priv->sample_rate) {
		d_print("invalid channel or sample_rate count\n");
		return -2;
	}

	/* 16-bit samples */
	bytes = frame_info.samples * 2;

	if (bytes > count) {
		/* decoded too much; keep overflow. */
		priv->overflow_buf = sample_buf + count;
		priv->overflow_buf_len = bytes - count;
		memcpy(buffer, sample_buf, count);
		return count;
	} else {
		memcpy(buffer, sample_buf, bytes);
	}

	return bytes;
}

static int mp4_read(struct input_plugin_data *ip_data, char *buffer, int count)
{
	struct mp4_private *priv;
	int rc;

	priv = ip_data->private;

	/* use overflow from previous call (if any) */
	if (priv->overflow_buf_len > 0) {
		int len = priv->overflow_buf_len;

		if (len > count)
			len = count;

		memcpy(buffer, priv->overflow_buf, len);
		priv->overflow_buf += len;
		priv->overflow_buf_len -= len;

		return len;
	}

	do {
		rc = decode_one_frame(ip_data, buffer, count);
	} while (rc == -2);

	return rc;
}

static int mp4_seek(struct input_plugin_data *ip_data, double offset)
{
	struct mp4_private *priv;
	MP4SampleId sample;
	uint32_t scale;

	priv = ip_data->private;

	scale = MP4GetTrackTimeScale(priv->mp4.handle, priv->mp4.track);
	if (scale == 0)
		return -IP_ERROR_INTERNAL;

	sample = MP4GetSampleIdFromTime(priv->mp4.handle, priv->mp4.track,
		(MP4Timestamp)(offset * (double)scale), 0);
	if (sample == MP4_INVALID_SAMPLE_ID)
		return -IP_ERROR_INTERNAL;

	priv->mp4.sample = sample;

	d_print("seeking to sample %d\n", sample);

	return 0;
}

static int mp4_read_comments(struct input_plugin_data *ip_data,
		struct keyval **comments)
{
	struct mp4_private *priv;
	uint16_t meta_num, meta_total;
	uint8_t val;
	/*uint8_t *ustr;
	uint32_t size;*/
	char *str;
	GROWING_KEYVALS(c);

	priv = ip_data->private;

	/* MP4GetMetadata* provides malloced pointers, and the data
	 * is in UTF-8 (or at least it should be). */
	if (MP4GetMetadataArtist(priv->mp4.handle, &str))
		comments_add(&c, "artist", str);
	if (MP4GetMetadataAlbum(priv->mp4.handle, &str))
		comments_add(&c, "album", str);
	if (MP4GetMetadataName(priv->mp4.handle, &str))
		comments_add(&c, "title", str);
	if (MP4GetMetadataGenre(priv->mp4.handle, &str))
		comments_add(&c, "genre", str);
	if (MP4GetMetadataYear(priv->mp4.handle, &str))
		comments_add(&c, "date", str);

	if (MP4GetMetadataCompilation(priv->mp4.handle, &val))
		comments_add_const(&c, "compilation", val ? "yes" : "no");
#if 0
	if (MP4GetBytesProperty(priv->mp4.handle, "moov.udta.meta.ilst.aART.data", &ustr, &size)) {
		char *xstr;

		/* What's this?
		 * This is the result from lack of documentation.
		 * It's supposed to return just a string, but it
		 * returns an additional 16 bytes of junk at the
		 * beginning. Could be a bug. Could be intentional.
		 * Hopefully this works around it:
		 */
		if (ustr[0] == 0 && size > 16) {
			ustr += 16;
			size -= 16;
		}
		xstr = xmalloc(size + 1);
		memcpy(xstr, ustr, size);
		xstr[size] = 0;
		comments_add(&c, "albumartist", xstr);
		free(xstr);
	}
#endif
	if (MP4GetMetadataTrack(priv->mp4.handle, &meta_num, &meta_total)) {
		char buf[6];
		snprintf(buf, 6, "%u", meta_num);
		comments_add_const(&c, "tracknumber", buf);
	}
	if (MP4GetMetadataDisk(priv->mp4.handle, &meta_num, &meta_total)) {
		char buf[6];
		snprintf(buf, 6, "%u", meta_num);
		comments_add_const(&c, "discnumber", buf);
	}

	comments_terminate(&c);
	*comments = c.comments;
	return 0;
}

static int mp4_duration(struct input_plugin_data *ip_data)
{
	struct mp4_private *priv;
	uint32_t scale;
	uint64_t duration;

	priv = ip_data->private;

	scale = MP4GetTrackTimeScale(priv->mp4.handle, priv->mp4.track);
	if (scale == 0)
		return 0;

	duration = MP4GetTrackDuration(priv->mp4.handle, priv->mp4.track);

	return duration / scale;
}

const struct input_plugin_ops ip_ops = {
	.open = mp4_open,
	.close = mp4_close,
	.read = mp4_read,
	.seek = mp4_seek,
	.read_comments = mp4_read_comments,
	.duration = mp4_duration
};

const char * const ip_extensions[] = { "mp4", "m4a", "m4b", NULL };
const char * const ip_mime_types[] = { /*"audio/mp4", "audio/mp4a-latm",*/ NULL };
