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

#include "soundsourceflac.h"
#include "soundsourcetaglib.h"
#include "sampleutil.h"
#include "util/math.h"

#include <taglib/flacfile.h>

#include <QtDebug>

SoundSourceFLAC::SoundSourceFLAC(QString filename)
        : Mixxx::SoundSource(filename), m_file(filename), m_decoder(NULL), m_minBlocksize(
                0), m_maxBlocksize(0), m_minFramesize(0), m_maxFramesize(0), m_sampleScale(
                kSampleValueZero), m_decodeSampleBufferReadOffset(0), m_decodeSampleBufferWriteOffset(
                0) {
    setType("flac");
}

SoundSourceFLAC::~SoundSourceFLAC() {
    closeThis();
}

// soundsource overrides
Result SoundSourceFLAC::open() {
    if (NULL != m_decoder) {
        qWarning() << "SSFLAC: Already open!";
        return ERR;
    }

    if (!m_file.open(QIODevice::ReadOnly)) {
        qWarning() << "SSFLAC: Could not read file!";
        return ERR;
    }

    m_decoder = FLAC__stream_decoder_new();
    if (m_decoder == NULL) {
        qWarning() << "SSFLAC: decoder allocation failed!";
        return ERR;
    }
    FLAC__stream_decoder_set_md5_checking(m_decoder, FALSE);
    FLAC__StreamDecoderInitStatus initStatus(
            FLAC__stream_decoder_init_stream(m_decoder, FLAC_read_cb,
                    FLAC_seek_cb, FLAC_tell_cb, FLAC_length_cb, FLAC_eof_cb,
                    FLAC_write_cb, FLAC_metadata_cb, FLAC_error_cb,
                    (void*) this));
    if (initStatus != FLAC__STREAM_DECODER_INIT_STATUS_OK) {
        qWarning() << "SSFLAC: decoder init failed!";
        goto decoderError;
    }
    if (!FLAC__stream_decoder_process_until_end_of_metadata(m_decoder)) {
        qWarning() << "SSFLAC: process to end of meta failed!";
        qWarning() << "SSFLAC: decoder state: "
                << FLAC__stream_decoder_get_state(m_decoder);
        goto decoderError;
    } // now number of samples etc. should be populated
    // we want kChannelCount samples per frame, see ::flacWrite code -- bkgood
    m_decodeSampleBufferReadOffset = 0;
    m_decodeSampleBufferWriteOffset = 0;
    m_decodeSampleBuffer.resize(m_maxBlocksize * getChannelCount());
//    qDebug() << "SSFLAC: Total samples: " << m_samples;
//    qDebug() << "SSFLAC: Sampling rate: " << m_iSampleRate << " Hz";
//    qDebug() << "SSFLAC: Channels: " << m_iChannels;
//    qDebug() << "SSFLAC: BPS: " << m_bps;
    return OK;

    decoderError: closeThis();

    qWarning() << "SSFLAC: Decoder error at file" << getFilename();
    return ERR;
}

void SoundSourceFLAC::closeThis() {
    if (m_decoder) {
        FLAC__stream_decoder_finish(m_decoder);
        FLAC__stream_decoder_delete(m_decoder); // frees memory
        m_decoder = NULL;
    }
    m_decodeSampleBuffer.clear();
}

void SoundSourceFLAC::close() {
    closeThis();
    Super::close();
}

Mixxx::AudioSource::diff_type SoundSourceFLAC::seekFrame(diff_type frameIndex) {
    // clear decode buffer before seeking
    m_decodeSampleBufferReadOffset = 0;
    m_decodeSampleBufferWriteOffset = 0;
    bool result = FLAC__stream_decoder_seek_absolute(m_decoder, frameIndex);
    if (!result) {
        qWarning() << "SSFLAC: Seeking error at file" << getFilename();
    }
    return frameIndex;
}

Mixxx::AudioSource::size_type SoundSourceFLAC::readFrameSamplesInterleaved(
        size_type frameCount, sample_type* sampleBuffer) {
    return readFrameSamplesInterleaved(frameCount, sampleBuffer, false);
}

Mixxx::AudioSource::size_type SoundSourceFLAC::readStereoFrameSamplesInterleaved(
        size_type frameCount, sample_type* sampleBuffer) {
    return readFrameSamplesInterleaved(frameCount, sampleBuffer, true);
}

