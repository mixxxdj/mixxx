#include "sources/soundsourcemp3.h"

#include "util/math.h"

#include <id3tag.h>

namespace Mixxx {

namespace {

// In the worst case up to 29 MP3 frames need to be prefetched
// for accurate seeking:
// http://www.mars.org/mailman/public/mad-dev/2002-May/000634.html
const SINT kSeekFramePrefetchCount = 29;

const CSAMPLE kMadScale = AudioSource::kSampleValuePeak
        / CSAMPLE(MAD_F_ONE);

inline CSAMPLE madScale(mad_fixed_t sample) {
    return sample * kMadScale;

} // anonymous namespace

// Optimization: Reserve initial capacity for seek frame list
const SINT kMinutesPerFile = 10; // enough for the majority of files (tunable)
const SINT kSecondsPerMinute = 60; // fixed
const SINT kMaxMp3FramesPerSecond = 39; // fixed: 1 MP3 frame = 26 ms -> ~ 1000 / 26
const SINT kSeekFrameListCapacity = kMinutesPerFile
        * kSecondsPerMinute * kMaxMp3FramesPerSecond;

void logFrameHeader(QDebug logger, const mad_header& madHeader) {
    logger << "MP3 frame header |"
            << "layer:" << madHeader.layer
            << "mode:" << madHeader.mode
            << "#channels:" << MAD_NCHANNELS(&madHeader)
            << "#samples:" << MAD_NSBSAMPLES(&madHeader)
            << "bitrate:" << madHeader.bitrate
            << "samplerate:" << madHeader.samplerate
            << "flags:" << madHeader.flags;
}

inline bool isRecoverableError(int madError) {
    return MAD_RECOVERABLE(madError);
}

inline bool isUnrecoverableError(int madError) {
    return (MAD_ERROR_NONE != madError) && !isRecoverableError(madError);
}

inline bool isStreamValid(const mad_stream& madStream) {
    return !isUnrecoverableError(madStream.error);
}

bool decodeFrameHeader(
        mad_header* pMadHeader,
        mad_stream* pMadStream,
        bool skipId3Tag) {
    DEBUG_ASSERT(isStreamValid(*pMadStream));
    const int decodeResult = mad_header_decode(pMadHeader, pMadStream);
    if (MAD_ERROR_BUFLEN == decodeResult) {
        // EOF
        return false;
    }
    if (isUnrecoverableError(decodeResult)) {
        DEBUG_ASSERT(!isStreamValid(*pMadStream));
        qWarning() << "Unrecoverable MP3 header decoding error:"
                << mad_stream_errorstr(pMadStream);
        return false;
    }
#ifndef QT_NO_DEBUG_OUTPUT
    logFrameHeader(qDebug(), *pMadHeader);
#endif
    if (isRecoverableError(decodeResult)) {
        if ((MAD_ERROR_LOSTSYNC == decodeResult) && skipId3Tag) {
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
        qWarning() << "Recoverable MP3 header decoding error:"
                << mad_stream_errorstr(pMadStream);
        logFrameHeader(qWarning(), *pMadHeader);
        return false;
    }
    DEBUG_ASSERT(isStreamValid(*pMadStream));
    return true;
}

} // anonymous namespace

QList<QString> SoundSourceMp3::supportedFileExtensions() {
    QList<QString> list;
    list.push_back("mp3");
    return list;
}

SoundSourceMp3::SoundSourceMp3(QUrl url)
        : SoundSource(url, "mp3"),
          m_file(getLocalFileName()),
          m_fileSize(0),
          m_pFileData(NULL),
          m_avgSeekFrameCount(0),
          m_curFrameIndex(kFrameIndexMin),
          m_madSynthCount(0) {
    m_seekFrameList.reserve(kSeekFrameListCapacity);
}

SoundSourceMp3::~SoundSourceMp3() {
    close();
}

Result SoundSourceMp3::tryOpen(SINT /*channelCountHint*/) {
    DEBUG_ASSERT(!isChannelCountValid());
    DEBUG_ASSERT(!isFrameRateValid());

    DEBUG_ASSERT(!m_file.isOpen());
    if (!m_file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open file:" << m_file.fileName();
        return ERR;
    }

    // Get a pointer to the file using memory mapped IO
    m_fileSize = m_file.size();
    m_pFileData = m_file.map(0, m_fileSize);

    // Transfer it to the mad stream-buffer:
    mad_stream_init(&m_madStream);
    mad_stream_options(&m_madStream, MAD_OPTION_IGNORECRC);
    mad_stream_buffer(&m_madStream, m_pFileData, m_fileSize);
    DEBUG_ASSERT(m_pFileData == m_madStream.this_frame);
    mad_frame_init(&m_madFrame);
    mad_synth_init(&m_madSynth);

    DEBUG_ASSERT(m_seekFrameList.empty());
    m_avgSeekFrameCount = 0;
    m_curFrameIndex = kFrameIndexMin;

    // Decode all the headers and calculate audio properties

    mad_timer_t madDuration = mad_timer_zero;
    unsigned long sumBitrate = 0;

    mad_header madHeader;
    mad_header_init(&madHeader);

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
        const SINT madChannelCount = MAD_NCHANNELS(&madHeader);
        if (isChannelCountValid()) {
            // check for consistent number of channels
            if (getChannelCount() != madChannelCount) {
                qWarning() << "Differing number of channels in some headers:"
                        << m_file.fileName()
                        << madChannelCount << "<>" << getChannelCount();
                qWarning() << "MP3 files with varying channel configurations are not supported!";
                // Abort
                mad_header_finish(&madHeader);
                return ERR;
            }
        } else {
            // initially set the number of channels
            setChannelCount(madChannelCount);
        }
        const SINT madSampleRate = madHeader.samplerate;
        mad_units madUnits;
        switch (madSampleRate) {
        case 8000:
            madUnits = MAD_UNITS_8000_HZ;
            break;
        case 11025:
            madUnits = MAD_UNITS_11025_HZ;
            break;
        case 12000:
            madUnits = MAD_UNITS_12000_HZ;
            break;
        case 16000:
            madUnits = MAD_UNITS_16000_HZ;
            break;
        case 22050:
            madUnits = MAD_UNITS_22050_HZ;
            break;
        case 24000:
            madUnits = MAD_UNITS_24000_HZ;
            break;
        case 32000:
            madUnits = MAD_UNITS_32000_HZ;
            break;
        case 44100:
            madUnits = MAD_UNITS_44100_HZ;
            break;
        case 48000:
            madUnits = MAD_UNITS_48000_HZ;
            break;
        default:
            qWarning() << "Invalid sample rate:" << m_file.fileName()
                    << madSampleRate;
            // Abort
            mad_header_finish(&madHeader);
            return ERR;
        }
        if (isFrameRateValid()) {
            // check for consistent frame/sample rate
            if (getFrameRate() != madSampleRate) {
                qWarning() << "Differing sample rate in some headers:"
                        << m_file.fileName()
                        << madSampleRate << "<>" << getFrameRate();
                qWarning() << "MP3 files with varying sample rate are not supported!";
                // Abort
                mad_header_finish(&madHeader);
                return ERR;
            }
        } else {
            // initially set the frame/sample rate
            setFrameRate(madSampleRate);
        }

        addSeekFrame(m_curFrameIndex, m_madStream.this_frame);

        // Accumulate data from the header
        sumBitrate += madHeader.bitrate;
        mad_timer_add(&madDuration, madHeader.duration);

        // Update current stream position
        m_curFrameIndex = kFrameIndexMin +
                mad_timer_count(madDuration, madUnits);

        DEBUG_ASSERT(m_madStream.this_frame);
        DEBUG_ASSERT(0 < (m_madStream.this_frame - m_pFileData));
    } while (quint64(m_madStream.this_frame - m_pFileData) < m_fileSize);

