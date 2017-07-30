#include "sources/soundsourceflac.h"

#include "util/math.h"
#include "util/sample.h"
#include "util/logger.h"

namespace mixxx {

namespace {

const Logger kLogger("SoundSourceFLAC");

// The maximum number of retries to fix seek errors. On a seek error
// the next seek will start one (or more) sample blocks before the
// position of the preceding seek operation that has failed.
const int kSeekErrorMaxRetryCount = 3;

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

SoundSourceFLAC::SoundSourceFLAC(const QUrl& url)
        : SoundSource(url, "flac"),
          m_file(getLocalFileName()),
          m_decoder(nullptr),
          m_maxBlocksize(0),
          m_bitsPerSample(kBitsPerSampleDefault),
          m_sampleScaleFactor(CSAMPLE_ZERO),
          m_curFrameIndex(getMinFrameIndex()) {
}

SoundSourceFLAC::~SoundSourceFLAC() {
    close();
}

SoundSource::OpenResult SoundSourceFLAC::tryOpen(const AudioSourceConfig& /*audioSrcCfg*/) {
    DEBUG_ASSERT(!m_file.isOpen());
    if (!m_file.open(QIODevice::ReadOnly)) {
        kLogger.warning() << "Failed to open FLAC file:" << m_file.fileName();
        return OpenResult::FAILED;
    }

    m_decoder = FLAC__stream_decoder_new();
    if (m_decoder == nullptr) {
        kLogger.warning() << "Failed to create FLAC decoder!";
        return OpenResult::FAILED;
    }
    FLAC__stream_decoder_set_md5_checking(m_decoder, false);
    const FLAC__StreamDecoderInitStatus initStatus(
            FLAC__stream_decoder_init_stream(m_decoder, FLAC_read_cb,
                    FLAC_seek_cb, FLAC_tell_cb, FLAC_length_cb, FLAC_eof_cb,
                    FLAC_write_cb, FLAC_metadata_cb, FLAC_error_cb, this));
    if (initStatus != FLAC__STREAM_DECODER_INIT_STATUS_OK) {
        kLogger.warning() << "Failed to initialize FLAC decoder:" << initStatus;
        return OpenResult::FAILED;
    }
    if (!FLAC__stream_decoder_process_until_end_of_metadata(m_decoder)) {
        kLogger.warning() << "Failed to process FLAC metadata:"
                << FLAC__stream_decoder_get_state(m_decoder);
        return OpenResult::FAILED;
    }

    m_curFrameIndex = getMinFrameIndex();

    return OpenResult::SUCCEEDED;
}

void SoundSourceFLAC::close() {
    if (m_decoder) {
        FLAC__stream_decoder_finish(m_decoder);
        FLAC__stream_decoder_delete(m_decoder); // frees memory
        m_decoder = nullptr;
    }

    m_file.close();
}

SINT SoundSourceFLAC::seekSampleFrame(SINT frameIndex) {
    DEBUG_ASSERT(isValidFrameIndex(m_curFrameIndex));
    DEBUG_ASSERT(isValidFrameIndex(frameIndex));

    // Seek to the new position
    SINT seekFrameIndex = frameIndex;
    int retryCount = 0;
    // NOTE(uklotzde): This loop avoids unnecessary seek operations.
    // If the file is decoded from the beginning to the end during
    // continuous playback no seek operations are necessary. This
    // may hide rare seek errors that we have observed in some "flaky"
    // FLAC files. The retry strategy implemented by this loop tries
    // to solve these issues when randomly seeking through such a file.
    while ((seekFrameIndex != m_curFrameIndex) &&
            (retryCount <= kSeekErrorMaxRetryCount)){
        // Discard decoded sample data before seeking
        m_sampleBuffer.reset();
        // Invalidate the current position
        m_curFrameIndex = getMaxFrameIndex();
        if (FLAC__stream_decoder_seek_absolute(m_decoder, seekFrameIndex)) {
            // Success: Set the new position
            m_curFrameIndex = seekFrameIndex;
            DEBUG_ASSERT(FLAC__STREAM_DECODER_SEEK_ERROR != FLAC__stream_decoder_get_state(m_decoder));
        } else {
            // Failure
            kLogger.warning() << "Seek error at" << seekFrameIndex << "in" << m_file.fileName();
            if (FLAC__STREAM_DECODER_SEEK_ERROR == FLAC__stream_decoder_get_state(m_decoder)) {
                // Flush the input stream of the decoder according to the
                // documentation of FLAC__stream_decoder_seek_absolute()
                if (!FLAC__stream_decoder_flush(m_decoder)) {
                    kLogger.warning() << "Failed to flush input buffer of the FLAC decoder after seek failure in"
                            << m_file.fileName();
                    // Invalidate the current position again...
                    m_curFrameIndex = getMaxFrameIndex();
                    // ...and abort
                    return m_curFrameIndex;
                }
            }
            if (getMinFrameIndex() < seekFrameIndex) {
                // The next seek position should start at a preceding sample block.
                // By subtracting max. blocksize from the current seek position it
                // is guaranteed that the targeted sample blocks of subsequent seek
                // operations will differ.
                DEBUG_ASSERT(0 < m_maxBlocksize);
                seekFrameIndex -= m_maxBlocksize;
                if (seekFrameIndex < getMinFrameIndex()) {
                    seekFrameIndex = getMinFrameIndex();
                }
            } else {
                // We have already reached the beginning of the file
                // and cannot move the seek position backward any
                // further!
                break; // exit loop
            }
        }
    } while (m_curFrameIndex != seekFrameIndex);
    DEBUG_ASSERT(isValidFrameIndex(m_curFrameIndex));

    if (frameIndex > m_curFrameIndex) {
        // Adjust the current position
        skipSampleFrames(frameIndex - m_curFrameIndex);
    }

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
                kLogger.warning() << "Failed to decode FLAC file"
                        << m_file.fileName();
                break; // abort
            }
            // After seeking we might need to skip some samples if the decoder
            // complained that it has lost sync for some malformed(?) files
            if (curFrameIndexBeforeProcessing != m_curFrameIndex) {
                if (curFrameIndexBeforeProcessing > m_curFrameIndex) {
                    kLogger.warning() << "Trying to adjust frame index"
                            << m_curFrameIndex << "<>" << curFrameIndexBeforeProcessing
                            << "while decoding FLAC file"
                            << m_file.fileName();
                    skipSampleFrames(curFrameIndexBeforeProcessing - m_curFrameIndex);
                } else {
                    kLogger.warning() << "Unexpected frame index"
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

FLAC__StreamDecoderSeekStatus SoundSourceFLAC::flacSeek(FLAC__uint64 absolute_byte_offset) {
    if (m_file.seek(absolute_byte_offset)) {
        return FLAC__STREAM_DECODER_SEEK_STATUS_OK;
    } else {
        kLogger.warning() << "SoundSourceFLAC: An unrecoverable error occurred ("
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
        kLogger.warning() << "Corrupt or unsupported FLAC file:"
                << "Invalid number of channels in FLAC frame header"
                << frame->header.channels << "<>" << getChannelCount();
        return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
    }
    if (getSamplingRate() != SINT(frame->header.sample_rate)) {
        kLogger.warning() << "Corrupt or unsupported FLAC file:"
                << "Invalid sample rate in FLAC frame header"
                << frame->header.sample_rate << "<>" << getSamplingRate();
        return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
    }
    const SINT numReadableFrames = frame->header.blocksize;
    if (numReadableFrames > m_maxBlocksize) {
        kLogger.warning() << "Corrupt or unsupported FLAC file:"
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
        kLogger.warning() << "Sample buffer has not enough free space for all decoded FLAC samples:"
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
            if (hasValidChannelCount()) {
                // already set before -> check for consistency
                if (getChannelCount() != channelCount) {
                    kLogger.warning() << "Unexpected channel count:"
                            << channelCount << " <> " << getChannelCount();
                }
            } else {
                // not set before
                setChannelCount(channelCount);
            }
        } else {
            kLogger.warning() << "Invalid channel count:"
                    << channelCount;
        }
        const SINT samplingRate = metadata->data.stream_info.sample_rate;
        if (isValidSamplingRate(samplingRate)) {
            if (hasValidSamplingRate()) {
                // already set before -> check for consistency
                if (getSamplingRate() != samplingRate) {
                    kLogger.warning() << "Unexpected sampling rate:"
                            << samplingRate << " <> " << getSamplingRate();
                }
            } else {
                // not set before
                setSamplingRate(samplingRate);
            }
        } else {
            kLogger.warning() << "Invalid sampling rate:"
                    << samplingRate;
        }
        const SINT frameCount = metadata->data.stream_info.total_samples;
        DEBUG_ASSERT(isValidFrameCount(frameCount));
        if (isEmpty()) {
            // not set before
            setFrameCount(frameCount);
        } else {
            // already set before -> check for consistency
            if (getFrameCount() != frameCount) {
                kLogger.warning() << "Unexpected frame count:"
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
                kLogger.warning() << "Unexpected bits per sample:"
                        << bitsPerSample << " <> " << m_bitsPerSample;
            }
        }
        m_maxBlocksize = metadata->data.stream_info.max_blocksize;
        if (0 >= m_maxBlocksize) {
            kLogger.warning() << "Invalid max. blocksize" << m_maxBlocksize;
        }
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
    kLogger.warning() << "FLAC decoding error" << error << "in file"
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

} // namespace mixxx
