#include "sources/soundsourcemp3.h"
#include "sources/mp3decoding.h"

#include "util/math.h"
#include "util/logger.h"

#include <id3tag.h>

namespace mixxx {

namespace {

const Logger kLogger("SoundSourceMP3");

// MP3 does only support 1 or 2 channels
const SINT kChannelCountMax = AudioSource::kChannelCountStereo;

const SINT kMaxBytesPerMp3Frame = 1441;

// mp3 supports 9 different sampling rates
const int kSamplingRateCount = 9;

int getIndexBySamplingRate(SINT samplingRate) {
    switch (samplingRate) {
    case 8000:
        return 0;
    case 11025:
        return 1;
    case 12000:
        return 2;
    case 16000:
        return 3;
    case 22050:
        return 4;
    case 24000:
        return 5;
    case 32000:
        return 6;
    case 44100:
        return 7;
    case 48000:
        return 8;
    default:
        // unsupported sampling rate
        return kSamplingRateCount;
    }
}

SINT getSamplingRateByIndex(int samplingRateIndex) {
    switch (samplingRateIndex) {
    case 0:
        return 8000;
    case 1:
        return 11025;
    case 2:
        return 12000;
    case 3:
        return 16000;
    case 4:
        return 22050;
    case 5:
        return 24000;
    case 6:
        return 32000;
    case 7:
        return 44100;
    case 8:
        return 48000;
    default:
        // index out of range
        return AudioSignal::kSamplingRateZero;
    }
}


const CSAMPLE kMadScale = CSAMPLE_PEAK / CSAMPLE(MAD_F_ONE);

inline CSAMPLE madScaleSampleValue(mad_fixed_t sampleValue) {
    return sampleValue * kMadScale;
}

// Optimization: Reserve initial capacity for seek frame list
const SINT kMinutesPerFile = 10; // enough for the majority of files (tunable)
const SINT kSecondsPerMinute = 60; // fixed
const SINT kMaxMp3FramesPerSecond = 39; // fixed: 1 MP3 frame = 26 ms -> ~ 1000 / 26
const SINT kSeekFrameListCapacity = kMinutesPerFile
        * kSecondsPerMinute * kMaxMp3FramesPerSecond;

inline QString formatHeaderFlags(int headerFlags) {
    return QString("0x%1").arg(headerFlags, 4, 16, QLatin1Char('0'));
}

void logFrameHeader(QDebug logger, const mad_header& madHeader) {
    logger << "MP3 frame header |"
            << "layer:" << madHeader.layer
            << "mode:" << madHeader.mode
            << "#channels:" << MAD_NCHANNELS(&madHeader)
            << "#samples:" << MAD_NSBSAMPLES(&madHeader)
            << "bitrate:" << madHeader.bitrate
            << "samplerate:" << madHeader.samplerate
            << "flags:" << formatHeaderFlags(madHeader.flags);
}

inline bool isRecoverableError(const mad_stream& madStream) {
    return MAD_RECOVERABLE(madStream.error);
}

inline bool isUnrecoverableError(const mad_stream& madStream) {
    return (MAD_ERROR_NONE != madStream.error) && !isRecoverableError(madStream);
}

inline bool isStreamValid(const mad_stream& madStream) {
    return !isUnrecoverableError(madStream);
}

bool decodeFrameHeader(
        mad_header* pMadHeader,
        mad_stream* pMadStream,
        bool skipId3Tag) {
    DEBUG_ASSERT(isStreamValid(*pMadStream));
    if (mad_header_decode(pMadHeader, pMadStream)) {
        // Something went wrong when decoding the frame header...
        if (MAD_ERROR_BUFLEN == pMadStream->error) {
            // EOF
            return false;
        }
        if (isUnrecoverableError(*pMadStream)) {
            DEBUG_ASSERT(!isStreamValid(*pMadStream));
            kLogger.warning() << "Unrecoverable MP3 header decoding error:"
                    << mad_stream_errorstr(pMadStream);
            return false;
        }
    #ifndef QT_NO_DEBUG_OUTPUT
        // Logging of MP3 frame headers should only be enabled
        // for debugging purposes.
        logFrameHeader(kLogger.debug(), *pMadHeader);
    #endif
        if (isRecoverableError(*pMadStream)) {
            if ((MAD_ERROR_LOSTSYNC == pMadStream->error) && skipId3Tag) {
                long tagsize = id3_tag_query(pMadStream->this_frame,
                        pMadStream->bufend - pMadStream->this_frame);
                if (0 < tagsize) {
                    // Skip ID3 tag data
                    mad_stream_skip(pMadStream, tagsize);
                    // Return immediately to suppress lost
                    // synchronization warnings
                    return false;
                }
            }
            kLogger.warning() << "Recoverable MP3 header decoding error:"
                    << mad_stream_errorstr(pMadStream);
            logFrameHeader(kLogger.warning(), *pMadHeader);
            return false;
        }
    }
    DEBUG_ASSERT(isStreamValid(*pMadStream));
    return true;
}

} // anonymous namespace

SoundSourceMp3::SoundSourceMp3(const QUrl& url)
        : SoundSource(url, "mp3"),
          m_file(getLocalFileName()),
          m_fileSize(0),
          m_pFileData(nullptr),
          m_avgSeekFrameCount(0),
          m_curFrameIndex(getMinFrameIndex()),
          m_madSynthCount(0),
          m_leftoverBuffer(kMaxBytesPerMp3Frame + MAD_BUFFER_GUARD) {
    m_seekFrameList.reserve(kSeekFrameListCapacity);
    initDecoding();
}

SoundSourceMp3::~SoundSourceMp3() {
    close();
    finishDecoding();
}

void SoundSourceMp3::initDecoding() {
    mad_stream_init(&m_madStream);
    mad_frame_init(&m_madFrame);
    mad_synth_init(&m_madSynth);
}

void SoundSourceMp3::finishDecoding() {
    m_madSynthCount = 0;
    mad_synth_finish(&m_madSynth);
    mad_frame_finish(&m_madFrame);
    mad_stream_finish(&m_madStream);
}

SoundSource::OpenResult SoundSourceMp3::tryOpen(const AudioSourceConfig& /*audioSrcCfg*/) {
    DEBUG_ASSERT(!hasValidChannelCount());
    DEBUG_ASSERT(!hasValidSamplingRate());

    DEBUG_ASSERT(!m_file.isOpen());
    if (!m_file.open(QIODevice::ReadOnly)) {
        kLogger.warning() << "Failed to open file:" << m_file.fileName();
        return OpenResult::FAILED;
    }

    // Get a pointer to the file using memory mapped IO
    m_fileSize = m_file.size();
    m_pFileData = m_file.map(0, m_fileSize);
    // NOTE(uklotzde): If the file disappears unexpectedly while mapped
    // a SIGBUS error might occur that is not handled and will terminate
    // Mixxx immediately. This behavior is documented in the manpage of
    // mmap(). It has already appeared due to hardware errors and is
    // described in the following bug report:
    // https://bugs.launchpad.net/mixxx/+bug/1452005

    // Transfer it to the mad stream-buffer:
    mad_stream_options(&m_madStream, MAD_OPTION_IGNORECRC);
    mad_stream_buffer(&m_madStream, m_pFileData, m_fileSize);
    DEBUG_ASSERT(m_pFileData == m_madStream.this_frame);

    DEBUG_ASSERT(m_seekFrameList.empty());
    m_avgSeekFrameCount = 0;
    m_curFrameIndex = getMinFrameIndex();
    int headerPerSamplingRate[kSamplingRateCount];
    for (int i = 0; i < kSamplingRateCount; ++i) {
        headerPerSamplingRate[i] = 0;
    }

    // Decode all the headers and calculate audio properties

    unsigned long sumBitrate = 0;

    mad_header madHeader;
    mad_header_init(&madHeader);

    SINT maxChannelCount = getChannelCount();
    do {
        if (!decodeFrameHeader(&madHeader, &m_madStream, true)) {
            if (isStreamValid(m_madStream)) {
                // Skip frame
                continue;
            } else {
                // Abort decoding
                break;
            }
        }

        // Grab data from madHeader
        const unsigned int madSampleRate = madHeader.samplerate;

        // TODO(XXX): Replace DEBUG_ASSERT with static_assert
        // MAD must not change its enum values!
        DEBUG_ASSERT(MAD_UNITS_8000_HZ == 8000);
        const mad_units madUnits = static_cast<mad_units>(madSampleRate);

        const long madFrameLength = mad_timer_count(madHeader.duration, madUnits);
        if (0 >= madFrameLength) {
            kLogger.warning() << "Skipping MP3 frame with invalid length"
                    << madFrameLength
                    << "in:" << m_file.fileName();
            // Skip frame
            continue;
        }

        const SINT madChannelCount = MAD_NCHANNELS(&madHeader);
        if (isValidChannelCount(maxChannelCount) && (madChannelCount != maxChannelCount)) {
            kLogger.warning() << "Differing number of channels"
                    << madChannelCount << "<>" << maxChannelCount
                    << "in some MP3 frame headers:"
                    << m_file.fileName();
        }
        maxChannelCount = math_max(madChannelCount, maxChannelCount);

        const int samplingRateIndex = getIndexBySamplingRate(madSampleRate);
        if (samplingRateIndex >= kSamplingRateCount) {
            kLogger.warning() << "Invalid sample rate:" << m_file.fileName()
                    << madSampleRate;
            // Abort
            mad_header_finish(&madHeader);
            return OpenResult::FAILED;
        }
        // Count valid frames separated by its sampling rate
        headerPerSamplingRate[samplingRateIndex]++;

        addSeekFrame(m_curFrameIndex, m_madStream.this_frame);

        // Accumulate data from the header
        sumBitrate += madHeader.bitrate;

        // Update current stream position
        m_curFrameIndex += madFrameLength;

        DEBUG_ASSERT(m_madStream.this_frame);
        DEBUG_ASSERT(0 <= (m_madStream.this_frame - m_pFileData));
    } while (quint64(m_madStream.this_frame - m_pFileData) < m_fileSize);

    mad_header_finish(&madHeader);

    if (MAD_ERROR_NONE != m_madStream.error) {
        // Unreachable code for recoverable errors
        DEBUG_ASSERT(!MAD_RECOVERABLE(m_madStream.error));
        if (MAD_ERROR_BUFLEN != m_madStream.error) {
            kLogger.warning() << "Unrecoverable MP3 header error:"
                    << mad_stream_errorstr(&m_madStream);
            // Abort
            return OpenResult::FAILED;
        }
    }

    if (m_seekFrameList.empty()) {
        // This is not a working MP3 file.
        kLogger.warning() << "SSMP3: This is not a working MP3 file:"
                << m_file.fileName();
        // Abort
        return OpenResult::FAILED;
    }
    DEBUG_ASSERT(m_seekFrameList.front().frameIndex == getMinFrameIndex());

    int mostCommonSamplingRateIndex = kSamplingRateCount; // invalid
    int mostCommonSamplingRateCount = 0;
    int differentRates = 0;
    for (int i = 0; i < kSamplingRateCount; ++i) {
        // Find most common sampling rate
        if (mostCommonSamplingRateCount < headerPerSamplingRate[i]) {
            mostCommonSamplingRateCount = headerPerSamplingRate[i];
            mostCommonSamplingRateIndex = i;
            differentRates++;
        }
    }

    if (differentRates > 1) {
        kLogger.warning() << "Differing sampling rate in some headers:"
                   << m_file.fileName();
        for (int i = 0; i < kSamplingRateCount; ++i) {
            if (0 < headerPerSamplingRate[i]) {
                kLogger.warning() << headerPerSamplingRate[i] << "MP3 headers with sampling rate" << getSamplingRateByIndex(i);
            }
        }

        kLogger.warning() << "MP3 files with varying sample rate are not supported!";
        kLogger.warning() << "Since this happens most likely due to a corrupt file";
        kLogger.warning() << "Mixxx tries to plays it with the most common sample rate for this file";
    }

    if (mostCommonSamplingRateIndex < kSamplingRateCount) {
        setSamplingRate(getSamplingRateByIndex(mostCommonSamplingRateIndex));
    } else {
        kLogger.warning() << "No single valid sampling rate in header";
        // Abort
        return OpenResult::FAILED;
    }

    // Initialize the AudioSource
    setChannelCount(maxChannelCount);
    setFrameCount(m_curFrameIndex);

    // Calculate average values
    m_avgSeekFrameCount = getFrameCount() / m_seekFrameList.size();
    const unsigned long avgBitrate = sumBitrate / m_seekFrameList.size();
    setBitrate(avgBitrate / 1000);

    // Terminate m_seekFrameList
    addSeekFrame(m_curFrameIndex, 0);
    DEBUG_ASSERT(m_seekFrameList.back().frameIndex == getMaxFrameIndex());

    // Restart decoding at the beginning of the audio stream
    restartDecoding(m_seekFrameList.front());

    if (m_curFrameIndex != getMinFrameIndex()) {
        kLogger.warning() << "Failed to start decoding:" << m_file.fileName();
        // Abort
        return OpenResult::FAILED;
    }

    return OpenResult::SUCCEEDED;
}

void SoundSourceMp3::close() {
    finishDecoding();

    if (m_pFileData) {
        m_file.unmap(m_pFileData);
        m_pFileData = nullptr;
    }

    m_file.close();

    m_seekFrameList.clear();

    // Re-init the decoder, because the SoundSource might be reopened and
    // the destructor calls finishDecoding() after close().
    initDecoding();
}

void SoundSourceMp3::restartDecoding(
        const SeekFrameType& seekFrame) {
    kLogger.debug() << "restartDecoding @" << seekFrame.frameIndex;

    // Discard decoded output
    m_madSynthCount = 0;

    if (getMinFrameIndex() == seekFrame.frameIndex) {
        mad_frame_finish(&m_madFrame);
        mad_synth_finish(&m_madSynth);
    }
    mad_stream_finish(&m_madStream);

    mad_stream_init(&m_madStream);
    mad_stream_options(&m_madStream, MAD_OPTION_IGNORECRC);
    if (getMinFrameIndex() == seekFrame.frameIndex) {
        mad_synth_init(&m_madSynth);
        mad_frame_init(&m_madFrame);
    }

    // Fill input buffer
    mad_stream_buffer(&m_madStream, seekFrame.pInputData,
            m_fileSize - (seekFrame.pInputData - m_pFileData));

    if (getMinFrameIndex() < seekFrame.frameIndex) {
        // Muting is done here to eliminate potential pops/clicks
        // from skipping Rob Leslie explains why here:
        // http://www.mars.org/mailman/public/mad-dev/2001-August/000321.html
        mad_frame_mute(&m_madFrame);
        mad_synth_mute(&m_madSynth);
    }

    if (decodeFrameHeader(&m_madFrame.header, &m_madStream, false)
            && isStreamValid(m_madStream)) {
        m_curFrameIndex = seekFrame.frameIndex;
    } else {
        // Failure -> Seek to EOF
        m_curFrameIndex = getMaxFrameIndex();
    }
}

void SoundSourceMp3::addSeekFrame(
        SINT frameIndex,
        const unsigned char* pInputData) {
    DEBUG_ASSERT(m_seekFrameList.empty() ||
            (m_seekFrameList.back().frameIndex < frameIndex));
    DEBUG_ASSERT(m_seekFrameList.empty() ||
            (nullptr == pInputData) ||
            (0 < (pInputData - m_seekFrameList.back().pInputData)));
    SeekFrameType seekFrame;
    seekFrame.pInputData = pInputData;
    seekFrame.frameIndex = frameIndex;
    m_seekFrameList.push_back(seekFrame);
}

SINT SoundSourceMp3::findSeekFrameIndex(
        SINT frameIndex) const {
    // Check preconditions
    DEBUG_ASSERT(0 < m_avgSeekFrameCount);
    DEBUG_ASSERT(!m_seekFrameList.empty());
    DEBUG_ASSERT(getMinFrameIndex() == m_seekFrameList.front().frameIndex);
    DEBUG_ASSERT(getMaxFrameIndex() == m_seekFrameList.back().frameIndex);

    SINT lowerBound =
            0;
    SINT upperBound =
            m_seekFrameList.size();
    DEBUG_ASSERT(lowerBound < upperBound);

    // Initial guess based on average frame size
    SINT seekFrameIndex =
            frameIndex / m_avgSeekFrameCount;
    if (seekFrameIndex >= upperBound) {
        seekFrameIndex = upperBound - 1;
    }

    while ((upperBound - lowerBound) > 1) {
        DEBUG_ASSERT(seekFrameIndex >= lowerBound);
        DEBUG_ASSERT(seekFrameIndex < upperBound);
        DEBUG_ASSERT(m_seekFrameList[lowerBound].frameIndex <= frameIndex);
        if (m_seekFrameList[seekFrameIndex].frameIndex <= frameIndex) {
            lowerBound = seekFrameIndex;
        } else {
            upperBound = seekFrameIndex;
        }
        // Next guess halfway between lower and upper bound
        seekFrameIndex = lowerBound + (upperBound - lowerBound) / 2;
    }

    // Check postconditions
    DEBUG_ASSERT(seekFrameIndex == lowerBound);
    DEBUG_ASSERT(SINT(m_seekFrameList.size()) > seekFrameIndex);
    DEBUG_ASSERT(m_seekFrameList[seekFrameIndex].frameIndex <= frameIndex);
    DEBUG_ASSERT(((seekFrameIndex + 1) >= SINT(m_seekFrameList.size())) ||
            (m_seekFrameList[seekFrameIndex + 1].frameIndex > frameIndex));

    return seekFrameIndex;
}

SINT SoundSourceMp3::seekSampleFrame(SINT frameIndex) {
    DEBUG_ASSERT(isValidFrameIndex(m_curFrameIndex));

    if (frameIndex >= getMaxFrameIndex()) {
        // EOF reached
        m_curFrameIndex = getMaxFrameIndex();
        return m_curFrameIndex;
    }

    if (frameIndex == m_curFrameIndex) {
        return m_curFrameIndex;
    }

    SINT seekFrameIndex = findSeekFrameIndex(
            frameIndex);
    DEBUG_ASSERT(SINT(m_seekFrameList.size()) > seekFrameIndex);
    const SINT curSeekFrameIndex = findSeekFrameIndex(
            m_curFrameIndex);
    DEBUG_ASSERT(SINT(m_seekFrameList.size()) > curSeekFrameIndex);
    // some consistency checks
    DEBUG_ASSERT((curSeekFrameIndex >= seekFrameIndex) || (m_curFrameIndex < frameIndex));
    DEBUG_ASSERT((curSeekFrameIndex <= seekFrameIndex) || (m_curFrameIndex > frameIndex));
    if ((getMaxFrameIndex() <= m_curFrameIndex) || // out of range
            (frameIndex < m_curFrameIndex) || // seek backward
            (seekFrameIndex > (curSeekFrameIndex + kMp3SeekFramePrefetchCount))) { // jump forward

        // Adjust the seek frame index for prefetching
        if (kMp3SeekFramePrefetchCount < seekFrameIndex) {
            // Restart decoding kMp3SeekFramePrefetchCount seek frames
            // before the expected sync position
            seekFrameIndex -= kMp3SeekFramePrefetchCount;
        } else {
            // Restart decoding at the beginning of the audio stream
            seekFrameIndex = 0;
        }

        restartDecoding(m_seekFrameList[seekFrameIndex]);

        DEBUG_ASSERT(findSeekFrameIndex(m_curFrameIndex) == seekFrameIndex);
    }

    // Decoding starts at or before the actual target position
    DEBUG_ASSERT(m_curFrameIndex <= frameIndex);

    // Skip (= decode and discard) all samples up to the target position
    if (m_curFrameIndex < frameIndex) {
        skipSampleFrames(frameIndex - m_curFrameIndex);
        DEBUG_ASSERT(m_curFrameIndex <= frameIndex);
        if (m_curFrameIndex < frameIndex) {
            kLogger.warning() << "Failed to prefetch sample data while seeking:"
                    << m_curFrameIndex << "<" << frameIndex;
        }
    }

    DEBUG_ASSERT(isValidFrameIndex(m_curFrameIndex));
    return m_curFrameIndex;
}

SINT SoundSourceMp3::readSampleFrames(
        SINT numberOfFrames, CSAMPLE* sampleBuffer) {
    return readSampleFrames(numberOfFrames,
            sampleBuffer, frames2samples(numberOfFrames),
            false);
}

SINT SoundSourceMp3::readSampleFramesStereo(
        SINT numberOfFrames, CSAMPLE* sampleBuffer,
        SINT sampleBufferSize) {
    return readSampleFrames(numberOfFrames,
            sampleBuffer, sampleBufferSize,
            true);
}

SINT SoundSourceMp3::readSampleFrames(
        SINT numberOfFrames, CSAMPLE* sampleBuffer,
        SINT sampleBufferSize, bool readStereoSamples) {
    DEBUG_ASSERT(isValidFrameIndex(m_curFrameIndex));
    DEBUG_ASSERT(getSampleBufferSize(numberOfFrames, readStereoSamples) <= sampleBufferSize);

    const SINT numberOfFramesTotal = math_min(
            numberOfFrames, getMaxFrameIndex() - m_curFrameIndex);

    CSAMPLE* pSampleBuffer = sampleBuffer;
    SINT numberOfFramesRemaining = numberOfFramesTotal;
    while (0 < numberOfFramesRemaining) {
        if (0 >= m_madSynthCount) {
            // When all decoded output data has been consumed...
            DEBUG_ASSERT(0 == m_madSynthCount);
            // ...decode the next MP3 frame
            DEBUG_ASSERT(m_madStream.buffer);
            DEBUG_ASSERT(m_madStream.this_frame);

            // WARNING: Correctly evaluating and handling the result
            // of mad_frame_decode() has proven to be extremely tricky.
            // Don't change anything at the following lines of code
            // unless you know what you are doing!!!
            unsigned char const* pMadThisFrame = m_madStream.this_frame;
            if (mad_frame_decode(&m_madFrame, &m_madStream)) {
                // Something went wrong when decoding the frame...
                if (MAD_ERROR_BUFLEN == m_madStream.error) {
                    // Abort when reaching the end of the stream
                    DEBUG_ASSERT(isUnrecoverableError(m_madStream));
                    if (m_madStream.next_frame != nullptr) {
                        // Decoding of the last MP3 frame fails if it is not padded
                        // with 0 bytes. MAD requires that the last frame ends with
                        // at least MAD_BUFFER_GUARD of 0 bytes.
                        // https://www.mars.org/pipermail/mad-dev/2001-May/000262.html
                        // "The reason for MAD_BUFFER_GUARD has to do with the way decoding is performed.
                        // In Layer III, Huffman decoding may inadvertently read a few bytes beyond the
                        // end of the buffer in the case of certain invalid input. This is not detected
                        // until after the fact. To prevent this from causing problems, and also to
                        // ensure the next frame's main_data_begin pointer is always accessible, MAD
                        // requires MAD_BUFFER_GUARD (currently 8) bytes to be present in the buffer past
                        // the end of the current frame in order to decode the frame."
                        const SINT remainingBytes = m_madStream.bufend - m_madStream.next_frame;
                        DEBUG_ASSERT(remainingBytes <= kMaxBytesPerMp3Frame); // only last MP3 frame
                        const SINT leftoverBytes = remainingBytes + MAD_BUFFER_GUARD;
                        if ((remainingBytes > 0) && (leftoverBytes <= SINT(m_leftoverBuffer.size()))) {
                            // Copy the data of the last MP3 frame into the leftover buffer...
                            unsigned char* pLeftoverBuffer = &*m_leftoverBuffer.begin();
                            std::copy(m_madStream.next_frame, m_madStream.next_frame + remainingBytes, pLeftoverBuffer);
                            // ...append the required guard bytes...
                            std::fill(pLeftoverBuffer + remainingBytes, pLeftoverBuffer + leftoverBytes, 0);
                            // ...and retry decoding.
                            mad_stream_buffer(&m_madStream, pLeftoverBuffer, leftoverBytes);
                            m_madStream.error = MAD_ERROR_NONE;
                            continue;
                        }
                        if (m_curFrameIndex < getMaxFrameIndex()) {
                            kLogger.warning() << "Failed to decode the end of the MP3 stream"
                                    << m_curFrameIndex << "<" << getMaxFrameIndex();
                        }
                    }
                    break;
                }
                if (isUnrecoverableError(m_madStream)) {
                    kLogger.warning() << "Unrecoverable MP3 frame decoding error:"
                            << mad_stream_errorstr(&m_madStream);
                    // Abort decoding
                    break;
                }
                if (isRecoverableError(m_madStream)) {
                    if (pMadThisFrame != m_madStream.this_frame) {
                        // Ignore all recoverable errors (and especially
                        // "lost synchronization" warnings) while skipping
                        // over prefetched frames after seeking.
                        if (pSampleBuffer) {
                            kLogger.warning() << "Recoverable MP3 frame decoding error:"
                                    << mad_stream_errorstr(&m_madStream);
                        } else {
                            // Decoded samples will simply be discarded
                            kLogger.debug() << "Recoverable MP3 frame decoding error while skipping:"
                                << mad_stream_errorstr(&m_madStream);
                        }
                    }
                    // Continue decoding
                }
            }
            if (pMadThisFrame == m_madStream.this_frame) {
                kLogger.debug() << "Retry decoding MP3 frame @" << m_curFrameIndex;
                // Retry decoding
                continue;
            }

            DEBUG_ASSERT(isStreamValid(m_madStream));

#ifndef QT_NO_DEBUG_OUTPUT
            const SINT madFrameChannelCount = MAD_NCHANNELS(&m_madFrame.header);
            if (madFrameChannelCount != getChannelCount()) {
                kLogger.debug() << "MP3 frame header with mismatching number of channels"
                        << madFrameChannelCount << "<>" << getChannelCount();
            }
#endif

            // Once decoded the frame is synthesized to PCM samples
            mad_synth_frame(&m_madSynth, &m_madFrame);
#ifndef QT_NO_DEBUG_OUTPUT
            const SINT madSynthSampleRate =  m_madSynth.pcm.samplerate;
            if (madSynthSampleRate != getSamplingRate()) {
                kLogger.debug() << "Reading MP3 data with different sampling rate"
                        << madSynthSampleRate << "<>" << getSamplingRate();
            }
#endif
            m_madSynthCount = m_madSynth.pcm.length;
            DEBUG_ASSERT(0 < m_madSynthCount);
        }

        const SINT synthReadCount = math_min(
                m_madSynthCount, numberOfFramesRemaining);
        if (pSampleBuffer) {
            DEBUG_ASSERT(m_madSynthCount <= m_madSynth.pcm.length);
            const SINT madSynthOffset =
                    m_madSynth.pcm.length - m_madSynthCount;
            DEBUG_ASSERT(madSynthOffset < m_madSynth.pcm.length);
            const SINT madSynthChannelCount = m_madSynth.pcm.channels;
            DEBUG_ASSERT(0 < madSynthChannelCount);
            DEBUG_ASSERT(madSynthChannelCount <= getChannelCount());
#ifndef QT_NO_DEBUG_OUTPUT
            if (madSynthChannelCount != getChannelCount()) {
                kLogger.debug() << "Reading MP3 data with different number of channels"
                        << madSynthChannelCount << "<>" << getChannelCount();
            }
#endif
            if (kChannelCountMono == madSynthChannelCount) {
                // MP3 frame contains a mono signal
                if (readStereoSamples || (kChannelCountStereo == getChannelCount())) {
                    // The reader explicitly requested a stereo signal
                    // or the AudioSource itself provides a stereo signal.
                    // Mono -> Stereo: Copy 1st channel twice
                    for (SINT i = 0; i < synthReadCount; ++i) {
                        const CSAMPLE sampleValue = madScaleSampleValue(
                                m_madSynth.pcm.samples[0][madSynthOffset + i]);
                        *pSampleBuffer++ = sampleValue;
                        *pSampleBuffer++ = sampleValue;
                    }
                } else {
                    // Mono -> Mono: Copy 1st channel
                    for (SINT i = 0; i < synthReadCount; ++i) {
                        const CSAMPLE sampleValue = madScaleSampleValue(
                                m_madSynth.pcm.samples[0][madSynthOffset + i]);
                        *pSampleBuffer++ = sampleValue;
                    }
                }
            } else {
                // MP3 frame contains a stereo signal
                DEBUG_ASSERT(kChannelCountStereo == madSynthChannelCount);
                // If the MP3 frame contains a stereo signal then the whole
                // AudioSource must also provide 2 channels, because the
                // maximum channel count of all MP3 frames is used.
                DEBUG_ASSERT(kChannelCountStereo == getChannelCount());
                // Stereo -> Stereo: Copy 1st + 2nd channel
                for (SINT i = 0; i < synthReadCount; ++i) {
                    *pSampleBuffer++ = madScaleSampleValue(
                            m_madSynth.pcm.samples[0][madSynthOffset + i]);
                    *pSampleBuffer++ = madScaleSampleValue(
                            m_madSynth.pcm.samples[1][madSynthOffset + i]);
                }
            }
        }
        // consume decoded output data
        m_madSynthCount -= synthReadCount;
        m_curFrameIndex += synthReadCount;
        numberOfFramesRemaining -= synthReadCount;
    }

    DEBUG_ASSERT(isValidFrameIndex(m_curFrameIndex));
    DEBUG_ASSERT(numberOfFramesTotal >= numberOfFramesRemaining);
    return numberOfFramesTotal - numberOfFramesRemaining;
}

QString SoundSourceProviderMp3::getName() const {
    return "MAD: MPEG Audio Decoder";
}

QStringList SoundSourceProviderMp3::getSupportedFileExtensions() const {
    QStringList supportedFileExtensions;
    supportedFileExtensions.append("mp3");
    return supportedFileExtensions;
}

} // namespace mixxx
