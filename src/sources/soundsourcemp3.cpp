#include "sources/soundsourcemp3.h"
#include "sources/mp3decoding.h"

#include "util/logger.h"
#include "util/math.h"

#include <id3tag.h>

namespace mixxx {

namespace {

const Logger kLogger("SoundSourceMp3");

// MP3 does only support 1 or 2 channels
constexpr SINT kChannelCountMax = 2;

constexpr SINT kMaxBytesPerMp3Frame = 1441;

// mp3 supports 9 different sample rates
constexpr int kSampleRateCount = 9;

int getIndexBySampleRate(audio::SampleRate sampleRate) {
    switch (sampleRate) {
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
        // unsupported sample rate
        return kSampleRateCount;
    }
}

audio::SampleRate getSampleRateByIndex(int sampleRateIndex) {
    switch (sampleRateIndex) {
    case 0:
        return audio::SampleRate(8000);
    case 1:
        return audio::SampleRate(11025);
    case 2:
        return audio::SampleRate(12000);
    case 3:
        return audio::SampleRate(16000);
    case 4:
        return audio::SampleRate(22050);
    case 5:
        return audio::SampleRate(24000);
    case 6:
        return audio::SampleRate(32000);
    case 7:
        return audio::SampleRate(44100);
    case 8:
        return audio::SampleRate(48000);
    default:
        // index out of range
        return audio::SampleRate();
    }
}

constexpr CSAMPLE kMadScale = CSAMPLE_PEAK / CSAMPLE(MAD_F_ONE);

inline CSAMPLE madScaleSampleValue(mad_fixed_t sampleValue) {
    return sampleValue * kMadScale;
}

// Optimization: Reserve initial capacity for seek frame list
constexpr SINT kMinutesPerFile = 10;        // enough for the majority of files (tunable)
constexpr SINT kSecondsPerMinute = 60;      // fixed
constexpr SINT kMaxMp3FramesPerSecond = 39; // fixed: 1 MP3 frame = 26 ms -> ~ 1000 / 26
constexpr SINT kSeekFrameListCapacity =
        kMinutesPerFile * kSecondsPerMinute * kMaxMp3FramesPerSecond;

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
    DEBUG_ASSERT(pMadStream);
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

//static
const QString SoundSourceProviderMp3::kDisplayName = QStringLiteral("MAD: MPEG Audio Decoder");

//static
const QStringList SoundSourceProviderMp3::kSupportedFileExtensions = {
        QStringLiteral("mp3"),
};

SoundSourceMp3::SoundSourceMp3(const QUrl& url)
        : SoundSource(url),
          m_file(getLocalFileName()),
          m_fileSize(0),
          m_pFileData(nullptr),
          m_avgSeekFrameCount(0),
          m_curFrameIndex(0),
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

SoundSource::OpenResult SoundSourceMp3::tryOpen(
        OpenMode /*mode*/,
        const OpenParams& /*config*/) {
    DEBUG_ASSERT(!m_file.isOpen());
    if (!m_file.open(QIODevice::ReadOnly)) {
        kLogger.warning() << "Failed to open file:" << m_file.fileName();
        return OpenResult::Failed;
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
    m_curFrameIndex = 0;
    int headerPerSampleRate[kSampleRateCount];
    for (int i = 0; i < kSampleRateCount; ++i) {
        headerPerSampleRate[i] = 0;
    }

    // Decode all the headers and calculate audio properties

    // The average bitrate is calculated by summing up the bitrate
    // (in bps <= 320_000) for each counted sample frame and dividing
    // by the number of counted sample frames. The maximum value for
    // the nominator is 320_000 * number of sample frames which is
    // sufficient for 2^63-1 / 320_000 bps / 48_000 Hz = almost
    // 7000 days of audio duration.
    quint64 sumBitrateFrames = 0; // nominator
    quint64 cntBitrateFrames = 0; // denominator

    mad_header madHeader;
    mad_header_init(&madHeader);

    auto maxChannelCount = audio::ChannelCount();
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

        const audio::ChannelCount madChannelCount(MAD_NCHANNELS(&madHeader));
        if (madChannelCount.isValid()) {
            if (maxChannelCount.isValid() && (madChannelCount != maxChannelCount)) {
                kLogger.warning()
                        << "Differing number of channels"
                        << madChannelCount << "<>" << maxChannelCount
                        << "in MP3 frame headers:"
                        << m_file.fileName();
            }
            maxChannelCount = math_max(madChannelCount, maxChannelCount);
        } else {
            kLogger.warning()
                    << "Missing number of channels in MP3 frame header:"
                    << m_file.fileName();
        }

        const int sampleRateIndex = getIndexBySampleRate(
                audio::SampleRate(madSampleRate));
        if (sampleRateIndex >= kSampleRateCount) {
            kLogger.warning() << "Invalid sample rate:" << m_file.fileName()
                              << madSampleRate;
            // Abort
            mad_header_finish(&madHeader);
            return OpenResult::Failed;
        }
        // Count valid frames separated by its sample rate
        headerPerSampleRate[sampleRateIndex]++;

        addSeekFrame(m_curFrameIndex, m_madStream.this_frame);

        // Accumulate data from the header
        if (audio::Bitrate(madHeader.bitrate).isValid()) {
            // Accumulate the bitrate per decoded sample frame to calculate
            // a weighted average for the whole file (see below)
            sumBitrateFrames += static_cast<quint64>(madHeader.bitrate) * static_cast<quint64>(madFrameLength);
            cntBitrateFrames += madFrameLength;
        }

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
            return OpenResult::Failed;
        }
    }