Mixxx::AudioSource::size_type SoundSourceFLAC::readFrameSamplesInterleaved(
        size_type frameCount, sample_type* sampleBuffer,
        bool readStereoSamples) {
    sample_type* outBuffer = sampleBuffer;
    size_type framesRemaining = frameCount;
    while (0 < framesRemaining) {
        Q_ASSERT(
                m_decodeSampleBufferReadOffset
                        <= m_decodeSampleBufferWriteOffset);
        // if our buffer from libflac is empty (either because we explicitly cleared
        // it or because we've simply used all the samples), ask for a new buffer
        if (m_decodeSampleBufferReadOffset >= m_decodeSampleBufferWriteOffset) {
            m_decodeSampleBufferReadOffset = 0;
            m_decodeSampleBufferWriteOffset = 0;
            if (FLAC__stream_decoder_process_single(m_decoder)) {
                if (m_decodeSampleBufferReadOffset
                        >= m_decodeSampleBufferWriteOffset) {
                    // EOF
                    break;
                }
            } else {
                qWarning() << "SSFLAC: decoder_process_single returned false ("
                        << getFilename() << ")";
                break;
            }
        }
        Q_ASSERT(
                m_decodeSampleBufferReadOffset
                        <= m_decodeSampleBufferWriteOffset);
        const size_type decodeBufferSamples = m_decodeSampleBufferWriteOffset
                - m_decodeSampleBufferReadOffset;
        const size_type decodeBufferFrames = samples2frames(
                decodeBufferSamples);
        const size_type framesToCopy = math_min(framesRemaining, decodeBufferFrames);
        const size_type samplesToCopy = frames2samples(framesToCopy);
        if (readStereoSamples && !isChannelCountStereo()) {
            if (isChannelCountMono()) {
                SampleUtil::copyMonoToDualMono(outBuffer,
                        &m_decodeSampleBuffer[m_decodeSampleBufferReadOffset],
                        samplesToCopy);
            } else {
                SampleUtil::copyMultiToStereo(outBuffer,
                        &m_decodeSampleBuffer[m_decodeSampleBufferReadOffset],
                        samplesToCopy, getChannelCount());
            }
            outBuffer += framesToCopy * 2; // copied 2 samples per frame
        } else {
            SampleUtil::copy(outBuffer,
                    &m_decodeSampleBuffer[m_decodeSampleBufferReadOffset],
                    samplesToCopy);
            outBuffer += samplesToCopy;
        }
        if (isChannelCountMono() && readStereoSamples) {
        } else {
        }
        m_decodeSampleBufferReadOffset += samplesToCopy;
        framesRemaining -= framesToCopy;
        Q_ASSERT(
                m_decodeSampleBufferReadOffset
                        <= m_decodeSampleBufferWriteOffset);
    }
    return frameCount - framesRemaining;
}

Result SoundSourceFLAC::parseHeader() {
    const QByteArray qBAFilename(getFilename().toLocal8Bit());
    TagLib::FLAC::File f(qBAFilename.constData());

    if (!readFileHeader(this, f)) {
        return ERR;
    }

    TagLib::Ogg::XiphComment *xiph(f.xiphComment());
    if (xiph) {
        readXiphComment(this, *xiph);
    } else {
        TagLib::ID3v2::Tag *id3v2(f.ID3v2Tag());
        if (id3v2) {
            readID3v2Tag(this, *id3v2);
        } else {
            // fallback
            const TagLib::Tag *tag(f.tag());
            if (tag) {
                readTag(this, *tag);
            } else {
                return ERR;
            }
        }
    }

    return OK;
}

QImage SoundSourceFLAC::parseCoverArt() {
    const QByteArray qBAFilename(getFilename().toLocal8Bit());
    TagLib::FLAC::File f(qBAFilename.constData());
    QImage coverArt;
    TagLib::Ogg::XiphComment *xiph(f.xiphComment());
    if (xiph) {
        coverArt = Mixxx::getCoverInXiphComment(*xiph);
    }
    if (coverArt.isNull()) {
        TagLib::ID3v2::Tag *id3v2(f.ID3v2Tag());
        if (id3v2) {
            coverArt = Mixxx::getCoverInID3v2Tag(*id3v2);
        }
        if (coverArt.isNull()) {
            TagLib::List<TagLib::FLAC::Picture*> covers = f.pictureList();
            if (!covers.isEmpty()) {
                std::list<TagLib::FLAC::Picture*>::iterator it = covers.begin();
                TagLib::FLAC::Picture* cover = *it;
                coverArt = QImage::fromData(
                        QByteArray(cover->data().data(), cover->data().size()));
            }
        }
    }
    return coverArt;
}

// static
QList<QString> SoundSourceFLAC::supportedFileExtensions() {
    QList<QString> list;
    list.push_back("flac");
    return list;
}