    mad_header_finish(&madHeader);

    if (MAD_ERROR_NONE != m_madStream.error) {
        // Unreachable code for recoverable errors
        DEBUG_ASSERT(!MAD_RECOVERABLE(m_madStream.error));
        if (MAD_ERROR_BUFLEN != m_madStream.error) {
            qWarning() << "Unrecoverable MP3 header error:"
                    << mad_stream_errorstr(&m_madStream);
            // Abort
            return ERR;
        }
    }

    if (m_seekFrameList.empty()) {
        // This is not a working MP3 file.
        qWarning() << "SSMP3: This is not a working MP3 file:"
                << m_file.fileName();
        // Abort
        return ERR;
    }

    // Initialize the audio stream length
    setFrameCount(m_curFrameIndex);

    // Calculate average values
    m_avgSeekFrameCount = getFrameCount() / m_seekFrameList.size();
    const unsigned long avgBitrate = sumBitrate / m_seekFrameList.size();
    setBitrate(avgBitrate / 1000);

    // Terminate m_seekFrameList
    addSeekFrame(m_curFrameIndex, 0);

    // Reset positions
    m_curFrameIndex = kFrameIndexMin;

    // Restart decoding at the beginning of the audio stream
    m_curFrameIndex = restartDecoding(m_seekFrameList.front());
    if (m_curFrameIndex != m_seekFrameList.front().frameIndex) {
        qWarning() << "Failed to start decoding:" << m_file.fileName();
        // Abort
        return ERR;
    }

