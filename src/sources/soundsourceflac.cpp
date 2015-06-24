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

SoundSourceFLAC::SoundSourceFLAC(QUrl url)
        : SoundSource(url, "flac"),
          m_file(getLocalFileName()),
          m_decoder(NULL),
          m_minBlocksize(0),
          m_maxBlocksize(0),
          m_minFramesize(0),
          m_maxFramesize(0),
          m_bitsPerSample(kBitsPerSampleDefault),
          m_sampleScaleFactor(CSAMPLE_ZERO),
          m_curFrameIndex(getMinFrameIndex()) {
}

SoundSourceFLAC::~SoundSourceFLAC() {
    close();
}

Result SoundSourceFLAC::tryOpen(const AudioSourceConfig& /*audioSrcCfg*/) {
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
    FLAC__stream_decoder_set_md5_checking(m_decoder, false);
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

    m_curFrameIndex = getMinFrameIndex();

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

    // Avoid unnecessary seeking
    // NOTE(uklotzde): Disabling this optimization might reveal rare
    // seek errors on certain FLAC files were the decoder loses sync!
    if (m_curFrameIndex == frameIndex) {
        return m_curFrameIndex;
    }

    // Discard decoded sample data before seeking
    m_sampleBuffer.reset();

    // Seek to the new position
    if (FLAC__stream_decoder_seek_absolute(m_decoder, frameIndex)) {
        // Set the new position
        m_curFrameIndex = frameIndex;
        DEBUG_ASSERT(FLAC__STREAM_DECODER_SEEK_ERROR != FLAC__stream_decoder_get_state(m_decoder));
    } else {
        qWarning() << "Seek error at" << frameIndex << "in" << m_file.fileName();
        // Invalidate the current position
        m_curFrameIndex = getMaxFrameIndex();
        if (FLAC__STREAM_DECODER_SEEK_ERROR == FLAC__stream_decoder_get_state(m_decoder)) {
            // Flush the input stream of the decoder according to the
            // documentation of FLAC__stream_decoder_seek_absolute()
            if (!FLAC__stream_decoder_flush(m_decoder)) {
                qWarning() << "Failed to flush input buffer of the FLAC decoder after seeking in"
                        << m_file.fileName();
                // Invalidate the current position...
                m_curFrameIndex = getMaxFrameIndex();
                // ...and abort
                return m_curFrameIndex;
            }
            // Discard previously decoded sample data before decoding
            // the next block of samples
            m_sampleBuffer.reset();
            // Trigger decoding of the next block to update the current position
            if (!FLAC__stream_decoder_process_single(m_decoder)) {
                qWarning() << "Failed to resync FLAC decoder after seeking in"
                        << m_file.fileName();
                // Invalidate the current position...
                m_curFrameIndex = getMaxFrameIndex();
                // ...and abort
                return m_curFrameIndex;
            }
            DEBUG_ASSERT(isValidFrameIndex(m_curFrameIndex));
            if (m_curFrameIndex < frameIndex) {
                // Adjust the current position
                skipSampleFrames(frameIndex - m_curFrameIndex);
            }
        }
    }

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

    const SINT numberOfFramesTotal =
            math_min(numberOfFrames, getMaxFrameIndex() - m_curFrameIndex);
    const SINT numberOfSamplesTotal = frames2samples(numberOfFramesTotal);

    CSAMPLE* outBuffer = sampleBuffer;
    SINT numberOfSamplesRemaining = numberOfSamplesTotal;
    while (0 < numberOfSamplesRemaining) {
        // If our buffer from libflac is empty (either because we explicitly cleared
        // it or because we've simply used all the samples), ask for a new buffer
        if (m_sampleBuffer.isEmpty()) {
            // Save the current frame index
            const SINT curFrameIndexBeforeProcessing = m_curFrameIndex;
            // Documentation of FLAC__stream_decoder_process_single():
            // "Depending on what was decoded, the metadata or write callback
            // will be called with the decoded metadata block or audio frame."
            // See also: https://xiph.org/flac/api/group__flac__stream__decoder.html#ga9d6df4a39892c05955122cf7f987f856
            if (!FLAC__stream_decoder_process_single(m_decoder)) {
                qWarning() << "Failed to decode FLAC file"
                        << m_file.fileName();
                break; // abort
            }
            // After seeking we might need to skip some samples if the decoder
            // complained that it has lost sync for some malformed(?) files
            if (curFrameIndexBeforeProcessing != m_curFrameIndex) {
                if (curFrameIndexBeforeProcessing > m_curFrameIndex) {
                    qWarning() << "Trying to adjust frame index"
                            << m_curFrameIndex << "<>" << curFrameIndexBeforeProcessing
                            << "while decoding FLAC file"
                            << m_file.fileName();
                    skipSampleFrames(curFrameIndexBeforeProcessing - m_curFrameIndex);
                } else {
                    qWarning() << "Unexpected frame index"
                            << m_curFrameIndex << "<>" << curFrameIndexBeforeProcessing
                            << "while decoding FLAC file"
                            << m_file.fileName();
                    break; // abort
                }
            }
            DEBUG_ASSERT(curFrameIndexBeforeProcessing == m_curFrameIndex);
        }
        if (m_sampleBuffer.isEmpty()) {
            break; // EOF
        }

        const SampleBuffer::ReadableChunk readableChunk(
                m_sampleBuffer.readFromHead(numberOfSamplesRemaining));
        const SINT framesToCopy = samples2frames(readableChunk.size());
        if (outBuffer) {
            if (readStereoSamples && (kChannelCountStereo != getChannelCount())) {
                if (kChannelCountMono == getChannelCount()) {
                    SampleUtil::copyMonoToDualMono(outBuffer,
                            readableChunk.data(),
                            framesToCopy);
                } else {
                    SampleUtil::copyMultiToStereo(outBuffer,
                            readableChunk.data(),
                            framesToCopy, getChannelCount());
                }
                outBuffer += framesToCopy * kChannelCountStereo;
            } else {
                SampleUtil::copy(outBuffer, readableChunk.data(), readableChunk.size());
                outBuffer += readableChunk.size();
            }
        }
        m_curFrameIndex += framesToCopy;
        numberOfSamplesRemaining -= readableChunk.size();
    }

    DEBUG_ASSERT(isValidFrameIndex(m_curFrameIndex));
    DEBUG_ASSERT(numberOfSamplesTotal >= numberOfSamplesRemaining);
    return samples2frames(numberOfSamplesTotal - numberOfSamplesRemaining);
}

