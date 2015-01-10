#include "sources/audiosourceflac.h"

#include "sampleutil.h"
#include "util/math.h"

#include <QtDebug>

namespace Mixxx
{

namespace
{
// begin callbacks (have to be regular functions because normal libFLAC isn't C++-aware)

FLAC__StreamDecoderReadStatus FLAC_read_cb(const FLAC__StreamDecoder*,
        FLAC__byte buffer[], size_t *bytes, void *client_data) {
    return static_cast<AudioSourceFLAC*>(client_data)->flacRead(buffer, bytes);
}

FLAC__StreamDecoderSeekStatus FLAC_seek_cb(const FLAC__StreamDecoder*,
        FLAC__uint64 absolute_byte_offset, void *client_data) {
    return static_cast<AudioSourceFLAC*>(client_data)->flacSeek(absolute_byte_offset);
}

FLAC__StreamDecoderTellStatus FLAC_tell_cb(const FLAC__StreamDecoder*,
        FLAC__uint64 *absolute_byte_offset, void *client_data) {
    return static_cast<AudioSourceFLAC*>(client_data)->flacTell(absolute_byte_offset);
}

FLAC__StreamDecoderLengthStatus FLAC_length_cb(const FLAC__StreamDecoder*,
        FLAC__uint64 *stream_length, void *client_data) {
    return static_cast<AudioSourceFLAC*>(client_data)->flacLength(stream_length);
}

FLAC__bool FLAC_eof_cb(const FLAC__StreamDecoder*, void *client_data) {
    return static_cast<AudioSourceFLAC*>(client_data)->flacEOF();
}

FLAC__StreamDecoderWriteStatus FLAC_write_cb(const FLAC__StreamDecoder*,
        const FLAC__Frame *frame, const FLAC__int32 * const buffer[],
        void *client_data) {
    return static_cast<AudioSourceFLAC*>(client_data)->flacWrite(frame, buffer);
}

void FLAC_metadata_cb(const FLAC__StreamDecoder*,
        const FLAC__StreamMetadata *metadata, void *client_data) {
    static_cast<AudioSourceFLAC*>(client_data)->flacMetadata(metadata);
}

void FLAC_error_cb(const FLAC__StreamDecoder*,
        FLAC__StreamDecoderErrorStatus status, void *client_data) {
    static_cast<AudioSourceFLAC*>(client_data)->flacError(status);
}

// end callbacks
}

AudioSourceFLAC::AudioSourceFLAC(QString fileName)
        : m_file(fileName), m_decoder(NULL), m_minBlocksize(
                0), m_maxBlocksize(0), m_minFramesize(0), m_maxFramesize(0), m_sampleScale(
                kSampleValueZero), m_decodeSampleBufferReadOffset(0), m_decodeSampleBufferWriteOffset(
                0) {
}

AudioSourceFLAC::~AudioSourceFLAC() {
    close();
}

AudioSourcePointer AudioSourceFLAC::create(QString fileName) {
    QSharedPointer<AudioSourceFLAC> pAudioSource(new AudioSourceFLAC(fileName));
    if (OK == pAudioSource->open()) {
        // success
        return pAudioSource;
    } else {
        // failure
        return AudioSourcePointer();
    }
}

Result AudioSourceFLAC::open() {
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
    const FLAC__StreamDecoderInitStatus initStatus(
            FLAC__stream_decoder_init_stream(m_decoder, FLAC_read_cb,
                    FLAC_seek_cb, FLAC_tell_cb, FLAC_length_cb, FLAC_eof_cb,
                    FLAC_write_cb, FLAC_metadata_cb, FLAC_error_cb, this));
    if (initStatus != FLAC__STREAM_DECODER_INIT_STATUS_OK) {
        qWarning() << "SSFLAC: decoder init failed:" << initStatus;
        return ERR;
    }
    if (!FLAC__stream_decoder_process_until_end_of_metadata(m_decoder)) {
        qWarning() << "SSFLAC: process to end of meta failed:"
                << FLAC__stream_decoder_get_state(m_decoder);
        return ERR;
    }

    return OK;
}

void AudioSourceFLAC::close() {
    if (m_decoder) {
        FLAC__stream_decoder_finish(m_decoder);
        FLAC__stream_decoder_delete(m_decoder); // frees memory
        m_decoder = NULL;
    }
    m_decodeSampleBuffer.clear();
    m_file.close();
    reset();
}

Mixxx::AudioSource::diff_type AudioSourceFLAC::seekSampleFrame(diff_type frameIndex) {
    // clear decode buffer before seeking
    m_decodeSampleBufferReadOffset = 0;
    m_decodeSampleBufferWriteOffset = 0;
    bool result = FLAC__stream_decoder_seek_absolute(m_decoder, frameIndex);
    if (!result) {
        qWarning() << "SSFLAC: Seeking error at file" << m_file.fileName();
    }
    return frameIndex;
}

Mixxx::AudioSource::size_type AudioSourceFLAC::readSampleFrames(
        size_type numberOfFrames, sample_type* sampleBuffer) {
    return readSampleFrames(numberOfFrames, sampleBuffer, false);
}

Mixxx::AudioSource::size_type AudioSourceFLAC::readSampleFramesStereo(
        size_type numberOfFrames, sample_type* sampleBuffer) {
    return readSampleFrames(numberOfFrames, sampleBuffer, true);
}

Mixxx::AudioSource::size_type AudioSourceFLAC::readSampleFrames(
        size_type numberOfFrames, sample_type* sampleBuffer,
        bool readStereoSamples) {
    sample_type* outBuffer = sampleBuffer;
    size_type framesRemaining = numberOfFrames;
    while (0 < framesRemaining) {
        DEBUG_ASSERT(
                m_decodeSampleBufferReadOffset
                        <= m_decodeSampleBufferWriteOffset);
        // if our buffer from libflac is empty (either because we explicitly cleared
        // it or because we've simply used all the samples), ask for a new buffer
        if (m_decodeSampleBufferReadOffset >= m_decodeSampleBufferWriteOffset) {
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
        framesRemaining -= framesToCopy;
        DEBUG_ASSERT(
                m_decodeSampleBufferReadOffset
                        <= m_decodeSampleBufferWriteOffset);
    }
    return numberOfFrames - framesRemaining;
}

// flac callback methods
FLAC__StreamDecoderReadStatus AudioSourceFLAC::flacRead(FLAC__byte buffer[],
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

FLAC__StreamDecoderSeekStatus AudioSourceFLAC::flacSeek(FLAC__uint64 offset) {
    if (m_file.seek(offset)) {
        return FLAC__STREAM_DECODER_SEEK_STATUS_OK;
    } else {
        qWarning() << "SSFLAC: An unrecoverable error occurred ("
                << m_file.fileName() << ")";
        return FLAC__STREAM_DECODER_SEEK_STATUS_ERROR;
    }
}

FLAC__StreamDecoderTellStatus AudioSourceFLAC::flacTell(FLAC__uint64 *offset) {
    if (m_file.isSequential()) {
        return FLAC__STREAM_DECODER_TELL_STATUS_UNSUPPORTED;
    }
    *offset = m_file.pos();
    return FLAC__STREAM_DECODER_TELL_STATUS_OK;
}

FLAC__StreamDecoderLengthStatus AudioSourceFLAC::flacLength(
        FLAC__uint64 *length) {
    if (m_file.isSequential()) {
        return FLAC__STREAM_DECODER_LENGTH_STATUS_UNSUPPORTED;
    }
    *length = m_file.size();
    return FLAC__STREAM_DECODER_LENGTH_STATUS_OK;
}

FLAC__bool AudioSourceFLAC::flacEOF() {
    if (m_file.isSequential()) {
        return false;
    }
    return m_file.atEnd();
}

FLAC__StreamDecoderWriteStatus AudioSourceFLAC::flacWrite(
        const FLAC__Frame *frame, const FLAC__int32 * const buffer[]) {
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
    const SampleBuffer::size_type maxBlocksize = samples2frames(m_decodeSampleBuffer.size());
    if (maxBlocksize < frame->header.blocksize) {
        qWarning() << "Corrupt or unsupported FLAC file:"
                << "Block size in FLAC frame header exceeds the maximum block size"
                << frame->header.blocksize << ">" << maxBlocksize;
    }
    switch (getChannelCount()) {
    case 1: {
        // optimized code for 1 channel (mono)
        DEBUG_ASSERT(1 <= frame->header.channels);
        for (unsigned i = 0; i < frame->header.blocksize; ++i) {
            m_decodeSampleBuffer[m_decodeSampleBufferWriteOffset++] =
                    buffer[0][i] * m_sampleScale;
        }
        break;
    }
    case 2: {
        // optimized code for 2 channels (stereo)
        DEBUG_ASSERT(2 <= frame->header.channels);
        for (unsigned i = 0; i < frame->header.blocksize; ++i) {
            m_decodeSampleBuffer[m_decodeSampleBufferWriteOffset++] =
                    buffer[0][i] * m_sampleScale;
            m_decodeSampleBuffer[m_decodeSampleBufferWriteOffset++] =
                    buffer[1][i] * m_sampleScale;
        }
        break;
    }
    default: {
        // generic code for multiple channels
        DEBUG_ASSERT(getChannelCount() == frame->header.channels);
        for (unsigned i = 0; i < frame->header.blocksize; ++i) {
            for (unsigned j = 0; j < frame->header.channels; ++j) {
                m_decodeSampleBuffer[m_decodeSampleBufferWriteOffset++] =
                        buffer[j][i] * m_sampleScale;
            }
        }
    }
    }
    DEBUG_ASSERT(m_decodeSampleBufferReadOffset <= m_decodeSampleBufferWriteOffset);
    return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

void AudioSourceFLAC::flacMetadata(const FLAC__StreamMetadata *metadata) {
    switch (metadata->type) {
    case FLAC__METADATA_TYPE_STREAMINFO:
        setChannelCount(metadata->data.stream_info.channels);
        setFrameRate(metadata->data.stream_info.sample_rate);
        setFrameCount(metadata->data.stream_info.total_samples);
        m_sampleScale = kSampleValuePeak
                / sample_type(
                        FLAC__int32(1)
                                << metadata->data.stream_info.bits_per_sample);
        qDebug() << "FLAC file " << m_file.fileName();
        qDebug() << getChannelCount() << " @ " << getFrameRate() << " Hz, "
                << getFrameCount() << " total, " << "bit depth"
                << " metadata->data.stream_info.bits_per_sample";
        m_minBlocksize = metadata->data.stream_info.min_blocksize;
        m_maxBlocksize = metadata->data.stream_info.max_blocksize;
        m_minFramesize = metadata->data.stream_info.min_framesize;
        m_maxFramesize = metadata->data.stream_info.max_framesize;
        qDebug() << "Blocksize in [" << m_minBlocksize << ", " << m_maxBlocksize
                << "], Framesize in [" << m_minFramesize << ", "
                << m_maxFramesize << "]";
        m_decodeSampleBufferReadOffset = 0;
        m_decodeSampleBufferWriteOffset = 0;
        m_decodeSampleBuffer.resize(m_maxBlocksize * getChannelCount());
        break;
    default:
        break;
    }
}

void AudioSourceFLAC::flacError(FLAC__StreamDecoderErrorStatus status) {
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
