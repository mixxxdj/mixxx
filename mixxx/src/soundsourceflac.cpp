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

#include <cstring> // memcpy
#include <QtDebug>
#include <taglib/flacfile.h>

#include "soundsourceflac.h"

SoundSourceFLAC::SoundSourceFLAC(QString filename)
    : Mixxx::SoundSource(filename)
    , m_file(filename)
    , m_decoder(NULL)
    , m_samples(0)
    , m_bps(0)
    , m_minBlocksize(0)
    , m_maxBlocksize(0)
    , m_minFramesize(0)
    , m_maxFramesize(0)
    , m_flacBuffer(NULL)
    , m_flacBufferLength(0)
    , m_leftoverBuffer(NULL)
    , m_leftoverBufferLength(0) {
}

SoundSourceFLAC::~SoundSourceFLAC() {
    if (m_flacBuffer != NULL) {
        delete [] m_flacBuffer;
        m_flacBuffer = NULL;
    }
    if (m_leftoverBuffer != NULL) {
        delete [] m_leftoverBuffer;
        m_leftoverBuffer = NULL;
    }
    if (m_decoder) {
        FLAC__stream_decoder_finish(m_decoder);
        FLAC__stream_decoder_delete(m_decoder); // frees memory
        m_decoder = NULL;
    }
}

// soundsource overrides
int SoundSourceFLAC::open() {
    m_file.open(QIODevice::ReadOnly);
    m_decoder = FLAC__stream_decoder_new();
    if (m_decoder == NULL) {
        qWarning() << "SSFLAC: decoder allocation failed!";
        return ERR;
    }
    FLAC__StreamDecoderInitStatus initStatus(
        FLAC__stream_decoder_init_stream(
            m_decoder, FLAC_read_cb, FLAC_seek_cb, FLAC_tell_cb, FLAC_length_cb,
            FLAC_eof_cb, FLAC_write_cb, FLAC_metadata_cb, FLAC_error_cb,
            (void*) this)
    );
    if (initStatus != FLAC__STREAM_DECODER_INIT_STATUS_OK) {
        qWarning() << "SSFLAC: decoder init failed!";
        goto decoderError;
    }
    if (!FLAC__stream_decoder_process_until_end_of_metadata(m_decoder)) {
        qWarning() << "SSFLAC: process to end of meta failed!";
        qWarning() << "SSFLAC: decoder state: " << FLAC__stream_decoder_get_state(m_decoder);
        goto decoderError;
    } // now number of samples etc. should be populated
    if (m_flacBuffer == NULL) {
        // we want 2 samples per frame, see ::flacWrite code -- bkgood
        m_flacBuffer = new FLAC__int16[m_maxBlocksize * 2 /*m_iChannels*/];
    }
    if (m_leftoverBuffer == NULL) {
        m_leftoverBuffer = new FLAC__int16[m_maxBlocksize * 2 /*m_iChannels*/];
    }
//    qDebug() << "SSFLAC: Total samples: " << m_samples;
//    qDebug() << "SSFLAC: Sampling rate: " << m_iSampleRate << " Hz";
//    qDebug() << "SSFLAC: Channels: " << m_iChannels;
//    qDebug() << "SSFLAC: BPS: " << m_bps;
    return OK;
decoderError:
    FLAC__stream_decoder_finish(m_decoder);
    FLAC__stream_decoder_delete(m_decoder);
    m_decoder = NULL;
    return ERR;
}

long SoundSourceFLAC::seek(long filepos) {
    if (!m_decoder) return 0;
    // important division here, filepos is in audio samples (i.e. shorts)
    // but libflac expects a number in time samples. I _think_ this should
    // be hard-coded at two because *2 is the assumption the caller makes
    // -- bkgood
    FLAC__stream_decoder_seek_absolute(m_decoder, filepos / 2);
    m_leftoverBufferLength = 0; // clear internal buffer since we moved
    return filepos;
}

unsigned int SoundSourceFLAC::read(unsigned long size, const SAMPLE *destination) {
    if (!m_decoder) return 0;
    SAMPLE *destBuffer(const_cast<SAMPLE*>(destination));
    unsigned int samplesWritten = 0;
    unsigned int i = 0;
    while (samplesWritten < size) {
        // if our buffer from libflac is empty (either because we explicitly cleared
        // it or because we've simply used all the samples), ask for a new buffer
        if (m_flacBufferLength == 0) {
            i = 0;
            if (!FLAC__stream_decoder_process_single(m_decoder)) {
                qWarning() << "SSFLAC: decoder_process_single returned false";
                break;
            } else if (m_flacBufferLength == 0) {
                // EOF
                break;
            }
        }
        destBuffer[samplesWritten++] = m_flacBuffer[i++];
        --m_flacBufferLength;
    }
    if (m_flacBufferLength != 0) {
        memcpy(m_leftoverBuffer, &m_flacBuffer[i],
                m_flacBufferLength * sizeof(m_flacBuffer[0])); // safe because leftoverBuffer
                                                               // is as long as flacbuffer
        memcpy(m_flacBuffer, m_leftoverBuffer,
                m_flacBufferLength * sizeof(m_leftoverBuffer[0]));
        // this whole if block could go away if this just used a ring buffer but I'd
        // rather do that after I've gotten off the inital happiness of getting this right,
        // if I see SIGSEGV one more time I'll pop -- bkgood
    }
    return samplesWritten;
}

