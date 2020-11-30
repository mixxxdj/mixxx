#include "sources/soundsourceflac.h"

#include "util/logger.h"
#include "util/math.h"
#include "util/sample.h"

namespace mixxx {

namespace {

const Logger kLogger("SoundSourceFLAC");

// The maximum number of retries to fix seek errors. On a seek error
// the next seek will start one (or more) sample blocks before the
// position of the preceding seek operation that has failed.
constexpr int kSeekErrorMaxRetryCount = 3;

// begin callbacks (have to be regular functions because normal libFLAC isn't C++-aware)

FLAC__StreamDecoderReadStatus FLAC_read_cb(const FLAC__StreamDecoder*,
        FLAC__byte buffer[],
        size_t* bytes,
        void* client_data) {
    return static_cast<SoundSourceFLAC*>(client_data)->flacRead(buffer, bytes);
}

FLAC__StreamDecoderSeekStatus FLAC_seek_cb(const FLAC__StreamDecoder*,
        FLAC__uint64 absolute_byte_offset,
        void* client_data) {
    return static_cast<SoundSourceFLAC*>(client_data)->flacSeek(absolute_byte_offset);
}

FLAC__StreamDecoderTellStatus FLAC_tell_cb(const FLAC__StreamDecoder*,
        FLAC__uint64* absolute_byte_offset,
        void* client_data) {
    return static_cast<SoundSourceFLAC*>(client_data)->flacTell(absolute_byte_offset);
}

FLAC__StreamDecoderLengthStatus FLAC_length_cb(const FLAC__StreamDecoder*,
        FLAC__uint64* stream_length,
        void* client_data) {
    return static_cast<SoundSourceFLAC*>(client_data)->flacLength(stream_length);
}

FLAC__bool FLAC_eof_cb(const FLAC__StreamDecoder*, void* client_data) {
    return static_cast<SoundSourceFLAC*>(client_data)->flacEOF();
}

FLAC__StreamDecoderWriteStatus FLAC_write_cb(const FLAC__StreamDecoder*,
        const FLAC__Frame* frame,
        const FLAC__int32* const buffer[],
        void* client_data) {
    return static_cast<SoundSourceFLAC*>(client_data)->flacWrite(frame, buffer);
}

void FLAC_metadata_cb(const FLAC__StreamDecoder*,
        const FLAC__StreamMetadata* metadata,
        void* client_data) {
    static_cast<SoundSourceFLAC*>(client_data)->flacMetadata(metadata);
}

void FLAC_error_cb(const FLAC__StreamDecoder*,
        FLAC__StreamDecoderErrorStatus status,
        void* client_data) {
    static_cast<SoundSourceFLAC*>(client_data)->flacError(status);
}

// end callbacks

const SINT kBitsPerSampleDefault = 0;

} // namespace

//static
const QString SoundSourceProviderFLAC::kDisplayName = QStringLiteral("Xiph.org libFLAC");

//static
const QStringList SoundSourceProviderFLAC::kSupportedFileExtensions = {
        QStringLiteral("flac"),
};

SoundSourceProviderPriority SoundSourceProviderFLAC::getPriorityHint(
        const QString& supportedFileExtension) const {
    Q_UNUSED(supportedFileExtension)
    // This reference decoder is supposed to produce more accurate
    // and reliable results than any other DEFAULT provider.
    return SoundSourceProviderPriority::Higher;
}

SoundSourceFLAC::SoundSourceFLAC(const QUrl& url)
        : SoundSource(url),
          m_file(getLocalFileName()),
          m_decoder(nullptr),
          m_maxBlocksize(0),
          m_bitsPerSample(kBitsPerSampleDefault),
          m_curFrameIndex(0) {
}

SoundSourceFLAC::~SoundSourceFLAC() {
    close();
}

SoundSource::OpenResult SoundSourceFLAC::tryOpen(
        OpenMode /*mode*/,
        const OpenParams& /*config*/) {
    DEBUG_ASSERT(!m_file.isOpen());
    if (!m_file.open(QIODevice::ReadOnly)) {
        kLogger.warning()
                << "Failed to open FLAC file:"
                << m_file.fileName();
        return OpenResult::Failed;
    }

    m_decoder = FLAC__stream_decoder_new();
    if (m_decoder == nullptr) {
        kLogger.warning()
                << "Failed to create FLAC decoder!";
        return OpenResult::Failed;
    }
    FLAC__stream_decoder_set_md5_checking(m_decoder, false);
    const FLAC__StreamDecoderInitStatus initStatus(
            FLAC__stream_decoder_init_stream(
                    m_decoder,
                    FLAC_read_cb,
                    FLAC_seek_cb,
                    FLAC_tell_cb,
                    FLAC_length_cb,
                    FLAC_eof_cb,
                    FLAC_write_cb,
                    FLAC_metadata_cb,
                    FLAC_error_cb,
                    this));
    if (initStatus != FLAC__STREAM_DECODER_INIT_STATUS_OK) {
        kLogger.warning()
                << "Failed to initialize FLAC decoder:"
                << initStatus;
        return OpenResult::Failed;
    }
    if (!FLAC__stream_decoder_process_until_end_of_metadata(m_decoder)) {
        kLogger.warning()
                << "Failed to process FLAC metadata:"
                << FLAC__stream_decoder_get_state(m_decoder);
        return OpenResult::Failed;
    }

    m_curFrameIndex = frameIndexMin();

    return OpenResult::Succeeded;
}

void SoundSourceFLAC::close() {
    if (m_decoder) {
        FLAC__stream_decoder_finish(m_decoder);
        FLAC__stream_decoder_delete(m_decoder); // frees memory
        m_decoder = nullptr;
    }

    m_file.close();
}

ReadableSampleFrames SoundSourceFLAC::readSampleFramesClamped(
        const WritableSampleFrames& writableSampleFrames) {
    const SINT firstFrameIndex = writableSampleFrames.frameIndexRange().start();

    if (m_curFrameIndex != firstFrameIndex) {
        // Seek to the new position
        SINT seekFrameIndex = firstFrameIndex;
        int retryCount = 0;
        // NOTE(uklotzde): This loop avoids unnecessary seek operations.
        // If the file is decoded from the beginning to the end during
        // continuous playback no seek operations are necessary. This
        // may hide rare seek errors that we have observed in some "flaky"
        // FLAC files. The retry strategy implemented by this loop tries
        // to solve these issues when randomly seeking through such a file.
        while ((seekFrameIndex != m_curFrameIndex) &&
                (retryCount <= kSeekErrorMaxRetryCount)) {
            // Discard decoded sample data before seeking
            m_sampleBuffer.clear();
            invalidateCurFrameIndex();
            if (FLAC__stream_decoder_seek_absolute(m_decoder, seekFrameIndex)) {
                DEBUG_ASSERT(FLAC__STREAM_DECODER_SEEK_ERROR != FLAC__stream_decoder_get_state(m_decoder));
                // Success: Set the new position
                m_curFrameIndex = seekFrameIndex;
            } else {
                // Failure
                kLogger.warning()
                        << "Seek error at" << seekFrameIndex
                        << "in file" << m_file.fileName();
                if (FLAC__STREAM_DECODER_SEEK_ERROR == FLAC__stream_decoder_get_state(m_decoder)) {
                    // Flush the input stream of the decoder according to the
                    // documentation of FLAC__stream_decoder_seek_absolute()
                    if (!FLAC__stream_decoder_flush(m_decoder)) {
                        kLogger.warning()
                                << "Failed to flush input buffer of the FLAC decoder after seek failure"
                                << "in file" << m_file.fileName();
                        invalidateCurFrameIndex();
                        // ...and abort
                        return ReadableSampleFrames(
                                IndexRange::between(
                                        m_curFrameIndex,
                                        m_curFrameIndex));
                    }
                }
                if (frameIndexMin() < seekFrameIndex) {
                    // The next seek position should start at a preceding sample block.
                    // By subtracting max. blocksize from the current seek position it
                    // is guaranteed that the targeted sample blocks of subsequent seek
                    // operations will differ.
                    DEBUG_ASSERT(0 < m_maxBlocksize);
                    seekFrameIndex -= m_maxBlocksize;
                    if (seekFrameIndex < frameIndexMin()) {
                        seekFrameIndex = frameIndexMin();
                    }
                } else {
                    // We have already reached the beginning of the file
                    // and cannot move the seek position backwards any
                    // further! As a last resort try to reset the decoder.
                    kLogger.warning()
                            << "Resetting decoder after seek errors";
                    if (!FLAC__stream_decoder_reset(m_decoder)) {
                        kLogger.critical()
                                << "Failed to reset decoder after seek errors";
                        break; // exit loop
                    }
                }
            }
        }

        // Decoding starts before the actual target position
        DEBUG_ASSERT(m_curFrameIndex <= firstFrameIndex);
        const auto precedingFrames =
                IndexRange::between(m_curFrameIndex, firstFrameIndex);
        if (!precedingFrames.empty() &&
                (precedingFrames != readSampleFramesClamped(WritableSampleFrames(precedingFrames)).frameIndexRange())) {
            kLogger.warning()
                    << "Resetting decoder after failure to skip preceding frames"
                    << precedingFrames;
            if (!FLAC__stream_decoder_reset(m_decoder)) {
                kLogger.critical()
                        << "Failed to reset decoder after skip errors";
            }
            // Abort
            return ReadableSampleFrames(
                    IndexRange::between(
                            m_curFrameIndex,
                            m_curFrameIndex));
        }
    }
    DEBUG_ASSERT(m_curFrameIndex == firstFrameIndex);

    const SINT numberOfSamplesTotal =
            getSignalInfo().frames2samples(
                    writableSampleFrames.frameLength());

    SINT numberOfSamplesRemaining = numberOfSamplesTotal;
    SINT outputSampleOffset = 0;
    while (0 < numberOfSamplesRemaining) {
        // If our buffer from libflac is empty (either because we explicitly cleared
        // it or because we've simply used all the samples), ask for a new buffer
        if (m_sampleBuffer.empty()) {
            // Save the current frame index
            const SINT curFrameIndexBeforeProcessing = m_curFrameIndex;
            // Documentation of FLAC__stream_decoder_process_single():
            // "Depending on what was decoded, the metadata or write callback
            // will be called with the decoded metadata block or audio frame."
            // See also: https://xiph.org/flac/api/group__flac__stream__decoder.html#ga9d6df4a39892c05955122cf7f987f856
            if (!FLAC__stream_decoder_process_single(m_decoder)) {
                kLogger.warning()
                        << "Failed to decode FLAC file"
                        << m_file.fileName();
                break; // abort
            }
            // After decoding we might first need to skip some samples if the
            // decoder complained that it has lost sync for some malformed(?)
            // files
            if (m_curFrameIndex != curFrameIndexBeforeProcessing) {
                if (m_curFrameIndex < curFrameIndexBeforeProcessing) {
                    kLogger.warning()
                            << "Trying to adjust frame index"
                            << m_curFrameIndex << "<" << curFrameIndexBeforeProcessing
                            << "while decoding FLAC file"
                            << m_file.fileName();
                    const auto skipFrames =
                            IndexRange::between(m_curFrameIndex, curFrameIndexBeforeProcessing);
                    if (skipFrames != readSampleFramesClamped(WritableSampleFrames(skipFrames)).frameIndexRange()) {
                        kLogger.warning()
                                << "Failed to skip sample frames"
                                << skipFrames
                                << "while decoding FLAC file"
                                << m_file.fileName();
                        break; // abort
                    }
                } else {
                    kLogger.warning()
                            << "Unexpected frame index"
                            << m_curFrameIndex << ">" << curFrameIndexBeforeProcessing
                            << "while decoding FLAC file"
                            << m_file.fileName();
                    break; // abort
                }
            }
            DEBUG_ASSERT(curFrameIndexBeforeProcessing == m_curFrameIndex);
        }
        if (m_sampleBuffer.empty()) {
            break; // EOF
        }

        const SINT numberOfSamplesRead =
                std::min(m_sampleBuffer.readableLength(), numberOfSamplesRemaining);
        const SampleBuffer::ReadableSlice readableSlice(
                m_sampleBuffer.shrinkForReading(numberOfSamplesRead));
        DEBUG_ASSERT(readableSlice.length() == numberOfSamplesRead);
        if (writableSampleFrames.writableData()) {
            SampleUtil::copy(
                    writableSampleFrames.writableData(outputSampleOffset),
                    readableSlice.data(),
                    readableSlice.length());
            outputSampleOffset += numberOfSamplesRead;
        }
        m_curFrameIndex += getSignalInfo().samples2frames(numberOfSamplesRead);
        numberOfSamplesRemaining -= numberOfSamplesRead;
    }

    DEBUG_ASSERT(isValidFrameIndex(m_curFrameIndex));
    DEBUG_ASSERT(numberOfSamplesTotal >= numberOfSamplesRemaining);
    const SINT numberOfSamples = numberOfSamplesTotal - numberOfSamplesRemaining;
    return ReadableSampleFrames(
            IndexRange::forward(firstFrameIndex, getSignalInfo().samples2frames(numberOfSamples)),
            SampleBuffer::ReadableSlice(
                    writableSampleFrames.writableData(),
                    std::min(writableSampleFrames.writableLength(), numberOfSamples)));
}

// flac callback methods
FLAC__StreamDecoderReadStatus SoundSourceFLAC::flacRead(FLAC__byte buffer[],
        size_t* bytes) {
    const qint64 maxlen = *bytes;
    if (0 >= maxlen) {
        *bytes = 0;
        return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
    }

    const qint64 readlen = m_file.read((char*)buffer, maxlen);

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
        kLogger.warning()
                << "SoundSourceFLAC: An unrecoverable error occurred ("
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

namespace {

// Workaround for improperly encoded FLAC files that may contain
// garbage in the most significant, unused bits of decoded samples.
// Required at least for libFLAC 1.3.2. This workaround might become
// obsolete once libFLAC is taking care of these issues internally.
// https://bugs.launchpad.net/mixxx/+bug/1769717
// https://hydrogenaud.io/index.php/topic,61792.msg559045.html#msg559045

// We will shift decoded samples left by (32 - m_bitsPerSample) to
// get rid of the garbage in the most significant bits before scaling
// to the range [-CSAMPLE_PEAK, CSAMPLE_PEAK - epsilon] with
// epsilon = 1 / 2 ^ bitsPerSample.
constexpr CSAMPLE kSampleScaleFactor = CSAMPLE_PEAK /
        (static_cast<FLAC__int32>(1) << std::numeric_limits<FLAC__int32>::digits);

inline CSAMPLE convertDecodedSample(FLAC__int32 decodedSample, int bitsPerSample) {
    DEBUG_ASSERT(std::numeric_limits<FLAC__int32>::is_signed);
    return (decodedSample << ((std::numeric_limits<FLAC__int32>::digits + 1) - bitsPerSample)) * kSampleScaleFactor;
}

} // anonymous namespace

FLAC__StreamDecoderWriteStatus SoundSourceFLAC::flacWrite(
        const FLAC__Frame* frame, const FLAC__int32* const buffer[]) {
    const SINT numChannels = frame->header.channels;
    if (getSignalInfo().getChannelCount() > numChannels) {
        kLogger.warning()
                << "Corrupt or unsupported FLAC file:"
                << "Invalid number of channels in FLAC frame header"
                << frame->header.channels << "<>" << getSignalInfo().getChannelCount();
        return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
    }
    if (getSignalInfo().getSampleRate() != SINT(frame->header.sample_rate)) {
        kLogger.warning()
                << "Corrupt or unsupported FLAC file:"
                << "Invalid sample rate in FLAC frame header"
                << frame->header.sample_rate << "<>" << getSignalInfo().getSampleRate();
        return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
    }
    const SINT numReadableFrames = frame->header.blocksize;
    if (numReadableFrames > m_maxBlocksize) {
        kLogger.warning()
                << "Corrupt or unsupported FLAC file:"
                << "Block size in FLAC frame header exceeds the maximum block size"
                << frame->header.blocksize << ">" << m_maxBlocksize;
        return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
    }

    // According to the API docs the decoder will always report the current
    // position in "FLAC samples" (= "Mixxx frames") for convenience
    DEBUG_ASSERT(frame->header.number_type == FLAC__FRAME_NUMBER_TYPE_SAMPLE_NUMBER);
    m_curFrameIndex = frame->header.number.sample_number;

    // Decode buffer should be empty before decoding the next frame
    DEBUG_ASSERT(m_sampleBuffer.empty());
    const SampleBuffer::WritableSlice writableSlice(
            m_sampleBuffer.growForWriting(
                    getSignalInfo().frames2samples(numReadableFrames)));

    const SINT numWritableFrames =
            getSignalInfo().samples2frames(writableSlice.length());
    DEBUG_ASSERT(numWritableFrames <= numReadableFrames);
    if (numWritableFrames < numReadableFrames) {
        kLogger.warning()
                << "Sample buffer has not enough free space for all decoded FLAC samples:"
                << numWritableFrames << "<" << numReadableFrames;
    }

    CSAMPLE* pSampleBuffer = writableSlice.data();
    DEBUG_ASSERT(getSignalInfo().getChannelCount() <= numChannels);
    switch (getSignalInfo().getChannelCount()) {
    case 1: {
        // optimized code for 1 channel (mono)
        for (SINT i = 0; i < numWritableFrames; ++i) {
            *pSampleBuffer++ = convertDecodedSample(buffer[0][i], m_bitsPerSample);
        }
        break;
    }
    case 2: {
        // optimized code for 2 channels (stereo)
        for (SINT i = 0; i < numWritableFrames; ++i) {
            *pSampleBuffer++ = convertDecodedSample(buffer[0][i], m_bitsPerSample);
            *pSampleBuffer++ = convertDecodedSample(buffer[1][i], m_bitsPerSample);
        }
        break;
    }
    default: {
        // generic code for multiple channels
        for (SINT i = 0; i < numWritableFrames; ++i) {
            for (SINT j = 0; j < getSignalInfo().getChannelCount(); ++j) {
                *pSampleBuffer++ = convertDecodedSample(buffer[j][i], m_bitsPerSample);
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
    case FLAC__METADATA_TYPE_STREAMINFO: {
        initChannelCountOnce(metadata->data.stream_info.channels);
        initSampleRateOnce(metadata->data.stream_info.sample_rate);
        initFrameIndexRangeOnce(
                IndexRange::forward(
                        0,
                        metadata->data.stream_info.total_samples));

        const SINT bitsPerSample = metadata->data.stream_info.bits_per_sample;
        DEBUG_ASSERT(bitsPerSample > 0);
        DEBUG_ASSERT(kBitsPerSampleDefault != bitsPerSample);
        if (kBitsPerSampleDefault == m_bitsPerSample) {
            // not set before
            if ((bitsPerSample >= 4) && (bitsPerSample <= 32)) {
                m_bitsPerSample = bitsPerSample;
            } else {
                kLogger.warning()
                        << "Invalid bits per sample:"
                        << bitsPerSample;
            }
        } else {
            // already set before -> check for consistency
            if (bitsPerSample != m_bitsPerSample) {
                kLogger.warning()
                        << "Unexpected bits per sample:"
                        << bitsPerSample << " <> " << m_bitsPerSample;
            }
        }
        DEBUG_ASSERT(m_maxBlocksize >= 0);
        m_maxBlocksize = math_max(
                m_maxBlocksize,
                static_cast<SINT>(metadata->data.stream_info.max_blocksize));
        if (m_maxBlocksize > 0) {
            const SINT sampleBufferCapacity =
                    m_maxBlocksize * getSignalInfo().getChannelCount();
            if (m_sampleBuffer.capacity() < sampleBufferCapacity) {
                m_sampleBuffer.adjustCapacity(sampleBufferCapacity);
            }
        } else {
            kLogger.warning()
                    << "Invalid max. blocksize" << m_maxBlocksize;
        }
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
    kLogger.warning()
            << "FLAC decoding error" << error
            << "in file" << m_file.fileName();
    // not much else to do here... whatever function that initiated whatever
    // decoder method resulted in this error will return an error, and the caller
    // will bail. libFLAC docs say to not close the decoder here -- bkgood
}

} // namespace mixxx
