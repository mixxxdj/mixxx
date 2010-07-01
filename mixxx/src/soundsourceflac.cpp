/**
 * \file soundsourceflac.cpp
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

#include <QtDebug>

#include "soundsourceflac.h"

SoundSourceFLAC::SoundSourceFLAC(QString filename)
    : SoundSource(filename)
    , m_file(filename)
    , m_decoder(NULL)
    , m_channels(0)
    , m_samples(0)
    , m_bps(0)
    , m_flacBuffer(NULL)
    , m_leftoverBuffer(NULL)
    , m_flacBufferLength(0) {
    qDebug() << "SSFLAC::ctor";
}

SoundSourceFLAC::~SoundSourceFLAC() {
    qDebug() << "SSFLAC::dtor";
    if (m_flacBuffer != NULL) {
        delete m_flacBuffer;
        m_flacBuffer = NULL;
    }
    if (m_leftoverBuffer != NULL) {
        delete m_leftoverBuffer;
        m_leftoverBuffer = NULL;
    }
    if (m_decoder) {
        FLAC__stream_decoder_finish(m_decoder);
        FLAC__stream_decoder_delete(m_decoder); // frees memory
        m_decoder = NULL; // probably not necessary
    }
}

// soundsource overrides
int SoundSourceFLAC::open() {
    m_file.open(QIODevice::ReadOnly);
    qDebug() << "file at end? " << m_file.atEnd();
    m_decoder = FLAC__stream_decoder_new();
    if (m_decoder == NULL) {
        qDebug() << "decoder allocation failed!";
        return ERR;
    }
    FLAC__StreamDecoderInitStatus initStatus;
    initStatus = FLAC__stream_decoder_init_stream(
        m_decoder, FLAC_read_cb, FLAC_seek_cb, FLAC_tell_cb, FLAC_length_cb,
        FLAC_eof_cb, FLAC_write_cb, FLAC_metadata_cb, FLAC_error_cb,
        (void*) this);
    if (initStatus != FLAC__STREAM_DECODER_INIT_STATUS_OK) {
        qDebug() << "decoder init failed!";
        FLAC__stream_decoder_finish(m_decoder);
        FLAC__stream_decoder_delete(m_decoder);
        m_decoder = NULL;
        return ERR;
    }
    if (!FLAC__stream_decoder_process_until_end_of_metadata(m_decoder)) {
        qDebug() << "process to end of meta failed!";
        qDebug() << "decoder state: " << FLAC__stream_decoder_get_state(m_decoder);
        FLAC__stream_decoder_finish(m_decoder);
        FLAC__stream_decoder_delete(m_decoder);
        m_decoder = NULL;
        return ERR;
    } // now number of samples etc. should be populated
    if (m_bps != 16) {
        qDebug() << "SoundSourceFLAC only supports FLAC files encoded at 16 bits per sample.";
        FLAC__stream_decoder_finish(m_decoder);
        FLAC__stream_decoder_delete(m_decoder);
        m_decoder = NULL;
        return ERR;
    }
    m_flacBuffer = new FLAC__int16[m_maxBlocksize * m_channels];
    m_leftoverBuffer = new FLAC__int16[m_maxBlocksize * m_channels];
    qDebug() << "Total samples: " << m_samples;
    qDebug() << "Sampling rate: " << m_iSampleRate << " Hz";
    qDebug() << "Channels: " << m_channels;
    qDebug() << "BPS: " << m_bps;
    return OK;
}

long SoundSourceFLAC::seek(long filepos) {
    qDebug() << "SSFLAC::seek";
    if (!m_decoder) return 0;
    FLAC__bool seekResult;
    seekResult = FLAC__stream_decoder_seek_absolute(m_decoder, filepos);
    return filepos;
}

unsigned int SoundSourceFLAC::read(unsigned long size, const SAMPLE *destination) {
    qDebug() << "SSFLAC::read";
    if (!m_decoder) return 0;
    SAMPLE *destBuffer = const_cast<SAMPLE*>(destination);
    unsigned int samplesWritten = 0;
    // first, empty the leftover buffer in case there were any samples left from last read()
    for (unsigned int i = 0; m_leftoverBufferLength > 0 && samplesWritten < size; ++i) {
        destBuffer[samplesWritten++] = m_leftoverBuffer[i];
        --m_leftoverBufferLength;
    }
    // then, read samples from FLAC as necessary
    unsigned int i = 0;
    while (samplesWritten < size) {
        if (m_flacBufferLength <= 0) {
            i = 0;
            if (!FLAC__stream_decoder_process_single(m_decoder)) {
                qDebug() << "decoder_process_single returned false";
                break;
            }
        }
        destBuffer[samplesWritten++] = m_flacBuffer[i++];
        --m_flacBufferLength;
    }
    // lastly, move the rest of flacBuffer to leftoverBuffer for the next read call
    i = 0;
    while (m_flacBufferLength > 0) {
        m_leftoverBuffer[m_leftoverBufferLength++] = m_flacBuffer[i++];
        --m_flacBufferLength;
    } 
    return samplesWritten;
}

inline unsigned long SoundSourceFLAC::length() {
    qDebug() << "SSFLAC::length";
    return m_samples * m_channels;
}

int SoundSourceFLAC::parseHeader() {
    qDebug() << "SSFLAC::parseHeader";
    return OK;
}

// static
QList<QString> SoundSourceFLAC::supportedFileExtensions()
{
    QList<QString> list;
    list.push_back("flac");
    return list;
}


// flac callback methods
FLAC__StreamDecoderReadStatus SoundSourceFLAC::flacRead(FLAC__byte buffer[], size_t *bytes) {
    //qDebug() << "SSFLAC::flacRead";
    *bytes = m_file.read((char*) buffer, *bytes);
    if (*bytes > 0) {
        return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
    } else if (*bytes == 0) {
        return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
    } else {
        return FLAC__STREAM_DECODER_READ_STATUS_ABORT;
    }
}

FLAC__StreamDecoderSeekStatus SoundSourceFLAC::flacSeek(FLAC__uint64 offset) {
    //qDebug() << "SSFLAC::flacSeek";
    if (m_file.seek(offset)) {
        return FLAC__STREAM_DECODER_SEEK_STATUS_OK;
    } else {
        return FLAC__STREAM_DECODER_SEEK_STATUS_ERROR;
    }
}

FLAC__StreamDecoderTellStatus SoundSourceFLAC::flacTell(FLAC__uint64 *offset) {
    //qDebug() << "SSFLAC::flacTell";
    if (m_file.isSequential()) {
        return FLAC__STREAM_DECODER_TELL_STATUS_UNSUPPORTED;
    }
    *offset = m_file.pos();
    return FLAC__STREAM_DECODER_TELL_STATUS_OK;
}

FLAC__StreamDecoderLengthStatus SoundSourceFLAC::flacLength(FLAC__uint64 *length) {
    //qDebug() << "SSFLAC::flacLength";
    if (m_file.isSequential()) {
        return FLAC__STREAM_DECODER_LENGTH_STATUS_UNSUPPORTED;
    }
    *length = m_file.size();
    return FLAC__STREAM_DECODER_LENGTH_STATUS_OK;
}

FLAC__bool SoundSourceFLAC::flacEOF() {
    //qDebug() << "SSFLAC::flacEOF";
    if (m_file.isSequential()) {
        return false;
    }
    return m_file.atEnd();
}

FLAC__StreamDecoderWriteStatus SoundSourceFLAC::flacWrite(const FLAC__Frame *frame, const FLAC__int32 *const buffer[]) {
    //qDebug() << "SSFLAC::flacWrite";
    unsigned int i;
    m_flacBufferLength = 0;
    if (frame->header.channels > 1) {
        // stereo (or greater)
        for (i = 0; i < frame->header.blocksize; ++i) {
            m_flacBuffer[i  ] = buffer[0][i]; // left channel
            m_flacBuffer[i+1] = buffer[1][i]; // right channel
            m_flacBufferLength += 2;
        }
    } else {
        // mono (sad)
        qDebug() << "mono!";
        for (i = 0; i < frame->header.blocksize; ++i) {
            m_flacBuffer[i  ] = buffer[0][i]; // left channel
            m_flacBuffer[i+1] = buffer[0][i]; // mono channel
            m_flacBufferLength += 2;
        }
    }
    return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE; // can't anticipate any errors here
}

void SoundSourceFLAC::flacMetadata(const FLAC__StreamMetadata *metadata) {
    //qDebug() << "SSFLAC::flacMetadata";
    if (metadata->type == FLAC__METADATA_TYPE_STREAMINFO) {
        m_samples = metadata->data.stream_info.total_samples;
        m_channels = metadata->data.stream_info.channels;
        m_iSampleRate = metadata->data.stream_info.sample_rate;
        m_bps = metadata->data.stream_info.bits_per_sample;
        m_minBlocksize = metadata->data.stream_info.min_blocksize;
        m_maxBlocksize = metadata->data.stream_info.max_blocksize;
        m_minFramesize = metadata->data.stream_info.min_framesize;
        m_maxFramesize = metadata->data.stream_info.max_framesize;
        qDebug() << "FLAC file " << m_qFilename;
        qDebug() << m_channels << " @ " << m_iSampleRate << " Hz, " << m_samples << " total, " << m_bps << " bps";
        qDebug() << "Blocksize in [" << m_minBlocksize << ", " << m_maxBlocksize << "], Framesize in ["
                 << m_minFramesize << ", " << m_maxFramesize << "]";
    }
}

void SoundSourceFLAC::flacError(FLAC__StreamDecoderErrorStatus status) {
    qDebug() << "SSFLAC::flacError";
    (void) status;
}

// begin callbacks (have to be regular functions because normal libFLAC isn't C++-aware)

FLAC__StreamDecoderReadStatus FLAC_read_cb(const FLAC__StreamDecoder*, FLAC__byte buffer[], size_t *bytes, void *client_data) {
    return ((SoundSourceFLAC*) client_data)->flacRead(buffer, bytes);
}

FLAC__StreamDecoderSeekStatus FLAC_seek_cb(const FLAC__StreamDecoder*, FLAC__uint64 absolute_byte_offset, void *client_data) {
    return ((SoundSourceFLAC*) client_data)->flacSeek(absolute_byte_offset);
}

FLAC__StreamDecoderTellStatus FLAC_tell_cb(const FLAC__StreamDecoder*, FLAC__uint64 *absolute_byte_offset, void *client_data) {
    return ((SoundSourceFLAC*) client_data)->flacTell(absolute_byte_offset);
}

FLAC__StreamDecoderLengthStatus FLAC_length_cb(const FLAC__StreamDecoder*, FLAC__uint64 *stream_length, void *client_data) {
    return ((SoundSourceFLAC*) client_data)->flacLength(stream_length);
}

FLAC__bool FLAC_eof_cb(const FLAC__StreamDecoder*, void *client_data) {
    return ((SoundSourceFLAC*) client_data)->flacEOF();
}

FLAC__StreamDecoderWriteStatus FLAC_write_cb(const FLAC__StreamDecoder*, const FLAC__Frame *frame, const FLAC__int32 *const buffer[], void *client_data) {
    return ((SoundSourceFLAC*) client_data)->flacWrite(frame, buffer);
}

void FLAC_metadata_cb(const FLAC__StreamDecoder*, const FLAC__StreamMetadata *metadata, void *client_data) {
    ((SoundSourceFLAC*) client_data)->flacMetadata(metadata);
}

void FLAC_error_cb(const FLAC__StreamDecoder*, FLAC__StreamDecoderErrorStatus status, void *client_data) {
    ((SoundSourceFLAC*) client_data)->flacError(status);
}
// end callbacks