    return OK;
}

void SoundSourceMp3::close() {
    if (m_pFileData) {
        mad_synth_finish(&m_madSynth);
        mad_frame_finish(&m_madFrame);
        mad_stream_finish(&m_madStream);
        m_file.unmap(m_pFileData);
        m_pFileData = NULL;
    }

    m_file.close();

    m_seekFrameList.clear();
}

SINT SoundSourceMp3::restartDecoding(
        const SeekFrameType& seekFrame) {
    qDebug() << "restartDecoding @" << seekFrame.frameIndex;

    // Discard decoded output
    m_madSynthCount = 0;

    if (kFrameIndexMin == seekFrame.frameIndex) {
        mad_frame_finish(&m_madFrame);
        mad_synth_finish(&m_madSynth);
    }
    mad_stream_finish(&m_madStream);

    mad_stream_init(&m_madStream);
    mad_stream_options(&m_madStream, MAD_OPTION_IGNORECRC);
    if (kFrameIndexMin == seekFrame.frameIndex) {
        mad_synth_init(&m_madSynth);
        mad_frame_init(&m_madFrame);
    }

    // Fill input buffer
    mad_stream_buffer(&m_madStream, seekFrame.pInputData,
            m_fileSize - (seekFrame.pInputData - m_pFileData));

    if (kFrameIndexMin < seekFrame.frameIndex) {
        // Muting is done here to eliminate potential pops/clicks
        // from skipping Rob Leslie explains why here:
        // http://www.mars.org/mailman/public/mad-dev/2001-August/000321.html
        mad_frame_mute(&m_madFrame);
        mad_synth_mute(&m_madSynth);
    }

    if (!decodeFrameHeader(&m_madFrame.header, &m_madStream, false)) {
        if (!isStreamValid(m_madStream)) {
            // Failure -> Seek to EOF
            return getFrameCount();
        }
    }

    return seekFrame.frameIndex;
}

void SoundSourceMp3::addSeekFrame(
        SINT frameIndex,
        const unsigned char* pInputData) {
    DEBUG_ASSERT(m_seekFrameList.empty() ||
            (m_seekFrameList.back().frameIndex < frameIndex));
    DEBUG_ASSERT(m_seekFrameList.empty() ||
            (NULL == pInputData) ||
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
    DEBUG_ASSERT(kFrameIndexMin == m_seekFrameList.front().frameIndex);
    DEBUG_ASSERT(SINT(kFrameIndexMin + getFrameIndexMax()) == m_seekFrameList.back().frameIndex);

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
    DEBUG_ASSERT(isValidFrameIndex(frameIndex));

    SINT seekFrameIndex = findSeekFrameIndex(
            frameIndex);
    DEBUG_ASSERT(SINT(m_seekFrameList.size()) > seekFrameIndex);
    const SINT curSeekFrameIndex = findSeekFrameIndex(
            m_curFrameIndex);
    DEBUG_ASSERT(SINT(m_seekFrameList.size()) > curSeekFrameIndex);
    // some consistency checks
    DEBUG_ASSERT((curSeekFrameIndex >= seekFrameIndex) || (m_curFrameIndex < frameIndex));
    DEBUG_ASSERT((curSeekFrameIndex <= seekFrameIndex) || (m_curFrameIndex > frameIndex));
    if ((getFrameIndexMax() <= m_curFrameIndex) || // out of range
            (frameIndex < m_curFrameIndex) || // seek backward
            (seekFrameIndex > (curSeekFrameIndex + kSeekFramePrefetchCount))) { // jump forward

        // Adjust the seek frame index for prefetching
        // Implementation note: The type SINT is unsigned so
        // need to be careful when subtracting!
        if (kSeekFramePrefetchCount < seekFrameIndex) {
            // Restart decoding kSeekFramePrefetchCount seek frames
            // before the expected sync position
            seekFrameIndex -= kSeekFramePrefetchCount;
        } else {
            // Restart decoding at the beginning of the audio stream
            seekFrameIndex = 0;
        }

        m_curFrameIndex = restartDecoding(m_seekFrameList[seekFrameIndex]);
        if (getFrameIndexMax() <= m_curFrameIndex) {
            // out of range -> abort
            return m_curFrameIndex;
        }
        DEBUG_ASSERT(findSeekFrameIndex(m_curFrameIndex) == seekFrameIndex);
    }

    // Decoding starts before the actual target position
    DEBUG_ASSERT(m_curFrameIndex <= frameIndex);

    // Skip (= decode and discard) prefetch data
    const SINT skipFrameCount = frameIndex - m_curFrameIndex;
    skipSampleFrames(skipFrameCount);
    DEBUG_ASSERT(m_curFrameIndex == frameIndex);

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

    const SINT numberOfFramesTotal = math_min(numberOfFrames,
            SINT(getFrameIndexMax() - m_curFrameIndex));

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
            const int decodeResult = mad_frame_decode(&m_madFrame, &m_madStream);
            if (MAD_ERROR_BUFLEN == decodeResult) {
                // Abort
                break;
            }
            if (isUnrecoverableError(decodeResult)) {
                qWarning() << "Unrecoverable MP3 frame decoding error:"
                        << mad_stream_errorstr(&m_madStream);
                // Abort
                break;
            }
            if (isRecoverableError(decodeResult)) {
                if (pMadThisFrame != m_madStream.this_frame) {
                    // Ignore all recoverable errors (and especially
                    // "lost synchronization" warnings) while skipping
                    // over prefetched frames after seeking.
                    if (pSampleBuffer) {
                        qWarning() << "Recoverable MP3 frame decoding error:"
                                << mad_stream_errorstr(&m_madStream);
                    } else {
                        // Decoded samples will simply be discarded
                        qDebug() << "Recoverable MP3 frame decoding error while skipping:"
                            << mad_stream_errorstr(&m_madStream);
                    }
                }
                // Acknowledge error...
                m_madStream.error = MAD_ERROR_NONE;
                // ...and continue
            }
            if (pMadThisFrame == m_madStream.this_frame) {
                qDebug() << "Retry decoding MP3 frame @" << m_curFrameIndex;
                // Retry
                continue;
            }

            DEBUG_ASSERT(isStreamValid(m_madStream));
            DEBUG_ASSERT(getChannelCount() == MAD_NCHANNELS(&m_madFrame.header));

            // Once decoded the frame is synthesized to PCM samples
            mad_synth_frame(&m_madSynth, &m_madFrame);
            DEBUG_ASSERT(getFrameRate() == m_madSynth.pcm.samplerate);
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
            if (isChannelCountMono()) {
                for (SINT i = 0; i < synthReadCount; ++i) {
                    const CSAMPLE sampleValue = madScale(
                            m_madSynth.pcm.samples[0][madSynthOffset + i]);
                    *pSampleBuffer = sampleValue;
                    ++pSampleBuffer;
                    if (readStereoSamples) {
                        *pSampleBuffer = sampleValue;
                        ++pSampleBuffer;
                    }
                }
            } else if (isChannelCountStereo() || readStereoSamples) {
                for (SINT i = 0; i < synthReadCount; ++i) {
                    *pSampleBuffer = madScale(
                            m_madSynth.pcm.samples[0][madSynthOffset + i]);
                    ++pSampleBuffer;
                    *pSampleBuffer = madScale(
                            m_madSynth.pcm.samples[1][madSynthOffset + i]);
                    ++pSampleBuffer;
                }
            } else {
                for (SINT i = 0; i < synthReadCount; ++i) {
                    for (SINT j = 0; j < getChannelCount(); ++j) {
                        *pSampleBuffer = madScale(
                                m_madSynth.pcm.samples[j][madSynthOffset + i]);
                        ++pSampleBuffer;
                    }
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

} // namespace Mixxx
