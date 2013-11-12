// mp4-mixxx.cpp
// This file is a hopefully shortlived fork + convertsion to C++ of the M4A audio playback plugin from the C* Music Player (cmus) project.
// The original file mp4.c is also in this directory.
//
// This forked and converted by Garth and Albert in Summer 2008 to support M4A playback in Mixxx
//
// g++ $(pkg-config --cflags QtCore) $(pkg-config --libs-only-l QtCore) -lmp4v2 -lfaad -o mp4-mixxx mp4-mixxx.cpp
//
#include <QtCore>
#include <stdlib.h>

#include "mathstuff.h"

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
// #include "xmalloc.h"
// #include "debug.h"
// #include "file.h"

#ifdef __MP4V2__
    #include <mp4v2/mp4v2.h>
#else
    #include <mp4.h>
#endif

#include <neaacdec.h>

#include <errno.h>
#include <string.h>
#include <sys/types.h>
#ifndef _MSC_VER
    #include <unistd.h>
#endif

#ifdef _MSC_VER
    #define S_ISDIR(mode) (mode & _S_IFDIR)
    #define strcasecmp stricmp
    #define strncasecmp strnicmp
#endif

#ifdef __M4AHACK__
    typedef uint32_t SAMPLERATE_TYPE;
#else
    typedef unsigned long SAMPLERATE_TYPE;
#endif

struct mp4_private {
    char *overflow_buf;
    int overflow_buf_len;

    unsigned char *aac_data;
    unsigned int aac_data_len;

    char *sample_buf;
    unsigned int sample_buf_frame;
    unsigned int sample_buf_len;

    unsigned char channels;
    unsigned long sample_rate;

    faacDecHandle decoder; /* typedef void * */

    struct {
        MP4FileHandle handle; /* typedef void * */

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

    /* init private_ipd struct */
    //  priv = xnew0(struct mp4_private, 1);
    priv = new mp4_private();
    //priv = (mp4_private*) calloc(1, sizeof(mp4_private));
    // FIXME: there was some alloc error checking in the orgininal ver
    memset(priv, 0, sizeof(*priv));

    priv->overflow_buf_len = 0;
    priv->overflow_buf = NULL;

    priv->sample_buf_len = 4096;
    priv->sample_buf = new char[priv->sample_buf_len];
    priv->sample_buf_frame = -1;

    ip_data->private_ipd = priv;

    priv->decoder = faacDecOpen();
    /* set decoder config */
    neaac_cfg = faacDecGetCurrentConfiguration(priv->decoder);
    neaac_cfg->outputFormat = FAAD_FMT_16BIT; /* force 16 bit audio */
    neaac_cfg->downMatrix = 1; /* 5.1 -> stereo */
    neaac_cfg->defObjectType = LC;
    //qDebug() << "Decoder Config" << neaac_cfg->defObjectType
    //         << neaac_cfg->defSampleRate
    //         << neaac_cfg->useOldADTSFormat
    //         << neaac_cfg->dontUpSampleImplicitSBR;
    faacDecSetConfiguration(priv->decoder, neaac_cfg);

    /* open mpeg-4 file, check for >= ver 1.9.1 */
#if MP4V2_PROJECT_version_hex <= 0x00010901
    priv->mp4.handle = MP4Read(ip_data->filename, 0);
#else
    priv->mp4.handle = MP4Read(ip_data->filename);
#endif
    if (!priv->mp4.handle) {
        qDebug() << "MP4Read failed";
        goto out;
    }

    /* find aac audio track */
    priv->mp4.track = mp4_get_track((MP4FileHandle*)priv->mp4.handle);
    if (priv->mp4.track == MP4_INVALID_TRACK_ID) {
        qDebug() << "MP4FindTrackId failed";
        goto out;
    }

    // Allocate AAC read buffer
    priv->aac_data_len = MP4GetTrackMaxSampleSize(priv->mp4.handle, priv->mp4.track);
    priv->aac_data = new unsigned char[priv->aac_data_len];

