#include "sources/soundsourceflac.h"

#include "sampleutil.h"
#include "util/math.h"

namespace Mixxx {

namespace {

// begin callbacks (have to be regular functions because normal libFLAC isn't C++-aware)

FLAC__StreamDecoderReadStatus FLAC_read_cb(const FLAC__StreamDecoder*,
        FLAC__byte buffer[], size_t* bytes, void* client_data) {
    return static_cast<SoundSourceFLAC*>(client_data)->flacRead(buffer, bytes);
}

FLAC__StreamDecoderSeekStatus FLAC_seek_cb(const FLAC__StreamDecoder*,
        FLAC__uint64 absolute_byte_offset, void* client_data) {
    return static_cast<SoundSourceFLAC*>(client_data)->flacSeek(
            absolute_byte_offset);
}

FLAC__StreamDecoderTellStatus FLAC_tell_cb(const FLAC__StreamDecoder*,
        FLAC__uint64 *absolute_byte_offset, void* client_data) {
    return static_cast<SoundSourceFLAC*>(client_data)->flacTell(
            absolute_byte_offset);
}

FLAC__StreamDecoderLengthStatus FLAC_length_cb(const FLAC__StreamDecoder*,
        FLAC__uint64 *stream_length, void* client_data) {
    return static_cast<SoundSourceFLAC*>(client_data)->flacLength(stream_length);
}

FLAC__bool FLAC_eof_cb(const FLAC__StreamDecoder*, void* client_data) {
    return static_cast<SoundSourceFLAC*>(client_data)->flacEOF();
}

FLAC__StreamDecoderWriteStatus FLAC_write_cb(const FLAC__StreamDecoder*,
        const FLAC__Frame* frame, const FLAC__int32* const buffer[],
        void* client_data) {
    return static_cast<SoundSourceFLAC*>(client_data)->flacWrite(frame, buffer);
}

void FLAC_metadata_cb(const FLAC__StreamDecoder*,
        const FLAC__StreamMetadata* metadata, void* client_data) {
    static_cast<SoundSourceFLAC*>(client_data)->flacMetadata(metadata);
}

void FLAC_error_cb(const FLAC__StreamDecoder*,
        FLAC__StreamDecoderErrorStatus status, void* client_data) {
    static_cast<SoundSourceFLAC*>(client_data)->flacError(status);
}

// end callbacks

const unsigned kBitsPerSampleDefault = 0;

}

QList<QString> SoundSourceFLAC::supportedFileExtensions() {
    QList<QString> list;
    list.push_back("flac");
    return list;
}

SoundSourceFLAC::SoundSourceFLAC(QUrl url)
        : SoundSource(url, "flac"),
          m_file(getLocalFileName()),
          m_decoder(NULL),
          m_minBlocksize(0),
          m_maxBlocksize(0),
          m_minFramesize(0),
          m_maxFramesize(0),
          m_bitsPerSample(kBitsPerSampleDefault),
          m_sampleScaleFactor(kSampleValueZero),
          m_decodeSampleBufferReadOffset(0),
          m_decodeSampleBufferWriteOffset(0),
          m_curFrameIndex(kFrameIndexMin) {
}

SoundSourceFLAC::~SoundSourceFLAC() {
    close();
}

Result SoundSourceFLAC::tryOpen(SINT /*channelCountHint*/) {
    DEBUG_ASSERT(!m_file.isOpen());
    if (!m_file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open FLAC file:" << m_file.fileName();
        return ERR;
    }

    m_decoder = FLAC__stream_decoder_new();
    if (m_decoder == NULL) {
        qWarning() << "Failed to create FLAC decoder!";
        return ERR;
    }
    FLAC__stream_decoder_set_md5_checking(m_decoder, FALSE);
    const FLAC__StreamDecoderInitStatus initStatus(
            FLAC__stream_decoder_init_stream(m_decoder, FLAC_read_cb,
                    FLAC_seek_cb, FLAC_tell_cb, FLAC_length_cb, FLAC_eof_cb,
                    FLAC_write_cb, FLAC_metadata_cb, FLAC_error_cb, this));
    if (initStatus != FLAC__STREAM_DECODER_INIT_STATUS_OK) {
        qWarning() << "Failed to initialize FLAC decoder:" << initStatus;
        return ERR;
    }
    if (!FLAC__stream_decoder_process_until_end_of_metadata(m_decoder)) {
        qWarning() << "Failed to process FLAC metadata:"
                << FLAC__stream_decoder_get_state(m_decoder);
        return ERR;
    }

    m_curFrameIndex = kFrameIndexMin;

    return OK;
}

void SoundSourceFLAC::close() {
    if (m_decoder) {
        FLAC__stream_decoder_finish(m_decoder);
        FLAC__stream_decoder_delete(m_decoder); // frees memory
        m_decoder = NULL;
    }

    m_file.close();
}

SINT SoundSourceFLAC::seekSampleFrame(SINT frameIndex) {
    DEBUG_ASSERT(isValidFrameIndex(m_curFrameIndex));
    DEBUG_ASSERT(isValidFrameIndex(frameIndex));

    // clear decode buffer before seeking
    m_decodeSampleBufferReadOffset = 0;
    m_decodeSampleBufferWriteOffset = 0;
    if (!FLAC__stream_decoder_seek_absolute(m_decoder, frameIndex)) {
        qWarning() << "SSFLAC: Seeking error at file" << m_file.fileName();
    }
    if ((FLAC__STREAM_DECODER_SEEK_ERROR == FLAC__stream_decoder_get_state(m_decoder)) &&
        !FLAC__stream_decoder_flush(m_decoder)) {
        qWarning() << "SSFLAC: Failed to flush the decoder's input buffer after seeking" << m_file.fileName();
    }
    m_curFrameIndex = frameIndex;

    DEBUG_ASSERT(isValidFrameIndex(m_curFrameIndex));
    return m_curFrameIndex;
}

SINT SoundSourceFLAC::readSampleFrames(
        SINT numberOfFrames, CSAMPLE* sampleBuffer) {
    return readSampleFrames(numberOfFrames, sampleBuffer,
            frames2samples(numberOfFrames), false);
}

SINT SoundSourceFLAC::readSampleFramesStereo(
        SINT numberOfFrames, CSAMPLE* sampleBuffer,
        SINT sampleBufferSize) {
    return readSampleFrames(numberOfFrames, sampleBuffer, sampleBufferSize,
            true);
}

SINT SoundSourceFLAC::readSampleFrames(
        SINT numberOfFrames, CSAMPLE* sampleBuffer,
        SINT sampleBufferSize, bool readStereoSamples) {
    DEBUG_ASSERT(isValidFrameIndex(m_curFrameIndex));
    DEBUG_ASSERT(getSampleBufferSize(numberOfFrames, readStereoSamples) <= sampleBufferSize);

    const SINT numberOfFramesTotal = numberOfFrames;

    CSAMPLE* outBuffer = sampleBuffer;
    SINT numberOfFramesRemaining = numberOfFramesTotal;
    while (0 < numberOfFramesRemaining) {
        DEBUG_ASSERT(
                m_decodeSampleBufferReadOffset <= m_decodeSampleBufferWriteOffset);
        // if our buffer from libflac is empty (either because we explicitly cleared
        // it or because we've simply used all the samples), ask for a new buffer
        if (m_decodeSampleBufferReadOffset >= m_decodeSampleBufferWriteOffset) {
            // Documentation of FLAC__stream_decoder_process_single():
            // "Depending on what was decoded, the metadata or write callback
            // will be called with the decoded metadata block or audio frame."
            // See also: https://xiph.org/flac/api/group__flac__stream__decoder.html#ga9d6df4a39892c05955122cf7f987f856
            if (FLAC__stream_decoder_process_single(m_decoder)) {
                if (m_decodeSampleBufferReadOffset
                        >= m_decodeSampleBufferWriteOffset) {
                    // EOF
                    break;
                }
            } else {
                qWarning() << "SSFLAC: decoder_process_single returned false ("
                        << m_file.fileName() << ")";
                break;
            }
        }
        DEBUG_ASSERT(
                m_decodeSampleBufferReadOffset <= m_decodeSampleBufferWriteOffset);
        const SINT decodeBufferSamples = m_decodeSampleBufferWriteOffset
                - m_decodeSampleBufferReadOffset;
        const SINT decodeBufferFrames = samples2frames(
                decodeBufferSamples);
        const SINT framesToCopy =
                math_min(decodeBufferFrames, numberOfFramesRemaining);
        const SINT samplesToCopy = frames2samples(framesToCopy);
        if (readStereoSamples && !isChannelCountStereo()) {
            if (isChannelCountMono()) {
                SampleUtil::copyMonoToDualMono(outBuffer,
                        &m_decodeSampleBuffer[m_decodeSampleBufferReadOffset],
                        framesToCopy);
            } else {
                SampleUtil::copyMultiToStereo(outBuffer,
                        &m_decodeSampleBuffer[m_decodeSampleBufferReadOffset],
                        framesToCopy, getChannelCount());
            }
            outBuffer += framesToCopy * 2; // copied 2 samples per frame
        } else {
            SampleUtil::copy(outBuffer,
                    &m_decodeSampleBuffer[m_decodeSampleBufferReadOffset],
                    samplesToCopy);
            outBuffer += samplesToCopy;
        }
        m_decodeSampleBufferReadOffset += samplesToCopy;
        m_curFrameIndex += framesToCopy;
        numberOfFramesRemaining -= framesToCopy;
        DEBUG_ASSERT(
                m_decodeSampleBufferReadOffset <= m_decodeSampleBufferWriteOffset);
    }

    DEBUG_ASSERT(isValidFrameIndex(m_curFrameIndex));
    DEBUG_ASSERT(numberOfFramesTotal >= numberOfFramesRemaining);
    return numberOfFramesTotal - numberOfFramesRemaining;
}

// flac callback methods
FLAC__StreamDecoderReadStatus SoundSourceFLAC::flacRead(FLAC__byte buffer[],
        size_t* bytes) {
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
                << m_file.fileName() << ")";
        return FLAC__STREAM_DECODER_SEEK_STATUS_ERROR;
    }
}

