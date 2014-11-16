/**
 * \file sourdsourceflac.h
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

#include "soundsource.h"
#include "util/defs.h"
#include "util/types.h"

#include <FLAC/stream_decoder.h>

#include <QFile>
#include <QString>

#include <vector>

class SoundSourceFLAC : public Mixxx::SoundSource {
    typedef SoundSource Super;

public:
    static QList<QString> supportedFileExtensions();

    explicit SoundSourceFLAC(QString filename);
    ~SoundSourceFLAC();

    Result parseHeader() /*override*/;

    QImage parseCoverArt() /*override*/;

    Result open() /*override*/;
    void close() /*override*/;

    diff_type seekFrame(diff_type frameIndex) /*override*/;
    size_type readFrameSamplesInterleaved(size_type frameCount, sample_type* sampleBuffer) /*override*/;
    size_type readStereoFrameSamplesInterleaved(size_type frameCount, sample_type* sampleBuffer) /*override*/;

    // callback methods
    FLAC__StreamDecoderReadStatus flacRead(FLAC__byte buffer[], size_t *bytes);
    FLAC__StreamDecoderSeekStatus flacSeek(FLAC__uint64 offset);
    FLAC__StreamDecoderTellStatus flacTell(FLAC__uint64 *offset);
    FLAC__StreamDecoderLengthStatus flacLength(FLAC__uint64 *length);
    FLAC__bool flacEOF();
    FLAC__StreamDecoderWriteStatus flacWrite(const FLAC__Frame *frame, const FLAC__int32 *const buffer[]);
    void flacMetadata(const FLAC__StreamMetadata *metadata);
    void flacError(FLAC__StreamDecoderErrorStatus status);

private:
    /*non-virtual*/ void closeThis();

    size_type readFrameSamplesInterleaved(size_type frameCount, sample_type* sampleBuffer, bool readStereoSamples);

    QFile m_file;

    FLAC__StreamDecoder *m_decoder;
    // misc bits about the flac format:
    // flac encodes from and decodes to LPCM in blocks, each block is made up of
    // subblocks (one for each chan)
    // flac stores in 'frames', each of which has a header and a certain number
    // of subframes (one for each channel)
    size_type m_minBlocksize; // in time samples (audio samples = time samples * chanCount)
    size_type m_maxBlocksize;
    size_type m_minFramesize;
    size_type m_maxFramesize;
    sample_type m_sampleScale;

    typedef std::vector<sample_type> SampleBuffer;
    std::vector<sample_type> m_decodeSampleBuffer;
    SampleBuffer::size_type m_decodeSampleBufferReadOffset;
    SampleBuffer::size_type m_decodeSampleBufferWriteOffset;
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