    if (m_seekFrameList.empty()) {
        // This is not a working MP3 file.
        kLogger.warning() << "This is not a working MP3 file:"
                          << m_file.fileName();
        // Abort
        return OpenResult::Failed;
    }
    DEBUG_ASSERT(m_seekFrameList.front().frameIndex == 0);

    int mostCommonSampleRateIndex = kSampleRateCount; // invalid
    int mostCommonSampleRateCount = 0;
    int differentRates = 0;
    for (int i = 0; i < kSampleRateCount; ++i) {
        // Find most common sample rate
        if (mostCommonSampleRateCount < headerPerSampleRate[i]) {
            mostCommonSampleRateCount = headerPerSampleRate[i];
            mostCommonSampleRateIndex = i;
            differentRates++;
        }
    }

    if (differentRates > 1) {
        kLogger.warning() << "Differing sample rate in some headers:"
                          << m_file.fileName();
        for (int i = 0; i < kSampleRateCount; ++i) {
            if (0 < headerPerSampleRate[i]) {
                kLogger.warning() << headerPerSampleRate[i] << "MP3 headers with sample rate" << getSampleRateByIndex(i);
            }
        }

        kLogger.warning() << "MP3 files with varying sample rate are not supported!";
        kLogger.warning() << "Since this happens most likely due to a corrupt file";
        kLogger.warning() << "Mixxx tries to plays it with the most common sample rate for this file";
    }

    // Initialize the AudioSource
    if (!maxChannelCount.isValid() || (maxChannelCount > kChannelCountMax)) {
        kLogger.warning()
                << "Invalid number of channels"
                << maxChannelCount
                << "in MP3 file:"
                << m_file.fileName();
        // Abort
        return OpenResult::Failed;
    }
    initChannelCountOnce(maxChannelCount);
    if (mostCommonSampleRateIndex > kSampleRateCount) {
        kLogger.warning()
                << "Unknown sample rate in MP3 file:"
                << m_file.fileName();
        // Abort
        return OpenResult::Failed;
    }
    initSampleRateOnce(getSampleRateByIndex(mostCommonSampleRateIndex));
    initFrameIndexRangeOnce(IndexRange::forward(0, m_curFrameIndex));

    // Calculate average bitrate values
    DEBUG_ASSERT(m_seekFrameList.size() > 0); // see above
    m_avgSeekFrameCount = frameLength() / m_seekFrameList.size();
    if (cntBitrateFrames > 0) {
        const unsigned long avgBitrate = sumBitrateFrames / cntBitrateFrames;
        initBitrateOnce(avgBitrate / 1000); // bps -> kbps
    } else {
        kLogger.warning() << "Bitrate cannot be calculated from headers";
    }

    // Terminate m_seekFrameList
    addSeekFrame(m_curFrameIndex, 0);
    DEBUG_ASSERT(m_seekFrameList.back().frameIndex == frameIndexMax());