FLAC__StreamDecoderTellStatus SoundSourceFLAC::flacTell(FLAC__uint64* offset) {
    if (m_file.isSequential()) {
        return FLAC__STREAM_DECODER_TELL_STATUS_UNSUPPORTED;
    }
    *offset = m_file.pos();
    return FLAC__STREAM_DECODER_TELL_STATUS_OK;
}

FLAC__StreamDecoderLengthStatus SoundSourceFLAC::flacLength(
        FLAC__uint64* length) {
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
        const FLAC__Frame* frame, const FLAC__int32* const buffer[]) {
    // decode buffer must be empty before decoding the next frame
    DEBUG_ASSERT(m_decodeSampleBufferReadOffset >= m_decodeSampleBufferWriteOffset);
    // reset decode buffer
    m_decodeSampleBufferReadOffset = 0;
    m_decodeSampleBufferWriteOffset = 0;
    if (getChannelCount() != frame->header.channels) {
        qWarning() << "Corrupt or unsupported FLAC file:"
                << "Invalid number of channels in FLAC frame header"
                << frame->header.channels << "<>" << getChannelCount();
        return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
    }
    if (getFrameRate() != frame->header.sample_rate) {
        qWarning() << "Corrupt or unsupported FLAC file:"
                << "Invalid sample rate in FLAC frame header"
                << frame->header.sample_rate << "<>" << getFrameRate();
        return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
    }
    const unsigned maxBlocksize = samples2frames(
            m_decodeSampleBuffer.size());
    if (maxBlocksize < frame->header.blocksize) {
        qWarning() << "Corrupt or unsupported FLAC file:"
                << "Block size in FLAC frame header exceeds the maximum block size"
                << frame->header.blocksize << ">" << maxBlocksize;
        return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
    }
    switch (getChannelCount()) {
    case 1: {
        // optimized code for 1 channel (mono)
        DEBUG_ASSERT(1 <= frame->header.channels);
        for (unsigned i = 0; i < frame->header.blocksize; ++i) {
            m_decodeSampleBuffer[m_decodeSampleBufferWriteOffset++] =
                    buffer[0][i] * m_sampleScaleFactor;
        }
        break;
    }
    case 2: {
        // optimized code for 2 channels (stereo)
        DEBUG_ASSERT(2 <= frame->header.channels);
        for (unsigned i = 0; i < frame->header.blocksize; ++i) {
            m_decodeSampleBuffer[m_decodeSampleBufferWriteOffset++] =
                    buffer[0][i] * m_sampleScaleFactor;
            m_decodeSampleBuffer[m_decodeSampleBufferWriteOffset++] =
                    buffer[1][i] * m_sampleScaleFactor;
        }
        break;
    }
    default: {
        // generic code for multiple channels
        DEBUG_ASSERT(getChannelCount() == frame->header.channels);
        for (unsigned i = 0; i < frame->header.blocksize; ++i) {
            for (unsigned j = 0; j < frame->header.channels; ++j) {
                m_decodeSampleBuffer[m_decodeSampleBufferWriteOffset++] =
                        buffer[j][i] * m_sampleScaleFactor;
            }
        }
    }
    }
    DEBUG_ASSERT(m_decodeSampleBufferReadOffset <= m_decodeSampleBufferWriteOffset);
    return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

void SoundSourceFLAC::flacMetadata(const FLAC__StreamMetadata* metadata) {
    // https://xiph.org/flac/api/group__flac__stream__decoder.html#ga43e2329c15731c002ac4182a47990f85
    // "...one STREAMINFO block, followed by zero or more other metadata blocks."
    // "...by default the decoder only calls the metadata callback for the STREAMINFO block..."
    // "...always before the first audio frame (i.e. write callback)."
    switch (metadata->type) {
    case FLAC__METADATA_TYPE_STREAMINFO:
    {
        const SINT channelCount = metadata->data.stream_info.channels;
        DEBUG_ASSERT(kChannelCountDefault != channelCount);
        if (getChannelCount() == kChannelCountDefault) {
            // not set before
            setChannelCount(channelCount);
        } else {
            // already set before -> check for consistency
            if (getChannelCount() != channelCount) {
                qWarning() << "Unexpected channel count:"
                        << channelCount << " <> " << getChannelCount();
            }
        }
        const SINT frameRate = metadata->data.stream_info.sample_rate;
        DEBUG_ASSERT(kFrameRateDefault != frameRate);
        if (getFrameRate() == kFrameRateDefault) {
            // not set before
            setFrameRate(frameRate);
        } else {
            // already set before -> check for consistency
            if (getFrameRate() != frameRate) {
                qWarning() << "Unexpected frame/sample rate:"
                        << frameRate << " <> " << getFrameRate();
            }
        }
        const SINT frameCount = metadata->data.stream_info.total_samples;
        DEBUG_ASSERT(kFrameCountDefault != frameCount);
        if (getFrameCount() == kFrameCountDefault) {
            // not set before
            setFrameCount(frameCount);
        } else {
            // already set before -> check for consistency
            if (getFrameCount() != frameCount) {
                qWarning() << "Unexpected frame count:"
                        << frameCount << " <> " << getFrameCount();
            }
        }
        const unsigned bitsPerSample = metadata->data.stream_info.bits_per_sample;
        DEBUG_ASSERT(kBitsPerSampleDefault != bitsPerSample);
        if (kBitsPerSampleDefault == m_bitsPerSample) {
            // not set before
            m_bitsPerSample = bitsPerSample;
            m_sampleScaleFactor = kSampleValuePeak
                    / CSAMPLE(FLAC__int32(1) << bitsPerSample);
        } else {
            // already set before -> check for consistency
            if (bitsPerSample != m_bitsPerSample) {
                qWarning() << "Unexpected bits per sample:"
                        << bitsPerSample << " <> " << m_bitsPerSample;
            }
        }
        m_minBlocksize = metadata->data.stream_info.min_blocksize;
        m_maxBlocksize = metadata->data.stream_info.max_blocksize;
        m_minFramesize = metadata->data.stream_info.min_framesize;
        m_maxFramesize = metadata->data.stream_info.max_framesize;
        m_decodeSampleBufferReadOffset = 0;
        m_decodeSampleBufferWriteOffset = 0;
        const unsigned decodeSampleBufferSize =
                m_maxBlocksize * getChannelCount();
        SampleBuffer(decodeSampleBufferSize).swap(m_decodeSampleBuffer);
        break;
    }
    default:
        // Ignore all other metadata types
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

} // namespace Mixxx