inline unsigned long SoundSourceFLAC::length() {
    return m_samples * m_iChannels;
}

int SoundSourceFLAC::parseHeader() {
    setType("flac");
#ifdef __WINDOWS__
    /* From Tobias: A Utf-8 string did not work on my Windows XP (German edition)
     * If you try this conversion, f.isValid() will return false in many cases
     * and processTaglibFile() will fail
     *
     * The method toLocal8Bit() returns the local 8-bit representation of the string as a QByteArray.
     * The returned byte array is undefined if the string contains characters not supported
     * by the local 8-bit encoding.
     */
    QByteArray qBAFilename = m_qFilename.toLocal8Bit();
#else
    QByteArray qBAFilename = m_qFilename.toUtf8();
#endif
    TagLib::FLAC::File f(qBAFilename.constData());
    bool result = processTaglibFile(f);
    TagLib::ID3v2::Tag *id3v2(f.ID3v2Tag());
    TagLib::Ogg::XiphComment *xiph(f.xiphComment());
    if (id3v2) {
        processID3v2Tag(id3v2);
    }
    if (xiph) {
        processXiphComment(xiph);
    }
    return result ? OK : ERR;
}

/**
 * Shift needed to take our FLAC sample size to Mixxx's 16-bit samples.
 * Shift right on negative, left on positive.
 */
inline int SoundSourceFLAC::getShift() const {
    return 16 - m_bps;
}

/**
 * Shift a sample from FLAC as necessary to get a 16-bit value.
 */
inline FLAC__int16 SoundSourceFLAC::shift(FLAC__int32 sample) const {
    // this is how libsndfile does this operation and is wonderfully
    // straightforward. Just shift the sample left or right so that
    // it fits in a 16-bit short. -- bkgood
    int shift(getShift());
    if (shift == 0) {
        return sample;
    } else if (shift < 0) {
        return sample >> abs(shift);
    } else {
        return sample << shift;
    }
};

// static
QList<QString> SoundSourceFLAC::supportedFileExtensions() {
    QList<QString> list;
    list.push_back("flac");
    return list;
}


// flac callback methods
FLAC__StreamDecoderReadStatus SoundSourceFLAC::flacRead(FLAC__byte buffer[], size_t *bytes) {
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
    if (m_file.seek(offset)) {
        return FLAC__STREAM_DECODER_SEEK_STATUS_OK;
    } else {
        return FLAC__STREAM_DECODER_SEEK_STATUS_ERROR;
    }
}

FLAC__StreamDecoderTellStatus SoundSourceFLAC::flacTell(FLAC__uint64 *offset) {
    if (m_file.isSequential()) {
        return FLAC__STREAM_DECODER_TELL_STATUS_UNSUPPORTED;
    }
    *offset = m_file.pos();
    return FLAC__STREAM_DECODER_TELL_STATUS_OK;
}

FLAC__StreamDecoderLengthStatus SoundSourceFLAC::flacLength(FLAC__uint64 *length) {
    if (m_file.isSequential()) {
        return FLAC__STREAM_DECODER_LENGTH_STATUS_UNSUPPORTED;
    }
    *length = m_file.size();
    return FLAC__STREAM_DECODER_LENGTH_STATUS_OK;
}

FLAC__bool SoundSourceFLAC::flacEOF() {
    if (m_file.isSequential()) {
        return false;
    }
    return m_file.atEnd();
}

FLAC__StreamDecoderWriteStatus SoundSourceFLAC::flacWrite(const FLAC__Frame *frame,
        const FLAC__int32 *const buffer[]) {
    unsigned int i(0);
    m_flacBufferLength = 0;
    if (frame->header.channels > 1) {
        // stereo (or greater)
        for (i = 0; i < frame->header.blocksize; ++i) {
            m_flacBuffer[m_flacBufferLength++] = shift(buffer[0][i]); // left channel
            m_flacBuffer[m_flacBufferLength++] = shift(buffer[1][i]); // right channel
        }
    } else {
        // mono
        for (i = 0; i < frame->header.blocksize; ++i) {
            m_flacBuffer[m_flacBufferLength++] = shift(buffer[0][i]); // left channel
            m_flacBuffer[m_flacBufferLength++] = shift(buffer[0][i]); // mono channel
        }
    }
    return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE; // can't anticipate any errors here
}