    // Restart decoding at the beginning of the audio stream
    restartDecoding(m_seekFrameList.front());

    if (m_curFrameIndex != frameIndexMin()) {
        kLogger.warning() << "Failed to start decoding:" << m_file.fileName();
        // Abort
        return OpenResult::Failed;
    }

    return OpenResult::Succeeded;
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
    if (kLogger.debugEnabled()) {
        kLogger.debug() << "restartDecoding @" << seekFrame.frameIndex;
    }

    // Discard decoded output
    m_madSynthCount = 0;

    if (frameIndexMin() == seekFrame.frameIndex) {
        mad_frame_finish(&m_madFrame);
        mad_synth_finish(&m_madSynth);
    }
    mad_stream_finish(&m_madStream);

    mad_stream_init(&m_madStream);
    mad_stream_options(&m_madStream, MAD_OPTION_IGNORECRC);
    if (frameIndexMin() == seekFrame.frameIndex) {
        mad_synth_init(&m_madSynth);
        mad_frame_init(&m_madFrame);
    }

    // Fill input buffer
    mad_stream_buffer(&m_madStream, seekFrame.pInputData, m_fileSize - (seekFrame.pInputData - m_pFileData));

    if (frameIndexMin() < seekFrame.frameIndex) {
        // Muting is done here to eliminate potential pops/clicks
        // from skipping Rob Leslie explains why here:
        // http://www.mars.org/mailman/public/mad-dev/2001-August/000321.html
        mad_frame_mute(&m_madFrame);
        mad_synth_mute(&m_madSynth);
    }