    priv->mp4.num_samples = MP4GetTrackNumberOfSamples(priv->mp4.handle, priv->mp4.track);
    // MP4 frames are 1-indexed
    priv->mp4.sample = 1;

    buf = NULL;
    buf_size = 0;
    if (!MP4GetTrackESConfiguration(priv->mp4.handle, priv->mp4.track, &buf, &buf_size)) {
        /* failed to get mpeg-4 audio config... this is ok.
         * faacDecInit2() will simply use default values instead.
         */
        qDebug() << "Didn't get MP4 Audio Config (not a bad thing)";
            buf = NULL;
            buf_size = 0;
    }

    /* init decoder according to mpeg-4 audio config */
    if (faacDecInit2(priv->decoder, buf, buf_size,
            (SAMPLERATE_TYPE*)&priv->sample_rate, &priv->channels) < 0) {
        free(buf);
        goto out;
    }
    free(buf);

    // qDebug() << "sample rate "<< priv->sample_rate <<"hz, channels" << priv->channels;

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
    delete [] priv->sample_buf;
    delete [] priv->aac_data;
    delete priv;
    return -IP_ERROR_FILE_FORMAT;
}

static int mp4_close(struct input_plugin_data *ip_data)
{
    struct mp4_private *priv;

    priv = (mp4_private*) ip_data->private_ipd;

    if (priv->mp4.handle)
        MP4Close(priv->mp4.handle);

    if (priv->decoder)
        faacDecClose(priv->decoder);

    if (priv->sample_buf) {
        delete [] priv->sample_buf;
    }

    if (priv->aac_data) {
        delete [] priv->aac_data;
    }

    delete priv;
    ip_data->private_ipd = NULL;

    return 0;
}

/* returns -1 on fatal errors
 * returns -2 on non-fatal errors
 * 0 on eof
 * number of bytes put in 'buffer' on success */
static int decode_one_frame(struct input_plugin_data *ip_data, void *buffer, int count)
{
    struct mp4_private *priv = (mp4_private*) ip_data->private_ipd;
    faacDecFrameInfo frame_info;
    int bytes;

    //BUG_ON(priv->overflow_buf_len);

    if (priv->mp4.sample > priv->mp4.num_samples)
        return 0; /* EOF */

    unsigned char *aac_data = priv->aac_data;
    unsigned int aac_data_len = priv->aac_data_len;

    // If you do this, then MP4ReadSample allocates the buffer for you. We don't
    // want this because it's slow.
    // unsigned char *aac_data = NULL;
    // unsigned int aac_data_len = 0;

    int this_frame = priv->mp4.sample;
    if (MP4ReadSample(priv->mp4.handle, priv->mp4.track, this_frame,
                      &aac_data, &aac_data_len,
                      NULL, NULL, NULL, NULL) == 0) {
        qWarning() << "m4a: error reading mp4 sample" << priv->mp4.sample;
        errno = EINVAL;
        return -1;
    }

    if (!aac_data) {
        qWarning() << "m4a: aac_data == NULL";
        errno = EINVAL;
        return -1;
    }

    char* sample_buf = priv->sample_buf;
    int sample_buf_len = priv->sample_buf_len;

    NeAACDecDecode2(priv->decoder,
                  &frame_info,
                  aac_data, aac_data_len,
                  (void**)&sample_buf, sample_buf_len);

    // qDebug() << "Sample frame" << priv->mp4.sample
    //          << "has" << frame_info.samples << "samples"
    //          << frame_info.bytesconsumed << "bytes"
    //          << frame_info.channels << "channels"
    //          << frame_info.error << "error"
    //          << frame_info.samplerate << "samplerate";

    if (!sample_buf || frame_info.bytesconsumed <= 0) {
        qWarning() << "m4a fatal error:" << faacDecGetErrorMessage(frame_info.error);
        errno = EINVAL;
        return -1;
    }

    if (frame_info.error != 0) {
        qDebug() << "frame error:" << faacDecGetErrorMessage(frame_info.error);
        return -2;
    }

    if (frame_info.samples <= 0) {
        return -2;
    }

    if (frame_info.channels != priv->channels ||
      frame_info.samplerate != priv->sample_rate) {
        qDebug() << "invalid channel or sample_rate count\n";
        return -2;
    }

    // The frame read was successful
    priv->sample_buf_frame = this_frame;
    priv->mp4.sample++;

    /* 16-bit samples */
    bytes = frame_info.samples * 2;

    if (bytes > count) {
        /* decoded too much; keep overflow. */
        //memcpy(priv->overflow_buf_base, sample_buf + count, bytes - count);
        //priv->overflow_buf = priv->overflow_buf_base;

        priv->overflow_buf = sample_buf + count;
        priv->overflow_buf_len = bytes - count;
        memcpy(buffer, sample_buf, count);
        return count;
    }

    memcpy(buffer, sample_buf, bytes);
    return bytes;
}

static int mp4_read(struct input_plugin_data *ip_data, char *buffer, int count)
{
    struct mp4_private *priv = (mp4_private*) ip_data->private_ipd;
    int rc;

    /* use overflow from previous call (if any) */
    if (priv->overflow_buf_len > 0) {
        int len = priv->overflow_buf_len;

        if (len > count)
            len = count;

        memcpy(buffer, priv->overflow_buf, len);
        priv->overflow_buf += len;
        priv->overflow_buf_len -= len;

        //qDebug() << "Reading" << len << "from overflow."
        //         << priv->overflow_buf_len << "overflow remains";

        return len;
    }

    do {
        rc = decode_one_frame(ip_data, buffer, count);
    } while (rc == -2);

    return rc;
}

static int mp4_total_samples(struct input_plugin_data *ip_data) {
    struct mp4_private *priv = (struct mp4_private*)ip_data->private_ipd;
    return priv->channels * priv->mp4.num_samples * 1024;
}

static int mp4_current_sample(struct input_plugin_data *ip_data) {
    struct mp4_private *priv = (struct mp4_private*)ip_data->private_ipd;
    int frame_length = priv->channels * 1024;
    if (priv->overflow_buf_len == 0) {
    return priv->mp4.sample * frame_length - priv->overflow_buf_len;
    }
    // rryan 9/2009 This is equivalent to the current sample. The full expression
    // is (priv->mp4.sample - 1) * frame_length + (frame_length -
    // priv->overflow_buf_len); but the frame_length lone terms drop out.

    // -1 because if overflow buf is filled then mp4.sample is incremented, and
    // the samples in the overflow buf are for sample - 1
    return (priv->mp4.sample - 1) * frame_length - priv->overflow_buf_len;
}

static int mp4_seek_sample(struct input_plugin_data *ip_data, int sample)
{
    struct mp4_private *priv;
    priv = (mp4_private*) ip_data->private_ipd;

    Q_ASSERT(sample >= 0);
    // The first frame is samples 0 through 2047. The first sample of the second
    // frame is 2048. 2048 / 2048 = 1, so frame_for_sample will be 2 on the
    // 2048'th sample. The frame_offset_samples is how many samples into the frame
    // the sample'th sample is. For x in (0,2047), the frame offset is x. For x in
    // (2048,4095) the offset is x-2048 and so on. sample % 2048 is therefore
    // suitable for calculating the offset.
    unsigned int frame_for_sample = 1 + (sample / (2 * 1024));
    unsigned int frame_offset_samples = sample % (2 * 1024);
    unsigned int frame_offset_bytes = frame_offset_samples * 2;

    //qDebug() << "Seeking to" << frame_for_sample << ":" << frame_offset;

    // Invalid sample requested -- return the current position.
    if (frame_for_sample < 1 || frame_for_sample > priv->mp4.num_samples)
        return mp4_current_sample(ip_data);

    // We don't have the current frame decoded -- decode it.
    if (priv->sample_buf_frame != frame_for_sample) {

        // We might have to 'prime the pump' if this isn't the first frame. The
        // decoder has internal state that it builds as it plays, and just seeking
        // to the frame we want will result in poor audio quality (clicks and
        // pops). This is akin to seeking in a video and seeing MPEG
        // artifacts. Figure out how many frames we need to go backward -- 1 seems
        // to work.
        const int how_many_backwards = 1;
        int start_frame = math_max(frame_for_sample - how_many_backwards, 1);
        priv->mp4.sample = start_frame;

        // rryan 9/2009 -- the documentation is sketchy on this, but I think that
        // it tells the decoder that you are seeking so it should flush its state
        faacDecPostSeekReset(priv->decoder, priv->mp4.sample);

        // Loop until the current frame is past the frame we intended to read
        // (i.e. we have decoded how_many_backwards + 1 frames). The desidered
        // decoded frame will be stored in the overflow buffer, since we're asking
        // to read 0 bytes.
        int result;
        do {
            result = decode_one_frame(ip_data, 0, 0);
            if (result < 0) qDebug() << "SEEK_ERROR";
        } while (result == -2 || priv->mp4.sample <= frame_for_sample);

        if (result == -1 || result == 0) {
            return mp4_current_sample(ip_data);
        }
    } else {
        qDebug() << "Seek within frame";
    }

    // Now the overflow buffer contains the sample we want to seek to. Fix the
    // overflow buffer so that the next call to read() will read starting with the
    // requested sample.
    priv->overflow_buf = priv->sample_buf;
    priv->overflow_buf += frame_offset_bytes;
    priv->overflow_buf_len -= frame_offset_bytes;

    return mp4_current_sample(ip_data);
}

static int mp4_seek(struct input_plugin_data *ip_data, double offset)
{
    struct mp4_private *priv;
    MP4SampleId sample;
    uint32_t scale;

    priv = (mp4_private*) ip_data->private_ipd;

    scale = MP4GetTrackTimeScale(priv->mp4.handle, priv->mp4.track);
    if (scale == 0)
        return -IP_ERROR_INTERNAL;

    sample = MP4GetSampleIdFromTime(priv->mp4.handle, priv->mp4.track,
        (MP4Timestamp)(offset * (double)scale), 0);
    if (sample == MP4_INVALID_SAMPLE_ID)
        return -IP_ERROR_INTERNAL;

    qDebug() << "seeking from sample" << priv->mp4.sample << "to sample" << sample;
    priv->mp4.sample = sample;
    priv->overflow_buf_len = 0;

    return priv->mp4.sample;
}

/* commented because we use TagLib now ??? -- bkgood
static int mp4_read_comments(struct input_plugin_data *ip_data,
        struct keyval **comments)
{
    struct mp4_private *priv;
    uint16_t meta_num, meta_total;
    uint8_t val;
    uint8_t *ustr;
    uint32_t size;
    char *str;
    GROWING_KEYVALS(c);

    priv = ip_data->private;

     MP4GetMetadata* provides malloced pointers, and the data
     * is in UTF-8 (or at least it should be).
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

         What's this?
         * This is the result from lack of documentation.
         * It's supposed to return just a string, but it
         * returns an additional 16 bytes of junk at the
         * beginning. Could be a bug. Could be intentional.
         * Hopefully this works around it:

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
*/

static int mp4_duration(struct input_plugin_data *ip_data)
{
    struct mp4_private *priv;
    uint32_t scale;
    uint64_t duration;

    priv = (mp4_private*) ip_data->private_ipd;

    scale = MP4GetTrackTimeScale(priv->mp4.handle, priv->mp4.track);
    if (scale == 0)
        return 0;

    duration = MP4GetTrackDuration(priv->mp4.handle, priv->mp4.track);

    return duration / scale;
}
/*
const struct input_plugin_ops ip_ops = {
    .open = mp4_open,
    .close = mp4_close,
    .read = mp4_read,
    .seek = mp4_seek,
    .read_comments = mp4_read_comments,
    .duration = mp4_duration
};

const char * const ip_extensions[] = { "mp4", "m4a", "m4b", NULL };
const char * const ip_mime_types[] = { "audio/mp4", "audio/mp4a-latm", NULL };
*/