void SoundSourceFLAC::flacMetadata(const FLAC__StreamMetadata *metadata) {
    switch (metadata->type) {
    case FLAC__METADATA_TYPE_STREAMINFO:
        m_samples = metadata->data.stream_info.total_samples;
        m_iChannels = metadata->data.stream_info.channels;
        m_iSampleRate = metadata->data.stream_info.sample_rate;
        m_bps = metadata->data.stream_info.bits_per_sample;
        m_minBlocksize = metadata->data.stream_info.min_blocksize;
        m_maxBlocksize = metadata->data.stream_info.max_blocksize;
        m_minFramesize = metadata->data.stream_info.min_framesize;
        m_maxFramesize = metadata->data.stream_info.max_framesize;
//        qDebug() << "FLAC file " << m_qFilename;
//        qDebug() << m_iChannels << " @ " << m_iSampleRate << " Hz, " << m_samples
//            << " total, " << m_bps << " bps";
//        qDebug() << "Blocksize in [" << m_minBlocksize << ", " << m_maxBlocksize
//            << "], Framesize in [" << m_minFramesize << ", " << m_maxFramesize << "]";
        break;
    default:
        break;
    }
}

void SoundSourceFLAC::flacError(FLAC__StreamDecoderErrorStatus status) {
    QString error;
    // not much can be done at this point -- luckly the decoder seems to be
    // pretty forgiving -- bkgood
    switch (status) {
    case FLAC__STREAM_DECODER_ERROR_STATUS_LOST_SYNC:
        error = "STREAM_DECODER_ERROR_STATUS_LOST_SYNC";
        break;
    case FLAC__STREAM_DECODER_ERROR_STATUS_BAD_HEADER:
        error = "STREAM_DECODER_ERROR_STATUS_BAD_HEADER";
        break;
    case FLAC__STREAM_DECODER_ERROR_STATUS_FRAME_CRC_MISMATCH:
        error = "STREAM_DECODER_ERROR_STATUS_FRAME_CRC_MISMATCH";
        break;
    case FLAC__STREAM_DECODER_ERROR_STATUS_UNPARSEABLE_STREAM:
        error = "STREAM_DECODER_ERROR_STATUS_UNPARSEABLE_STREAM";
        break;
    }
    qWarning() << "SSFLAC got error" << error << "from libFLAC for file"
        << m_file.fileName();
    // not much else to do here... whatever function that initiated whatever
    // decoder method resulted in this error will return an error, and the caller
    // will bail. libFLAC docs say to not close the decoder here -- bkgood
}

// begin callbacks (have to be regular functions because normal libFLAC isn't C++-aware)

FLAC__StreamDecoderReadStatus FLAC_read_cb(const FLAC__StreamDecoder*, FLAC__byte buffer[],
        size_t *bytes, void *client_data) {
    return ((SoundSourceFLAC*) client_data)->flacRead(buffer, bytes);
}

FLAC__StreamDecoderSeekStatus FLAC_seek_cb(const FLAC__StreamDecoder*,
        FLAC__uint64 absolute_byte_offset, void *client_data) {
    return ((SoundSourceFLAC*) client_data)->flacSeek(absolute_byte_offset);
}

FLAC__StreamDecoderTellStatus FLAC_tell_cb(const FLAC__StreamDecoder*,
        FLAC__uint64 *absolute_byte_offset, void *client_data) {
    return ((SoundSourceFLAC*) client_data)->flacTell(absolute_byte_offset);
}

FLAC__StreamDecoderLengthStatus FLAC_length_cb(const FLAC__StreamDecoder*,
        FLAC__uint64 *stream_length, void *client_data) {
    return ((SoundSourceFLAC*) client_data)->flacLength(stream_length);
}

FLAC__bool FLAC_eof_cb(const FLAC__StreamDecoder*, void *client_data) {
    return ((SoundSourceFLAC*) client_data)->flacEOF();
}

FLAC__StreamDecoderWriteStatus FLAC_write_cb(const FLAC__StreamDecoder*, const FLAC__Frame *frame,
        const FLAC__int32 *const buffer[], void *client_data) {
    return ((SoundSourceFLAC*) client_data)->flacWrite(frame, buffer);
}

void FLAC_metadata_cb(const FLAC__StreamDecoder*, const FLAC__StreamMetadata *metadata, void *client_data) {
    ((SoundSourceFLAC*) client_data)->flacMetadata(metadata);
}

void FLAC_error_cb(const FLAC__StreamDecoder*, FLAC__StreamDecoderErrorStatus status, void *client_data) {
    ((SoundSourceFLAC*) client_data)->flacError(status);
}
// end callbacks