    if (decodeFrameHeader(&m_madFrame.header, &m_madStream, false) && isStreamValid(m_madStream)) {
        m_curFrameIndex = seekFrame.frameIndex;
    } else {
        // Failure -> Seek to EOF
        m_curFrameIndex = frameIndexMax();
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
    DEBUG_ASSERT(frameIndexMin() == m_seekFrameList.front().frameIndex);
    DEBUG_ASSERT(frameIndexMax() == m_seekFrameList.back().frameIndex);

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

ReadableSampleFrames SoundSourceMp3::readSampleFramesClamped(
        WritableSampleFrames writableSampleFrames) {
    const SINT firstFrameIndex = writableSampleFrames.frameIndexRange().start();

    if ((m_curFrameIndex != firstFrameIndex)) {
        SINT seekFrameIndex = findSeekFrameIndex(firstFrameIndex);
        DEBUG_ASSERT(SINT(m_seekFrameList.size()) > seekFrameIndex);
        const SINT curSeekFrameIndex = findSeekFrameIndex(m_curFrameIndex);
        DEBUG_ASSERT(SINT(m_seekFrameList.size()) > curSeekFrameIndex);
        // some consistency checks
        DEBUG_ASSERT((curSeekFrameIndex >= seekFrameIndex) || (m_curFrameIndex < firstFrameIndex));
        DEBUG_ASSERT((curSeekFrameIndex <= seekFrameIndex) || (m_curFrameIndex > firstFrameIndex));
        if ((frameIndexMax() <= m_curFrameIndex) ||                                    // out of range
                (firstFrameIndex < m_curFrameIndex) ||                                 // seek backward
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

        // Decoding starts before the actual target position
        DEBUG_ASSERT(m_curFrameIndex <= firstFrameIndex);
        const auto precedingFrames =
                IndexRange::between(m_curFrameIndex, firstFrameIndex);
        if (!precedingFrames.empty() && (precedingFrames != readSampleFramesClamped(WritableSampleFrames(precedingFrames)).frameIndexRange())) {
            kLogger.warning()
                    << "Failed to skip preceding frames"
                    << precedingFrames;
            // Abort
            return ReadableSampleFrames(
                    IndexRange::between(
                            m_curFrameIndex,
                            m_curFrameIndex));
        }
    }
    DEBUG_ASSERT(m_curFrameIndex == firstFrameIndex);

    const SINT numberOfFramesTotal = writableSampleFrames.frameLength();

    CSAMPLE* pSampleBuffer = writableSampleFrames.writableData();
    SINT numberOfFramesRemaining = numberOfFramesTotal;
    SINT retryFrameIndex = numberOfFramesTotal;
    while (0 < numberOfFramesRemaining) {
        bool abortReading = false;

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
                        if (m_curFrameIndex < frameIndexMax()) {
                            kLogger.warning() << "Failed to decode the end of the MP3 stream"
                                              << m_curFrameIndex << "<" << frameIndexMax();
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
                        if (!pSampleBuffer ||
                                (m_madStream.error == MAD_ERROR_LOSTSYNC)) {
                            // Don't bother the user with warnings from recoverable
                            // errors while skipping decoded samples or that even
                            // might occur for files that are perfectly ok.
                            if (kLogger.debugEnabled()) {
                                kLogger.debug()
                                        << "Recoverable MP3 frame decoding error:"
                                        << mad_stream_errorstr(&m_madStream);
                            }
                        } else {
                            kLogger.info() << "Recoverable MP3 frame decoding error:"
                                           << mad_stream_errorstr(&m_madStream);
                        }
                    }
                    // Continue decoding
                }
            }
            if (pMadThisFrame == m_madStream.this_frame) {
                // Retry decoding, but only once for each position to
                // prevent infinite loops when decoding corrupt files
                if (retryFrameIndex != m_curFrameIndex) {
                    retryFrameIndex = m_curFrameIndex;
                    if (kLogger.debugEnabled()) {
                        kLogger.debug()
                                << "Retry decoding MP3 frame @"
                                << m_curFrameIndex;
                    }
                    continue;
                } else {
                    kLogger.warning()
                            << "Decoding MP3 frame @"
                            << m_curFrameIndex
                            << "failed again";
                    break;
                }
            }

            DEBUG_ASSERT(isStreamValid(m_madStream));

#ifndef QT_NO_DEBUG_OUTPUT
            const SINT madFrameChannelCount = MAD_NCHANNELS(&m_madFrame.header);
            if (madFrameChannelCount != getSignalInfo().getChannelCount()) {
                kLogger.warning() << "MP3 frame header with mismatching number of channels"
                                  << madFrameChannelCount << "<>" << getSignalInfo().getChannelCount()
                                  << " - aborting";
                abortReading = true;
            }
#endif

            // Once decoded the frame is synthesized to PCM samples
            mad_synth_frame(&m_madSynth, &m_madFrame);
#ifndef QT_NO_DEBUG_OUTPUT
            const SINT madSynthSampleRate = m_madSynth.pcm.samplerate;
            if (madSynthSampleRate != getSignalInfo().getSampleRate()) {
                kLogger.warning() << "Reading MP3 data with different sample rate"
                                  << madSynthSampleRate << "<>" << getSignalInfo().getSampleRate()
                                  << " - aborting";
                abortReading = true;
            }
#endif
            m_madSynthCount = m_madSynth.pcm.length;
            DEBUG_ASSERT(0 < m_madSynthCount);
        }

        if (abortReading) {
            // Refuse to continue for preventing crashes while
            // decoding/reading corrupt files
            break;
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
            DEBUG_ASSERT(madSynthChannelCount <= getSignalInfo().getChannelCount());
            if (madSynthChannelCount != getSignalInfo().getChannelCount()) {
                kLogger.warning() << "Reading MP3 data with different number of channels"
                                  << madSynthChannelCount << "<>" << getSignalInfo().getChannelCount();
            }
            if (madSynthChannelCount == 1) {
                // MP3 frame contains a mono signal
                if (getSignalInfo().getChannelCount() == 2) {
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
                DEBUG_ASSERT(madSynthChannelCount == 2);
                // If the MP3 frame contains a stereo signal then the whole
                // AudioSource must also provide 2 channels, because the
                // maximum channel count of all MP3 frames is used.
                DEBUG_ASSERT(getSignalInfo().getChannelCount() == 2);
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
    const SINT numberOfFrames = numberOfFramesTotal - numberOfFramesRemaining;
    return ReadableSampleFrames(
            IndexRange::forward(firstFrameIndex, numberOfFrames),
            SampleBuffer::ReadableSlice(
                    writableSampleFrames.writableData(),
                    std::min(writableSampleFrames.writableLength(), getSignalInfo().frames2samples(numberOfFrames))));
}

} // namespace mixxx
