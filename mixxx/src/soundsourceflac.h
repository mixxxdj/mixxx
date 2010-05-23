/**
 * \file SoundSourceFLAC.h
 * \class SoundSourceFLAC
 * \brief Decodes FLAC files using libFLAC for Mixxx.
 * \author Bill Good <bkgood at gmail dot com>
 * \date May 22, 2010
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SOUNDSOURCEFLAC_H
#define SOUNDSOURCEFLAC_H

#include <QString>
#include <Qt3Support>
#include <FLAC/stream_decoder.h>

#include "defs.h"

class TrackInfoObject;

class SoundSourceFLAC : public SoundSource {
public:
    SoundSourceFLAC(QString fileName);
    ~SoundSourceFLAC();
    long seek(long);
    unsigned read(unsigned long size, const SAMPLE*);
    inline long unsigned length();
    static int ParseHeader(TrackInfoObject*);
    // callback methods
    void decodeCallback(const FLAC__Frame *frame, const FLAC__int32)
private:
    QFile m_file;
    FLAC__StreamDecoder *m_decoder;
    unsigned int SRATE;
    unsigned int m_channels; // number of channels
    unsigned int m_samples; // total number of samples
    unsigned int m_bps; // bits per sample (not beats per second!)
};

// callbacks for libFLAC
FLAC__StreamDecoderReadStatus FLAC_read_cb(const FLAC__StreamDecoder *decoder, FLAC__byte buffer[], size_t *bytes, void *client_data);
FLAC__StreamDecoderSeekStatus FLAC_seek_cb(const FLAC__StreamDecoder *decoder, FLAC__uint64 absolute_byte_offset, void *client_data);
FLAC__StreamDecoderTellStatus FLAC_tell_cb(const FLAC__StreamDecoder *decoder, FLAC__uint64 *absolute_byte_offset, void *client_data);
FLAC__StreamDecoderLengthStatus FLAC_length_cb(const FLAC__StreamDecoder *decoder, FLAC__uint64 *stream_length, void *client_data);
FLAC__bool FLAC_eof_cb(const FLAC__StreamDecoder *decoder, void *client_data);
FLAC__StreamDecoderWriteStatus FLAC_write_cb(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 *const buffer[], void *client_data);
void FLAC_metadata_cb(const FLAC__StreamDecoder *decoder, const FLAC__StreamMetadata *metadata, void *client_data);
void FLAC_error_cb(const FLAC__StreamDecoder *decoder, FLAC__StreamDecoderErrorStatus status, void *client_data);

#endif // ifndef SOUNDSOURCEFLAC_H