// flac callback methods
FLAC__StreamDecoderReadStatus SoundSourceFLAC::flacRead(FLAC__byte buffer[],
        size_t* bytes) {
    const qint64 maxlen = *bytes;
    if (0 >= maxlen) {
        *bytes = 0;
        return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
    }

    const qint64 readlen = m_file.read((char*) buffer, maxlen);

    if (0 < readlen) {
        *bytes = readlen;
        return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
    } else {
        *bytes = 0;
        if (0 == readlen) {
            return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
        } else {
            return FLAC__STREAM_DECODER_READ_STATUS_ABORT;
        }
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
    const SINT numChannels = frame->header.channels;
    if (getChannelCount() > numChannels) {
        qWarning() << "Corrupt or unsupported FLAC file:"
                << "Invalid number of channels in FLAC frame header"
                << frame->header.channels << "<>" << getChannelCount();
        return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
    }
    if (getFrameRate() != SINT(frame->header.sample_rate)) {
        qWarning() << "Corrupt or unsupported FLAC file:"
                << "Invalid sample rate in FLAC frame header"
                << frame->header.sample_rate << "<>" << getFrameRate();
        return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
    }
    const SINT numReadableFrames = frame->header.blocksize;
    if (numReadableFrames > m_maxBlocksize) {
        qWarning() << "Corrupt or unsupported FLAC file:"
                << "Block size in FLAC frame header exceeds the maximum block size"
                << frame->header.blocksize << ">" << m_maxBlocksize;
        return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
    }

    // According to the API docs the decoder will always report the current
    // position in "FLAC samples" (= "Mixxx frames") for convenience
    DEBUG_ASSERT(frame->header.number_type == FLAC__FRAME_NUMBER_TYPE_SAMPLE_NUMBER);
    m_curFrameIndex = frame->header.number.sample_number;

    // Decode buffer should be empty before decoding the next frame
    DEBUG_ASSERT(m_sampleBuffer.isEmpty());
    const SampleBuffer::WritableChunk writableChunk(
            m_sampleBuffer.writeToTail(frames2samples(numReadableFrames)));

    const SINT numWritableFrames = samples2frames(writableChunk.size());
    DEBUG_ASSERT(numWritableFrames <= numReadableFrames);
    if (numWritableFrames < numReadableFrames) {
        qWarning() << "Sample buffer has not enough free space for all decoded FLAC samples:"
                << numWritableFrames << "<" << numReadableFrames;
    }

    CSAMPLE* pSampleBuffer = writableChunk.data();
    DEBUG_ASSERT(getChannelCount() <= numChannels);
    switch (getChannelCount()) {
    case 1: {
        // optimized code for 1 channel (mono)
        for (SINT i = 0; i < numWritableFrames; ++i) {
            *pSampleBuffer++ = buffer[0][i] * m_sampleScaleFactor;
        }
        break;
    }
    case 2: {
        // optimized code for 2 channels (stereo)
        for (SINT i = 0; i < numWritableFrames; ++i) {
            *pSampleBuffer++ = buffer[0][i] * m_sampleScaleFactor;
            *pSampleBuffer++ = buffer[1][i] * m_sampleScaleFactor;
        }
        break;
    }
    default: {
        // generic code for multiple channels
        for (SINT i = 0; i < numWritableFrames; ++i) {
            for (SINT j = 0; j < getChannelCount(); ++j) {
                *pSampleBuffer++ = buffer[j][i] * m_sampleScaleFactor;
            }
        }
    }
    }

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
        if (isValidChannelCount(channelCount)) {
            if (hasChannelCount()) {
                // already set before -> check for consistency
                if (getChannelCount() != channelCount) {
                    qWarning() << "Unexpected channel count:"
                            << channelCount << " <> " << getChannelCount();
                }
            } else {
                // not set before
                setChannelCount(channelCount);
            }
        } else {
            qWarning() << "Invalid channel count:"
                    << channelCount;
        }
        const SINT frameRate = metadata->data.stream_info.sample_rate;
        if (isValidFrameRate(frameRate)) {
            if (hasFrameRate()) {
                // already set before -> check for consistency
                if (getFrameRate() != frameRate) {
                    qWarning() << "Unexpected frame/sample rate:"
                            << frameRate << " <> " << getFrameRate();
                }
            } else {
                // not set before
                setFrameRate(frameRate);
            }
        } else {
            qWarning() << "Invalid frame/sample rate:"
                    << frameRate;
        }
        const SINT frameCount = metadata->data.stream_info.total_samples;
        DEBUG_ASSERT(isValidFrameCount(frameCount));
        if (isEmpty()) {
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
            m_sampleScaleFactor = CSAMPLE_PEAK
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
        const SINT sampleBufferCapacity =
                m_maxBlocksize * getChannelCount();
        m_sampleBuffer.resetCapacity(sampleBufferCapacity);
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

QString SoundSourceProviderFLAC::getName() const {
    return "Xiph.org libFLAC";
}

QStringList SoundSourceProviderFLAC::getSupportedFileExtensions() const {
    QStringList supportedFileExtensions;
    supportedFileExtensions.append("flac");
    return supportedFileExtensions;
}

} // namespace Mixxx