// flac callback methods
FLAC__StreamDecoderReadStatus SoundSourceFLAC::flacRead(FLAC__byte buffer[],
        size_t *bytes) {
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
        qWarning() << "SSFLAC: An unrecoverable error occurred ("
                << getFilename() << ")";
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

FLAC__StreamDecoderLengthStatus SoundSourceFLAC::flacLength(
        FLAC__uint64 *length) {
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

FLAC__StreamDecoderWriteStatus SoundSourceFLAC::flacWrite(
        const FLAC__Frame *frame, const FLAC__int32 * const buffer[]) {
    if (getChannelCount() != frame->header.channels) {
        qWarning() << "Invalid number of channels in FLAC frame header:"
                << "expected" << getChannelCount() << "actual"
                << frame->header.channels;
        return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
    }
    Q_ASSERT(m_decodeSampleBufferReadOffset <= m_decodeSampleBufferWriteOffset);
    Q_ASSERT(
            (m_decodeSampleBuffer.size() - m_decodeSampleBufferWriteOffset)
                    >= frames2samples(frame->header.blocksize));
    switch (getChannelCount()) {
    case 1:
    {
        // optimized code for 1 channel (mono)
        Q_ASSERT(1 <= frame->header.channels);
        for (unsigned i = 0; i < frame->header.blocksize; ++i) {
            m_decodeSampleBuffer[m_decodeSampleBufferWriteOffset++] = buffer[0][i]
                    * m_sampleScale;
        }
        break;
    }
    case 2:
    {
        // optimized code for 2 channels (stereo)
        Q_ASSERT(2 <= frame->header.channels);
        for (unsigned i = 0; i < frame->header.blocksize; ++i) {
            m_decodeSampleBuffer[m_decodeSampleBufferWriteOffset++] = buffer[0][i]
                    * m_sampleScale;
            m_decodeSampleBuffer[m_decodeSampleBufferWriteOffset++] = buffer[1][i]
                    * m_sampleScale;
        }
        break;
    }
    default:
    {
        // generic code for multiple channels
        for (unsigned i = 0; i < frame->header.blocksize; ++i) {
            for (unsigned j = 0; j < frame->header.channels; ++j) {
                m_decodeSampleBuffer[m_decodeSampleBufferWriteOffset++] = buffer[j][i]
                        * m_sampleScale;
            }
        }
    }
    }
    Q_ASSERT(m_decodeSampleBufferReadOffset <= m_decodeSampleBufferWriteOffset);
    return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

void SoundSourceFLAC::flacMetadata(const FLAC__StreamMetadata *metadata) {
    switch (metadata->type) {
    case FLAC__METADATA_TYPE_STREAMINFO:
        setChannelCount(metadata->data.stream_info.channels);
        setSampleRate(metadata->data.stream_info.sample_rate);
        setFrameCount(metadata->data.stream_info.total_samples);
        m_sampleScale = kSampleValuePeak / sample_type(FLAC__int32(1) << metadata->data.stream_info.bits_per_sample);
        qDebug() << "FLAC file " << getFilename();
        qDebug() << getChannelCount() << " @ "
        << getSampleRate() << " Hz, "
        << getFrameCount()<< " total, "
        << "bit depth" << " metadata->data.stream_info.bits_per_sample";
        m_minBlocksize = metadata->data.stream_info.min_blocksize;
        m_maxBlocksize = metadata->data.stream_info.max_blocksize;
        m_minFramesize = metadata->data.stream_info.min_framesize;
        m_maxFramesize = metadata->data.stream_info.max_framesize;
        qDebug() << "Blocksize in [" << m_minBlocksize << ", " << m_maxBlocksize
        << "], Framesize in [" << m_minFramesize << ", " << m_maxFramesize << "]";
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
            << getFilename();
    // not much else to do here... whatever function that initiated whatever
    // decoder method resulted in this error will return an error, and the caller
    // will bail. libFLAC docs say to not close the decoder here -- bkgood
}

// begin callbacks (have to be regular functions because normal libFLAC isn't C++-aware)

FLAC__StreamDecoderReadStatus FLAC_read_cb(const FLAC__StreamDecoder*,
        FLAC__byte buffer[], size_t *bytes, void *client_data) {
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

FLAC__StreamDecoderWriteStatus FLAC_write_cb(const FLAC__StreamDecoder*,
        const FLAC__Frame *frame, const FLAC__int32 * const buffer[],
        void *client_data) {
    return ((SoundSourceFLAC*) client_data)->flacWrite(frame, buffer);
}

void FLAC_metadata_cb(const FLAC__StreamDecoder*,
        const FLAC__StreamMetadata *metadata, void *client_data) {
    ((SoundSourceFLAC*) client_data)->flacMetadata(metadata);
}

void FLAC_error_cb(const FLAC__StreamDecoder*,
        FLAC__StreamDecoderErrorStatus status, void *client_data) {
    ((SoundSourceFLAC*) client_data)->flacError(status);
}
// end callbacks
