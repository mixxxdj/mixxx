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

#include <QFile>
#include <QString>

#include <FLAC/stream_decoder.h>

#include "defs.h"
#include "soundsource.h"

class SoundSourceFLAC : public Mixxx::SoundSource {
public:
    SoundSourceFLAC(QString filename);
    ~SoundSourceFLAC();
    int open();
    long seek(long filepos);
    unsigned read(unsigned long size, const SAMPLE *buffer);
    inline long unsigned length();
    int parseHeader();
    static QList<QString> supportedFileExtensions();
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
    // these next two are inline but are defined in the cpp file because
    // they should only be used there -- bkgood
    inline int getShift() const;
    inline FLAC__int16 shift(const FLAC__int32 sample) const;
    QFile m_file;
    FLAC__StreamDecoder *m_decoder;
    FLAC__StreamMetadata_StreamInfo *m_streamInfo;
    unsigned int m_samples; // total number of samples
    unsigned int m_bps; // bits per sample
    // misc bits about the flac format:
    // flac encodes from and decodes to LPCM in blocks, each block is made up of
    // subblocks (one for each chan)
    // flac stores in 'frames', each of which has a header and a certain number
    // of subframes (one for each channel)
    unsigned int m_minBlocksize; // in time samples (audio samples = time samples * chanCount)
    unsigned int m_maxBlocksize;
    unsigned int m_minFramesize;
    unsigned int m_maxFramesize;
    FLAC__int16 *m_flacBuffer; // buffer for the write callback to write a single frame's samples
    unsigned int m_flacBufferLength;
    FLAC__int16 *m_leftoverBuffer; // buffer to place any samples which haven't been used
                                   // at the end of a read call
    unsigned int m_leftoverBufferLength;
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
