#include "sources/audiosourcemp3.h"

#include "util/math.h"

#include <id3tag.h>

namespace Mixxx {

namespace {

// In the worst case up to 29 MP3 frames need to be prefetched
// for accurate seeking:
// http://www.mars.org/mailman/public/mad-dev/2002-May/000634.html
//
// TODO (XXX): Fix the implementation and determine the minimum
// required value for passing all unit tests. Currently even the
// theoretical maximum of 29 is not sufficient!?
const AudioSource::size_type kSeekFramePrefetchCount = 4;

const AudioSource::sample_type kMadScale = AudioSource::kSampleValuePeak
        / AudioSource::sample_type(MAD_F_ONE);

inline AudioSource::sample_type madScale(mad_fixed_t sample) {
    return sample * kMadScale;
}

// Optimization: Reserve initial capacity for seek frame list
const AudioSource::size_type kMinutesPerFile = 10; // enough for the majority of files (tunable)
const AudioSource::size_type kSecondsPerMinute = 60; // fixed
const AudioSource::size_type kMaxMp3FramesPerSecond = 39; // fixed: 1 MP3 frame = 26 ms -> ~ 1000 / 26
const AudioSource::size_type kSeekFrameListCapacity = kMinutesPerFile
        * kSecondsPerMinute * kMaxMp3FramesPerSecond;

} // anonymous namespace

AudioSourceMp3::AudioSourceMp3(QString fileName) :
        m_file(fileName),
        m_fileSize(0),
        m_pFileData(NULL),
        m_avgSeekFrameCount(0),
        m_curFrameIndex(0),
        m_madSynthCount(0) {
    mad_stream_init(&m_madStream);
    mad_stream_options(&m_madStream, MAD_OPTION_IGNORECRC);
    mad_frame_init(&m_madFrame);
    mad_synth_init(&m_madSynth);
    m_seekFrameList.reserve(kSeekFrameListCapacity);
}

AudioSourceMp3::~AudioSourceMp3() {
    close();
}

AudioSourcePointer AudioSourceMp3::create(QString fileName) {
    QSharedPointer<AudioSourceMp3> pAudioSource(new AudioSourceMp3(fileName));
    if (OK == pAudioSource->open()) {
        // success
        return pAudioSource;
    } else {
        // failure
        return AudioSourcePointer();
    }
}

void AudioSourceMp3::addSeekFrame(
        diff_type frameIndex,
        const unsigned char* pInputData) {
    SeekFrameType seekFrame;
    seekFrame.pInputData = pInputData;
    seekFrame.frameIndex = frameIndex;
//    qDebug() << "seekFrameList[" << m_seekFrameList.size() << "] = " << seekFrame.frameIndex;
    m_seekFrameList.push_back(seekFrame);
}

namespace {
    int decodeFrameHeaderAndSkipId3Tags(mad_frame* pMadFrame, mad_stream* pMadStream) {
        for (;;) {
            const int result = mad_header_decode(&pMadFrame->header, pMadStream);
            if ((-1 == result) && (MAD_ERROR_LOSTSYNC == pMadStream->error)) {
                long tagsize = id3_tag_query(pMadStream->this_frame,
                        pMadStream->bufend - pMadStream->this_frame);
                if (0 < tagsize) {
                    // Skip ID3 tag...
                    mad_stream_skip(pMadStream, tagsize);
                    // ...and continue with the next frame in the stream
                    continue;
                }
            }
            return result;
        }
    }
}

Result AudioSourceMp3::open() {
    if (!m_file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open file:" << m_file.fileName();
        return ERR;
    }

    // Get a pointer to the file using memory mapped IO
    m_fileSize = m_file.size();
    m_pFileData = m_file.map(0, m_fileSize);
    // Transfer it to the mad stream-buffer:
    mad_stream_buffer(&m_madStream, m_pFileData, m_fileSize);
    DEBUG_ASSERT(m_pFileData == m_madStream.this_frame);

    // Decode all the headers and calculate audio properties
    unsigned long sumBitrate = 0;
    setChannelCount(kChannelCountDefault);
    setFrameRate(kFrameRateDefault);

    mad_units madUnits = MAD_UNITS_44100_HZ; // default value
    mad_timer_t madDuration = mad_timer_zero;

    diff_type frameIndex = 0;
    do {
        if (-1 == decodeFrameHeaderAndSkipId3Tags(&m_madFrame, &m_madStream)) {
            if (MAD_RECOVERABLE(m_madStream.error)) {
                qDebug() << "Recoverable MP3 header decoding error:"
                        << mad_stream_errorstr(&m_madStream);
                continue;
            } else {
                if (MAD_ERROR_BUFLEN != m_madStream.error) {
                    qDebug() << "Unrecoverable MP3 header decoding error:"
                            << mad_stream_errorstr(&m_madStream);
                }
                break;
            }
        }

        // Grab data from madHeader
        const size_type madChannelCount = MAD_NCHANNELS(&m_madFrame.header);
        if (kChannelCountDefault == getChannelCount()) {
            // initially set the number of channels
            setChannelCount(madChannelCount);
        } else {
            // check for consistent number of channels
            if ((0 < madChannelCount) && getChannelCount() != madChannelCount) {
                qWarning() << "Differing number of channels in some headers:"
                        << m_file.fileName() << getChannelCount() << "<>"
                        << madChannelCount;
                mad_header_finish(&madHeader);
                return ERR; // abort
            }
        }
        const size_type madSampleRate = m_madFrame.header.samplerate;
        if (kFrameRateDefault == getFrameRate()) {
            // initially set the frame/sample rate
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
                mad_header_finish(&madHeader);
                return ERR; // abort
            }
            setFrameRate(madSampleRate);
        } else {
            // check for consistent frame/sample rate
            if ((0 < madSampleRate) && (getFrameRate() != madSampleRate)) {
                qWarning() << "Differing sample rate in some headers:"
                        << m_file.fileName()
                        << getFrameRate() << "<>" << madSampleRate;
                mad_header_finish(&madHeader);
                return ERR; // abort
            }
        }

        addSeekFrame(frameIndex, m_madStream.this_frame);

        mad_timer_add(&madDuration, m_madFrame.header.duration);
        frameIndex = mad_timer_count(madDuration, madUnits);

        sumBitrate += m_madFrame.header.bitrate;

        DEBUG_ASSERT(NULL != m_madStream.this_frame);
        DEBUG_ASSERT(0 < (m_madStream.this_frame - m_pFileData));
    } while (quint64(m_madStream.this_frame - m_pFileData) < m_fileSize);

    if (MAD_ERROR_NONE != m_madStream.error) {
        // Unreachable code for recoverable errors
        DEBUG_ASSERT(!MAD_RECOVERABLE(m_madStream.error));
        if (MAD_ERROR_BUFLEN != m_madStream.error) {
            qWarning() << "Unrecoverable MP3 header error:"
                    << mad_stream_errorstr(&m_madStream);
            return ERR; // abort
        }
    }

    if (m_seekFrameList.empty()) {
        // This is not a working MP3 file.
        qWarning() << "SSMP3: This is not a working MP3 file:"
                << m_file.fileName();
        return ERR; // abort
    }

    // Initialize audio stream length
    setFrameCount(frameIndex);

    // Calculate average values
    m_avgSeekFrameCount = getFrameCount() / m_seekFrameList.size();
    int avgBitrate = sumBitrate / m_seekFrameList.size();
    setBitrate(avgBitrate / 1000);

    // Terminate m_seekFrameList
    SeekFrameType terminalSeekFrame;
    terminalSeekFrame.pInputData = 0;
    terminalSeekFrame.frameIndex = frameIndex;
    m_seekFrameList.push_back(terminalSeekFrame);

    if (!restartDecoding(m_seekFrameList.front())) {
        qWarning() << "Failed to start decoding:" << m_file.fileName();
        return ERR;
    }

    return OK;
}

bool AudioSourceMp3::restartDecoding(const SeekFrameType& seekFrame) {
    // Reset the MAD decoder completely. Otherwise
    // audible artifacts and glitches occur when
    // seeking through the stream no matter how
    // many MP3 frames are prefetched!
    mad_synth_finish(&m_madSynth);
    mad_frame_finish(&m_madFrame);
    mad_stream_finish(&m_madStream);
    mad_stream_init(&m_madStream);
    mad_stream_options(&m_madStream, MAD_OPTION_IGNORECRC);
    mad_frame_init(&m_madFrame);
    mad_synth_init(&m_madSynth);

    // Reset stream position
    m_curFrameIndex = seekFrame.frameIndex;

    // Discard decoded output
    m_madSynthCount = 0;

    // Fill input buffer
    mad_stream_buffer(&m_madStream, seekFrame.pInputData,
            m_fileSize - (seekFrame.pInputData - m_pFileData));

    // Muting is done here to eliminate potential pops/clicks
    // from skipping Rob Leslie explains why here:
    // http://www.mars.org/mailman/public/mad-dev/2001-August/000321.html
    //
    // TODO(XXX): Should not be necessary since both members
    // m_madFrame and m_madSynth have been initialized just
    // before. Might be removed after reliable unit tests show
    // that those calls are really not needed anymore.
    mad_synth_mute(&m_madSynth);
    mad_frame_mute(&m_madFrame);

    // success
    return true;
}

void AudioSourceMp3::close() {
    mad_synth_finish(&m_madSynth);
    mad_frame_finish(&m_madFrame);
    mad_stream_finish(&m_madStream);
    m_madSynthCount = 0;

    m_seekFrameList.clear();
    m_avgSeekFrameCount = 0;
    m_curFrameIndex = 0;

    m_file.unmap(m_pFileData);
    m_fileSize = 0;
    m_pFileData = NULL;
    m_file.close();

    reset();
}

AudioSourceMp3::SeekFrameList::size_type AudioSourceMp3::findSeekFrameIndex(
        diff_type frameIndex) const {
    // Check preconditions
    DEBUG_ASSERT(0 < m_avgSeekFrameCount);
    DEBUG_ASSERT(!m_seekFrameList.empty());
    DEBUG_ASSERT(0 == m_seekFrameList.front().frameIndex);
    DEBUG_ASSERT(diff_type(getFrameCount()) == m_seekFrameList.back().frameIndex);

    AudioSourceMp3::SeekFrameList::size_type lowerBound =
            0;
    AudioSourceMp3::SeekFrameList::size_type upperBound =
            m_seekFrameList.size();
    // Initial guess based on average frame size
    AudioSourceMp3::SeekFrameList::size_type seekFrameIndex =
            frameIndex / m_avgSeekFrameCount;
    if (seekFrameIndex >= m_seekFrameList.size()) {
        seekFrameIndex = m_seekFrameList.size() - 1;
    }
    while ((upperBound - lowerBound) > 1) {
        DEBUG_ASSERT(seekFrameIndex >= lowerBound);
        DEBUG_ASSERT(seekFrameIndex < upperBound);
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
    DEBUG_ASSERT(m_seekFrameList.size() > seekFrameIndex);
    DEBUG_ASSERT(m_seekFrameList[seekFrameIndex].frameIndex <= frameIndex);
    DEBUG_ASSERT(((seekFrameIndex + 1) >= m_seekFrameList.size()) ||
            (m_seekFrameList[seekFrameIndex + 1].frameIndex > frameIndex));

    return seekFrameIndex;
}

AudioSource::diff_type AudioSourceMp3::seekSampleFrame(diff_type frameIndex) {
    DEBUG_ASSERT(isValidFrameIndex(frameIndex));
    if (!m_seekFrameList.empty()) {
        SeekFrameList::size_type seekFrameIndex = findSeekFrameIndex(
                frameIndex);
        DEBUG_ASSERT(m_seekFrameList.size() > seekFrameIndex);
        const SeekFrameList::size_type curSeekFrameIndex = findSeekFrameIndex(
                m_curFrameIndex);
        DEBUG_ASSERT(m_seekFrameList.size() > curSeekFrameIndex);
        // some consistency checks
        DEBUG_ASSERT((curSeekFrameIndex >= seekFrameIndex) || (m_curFrameIndex < frameIndex));
        DEBUG_ASSERT((curSeekFrameIndex <= seekFrameIndex) || (m_curFrameIndex > frameIndex));
        if ((frameIndex < m_curFrameIndex) || // seeking backwards?
                (seekFrameIndex
                        > (curSeekFrameIndex + kSeekFramePrefetchCount))) { // jumping forward?
            // Implementation note: The type size_type is unsigned so
            // need to be careful when subtracting!
            if (kSeekFramePrefetchCount < seekFrameIndex) {
                seekFrameIndex -= kSeekFramePrefetchCount;
            } else {
                seekFrameIndex = 0;
            }
            restartDecoding(m_seekFrameList[seekFrameIndex]);
            DEBUG_ASSERT(findSeekFrameIndex(m_curFrameIndex) == seekFrameIndex);
        }
        // decoding starts before the actual target position
        DEBUG_ASSERT(m_curFrameIndex <= frameIndex);
        // decode and discard prefetch data
        const size_type prefetchFrameCount = frameIndex - m_curFrameIndex;
        skipFrameSamples(prefetchFrameCount);
        DEBUG_ASSERT(m_curFrameIndex == frameIndex);
    }
    return m_curFrameIndex;
}

AudioSource::size_type AudioSourceMp3::readSampleFrames(
        size_type numberOfFrames, sample_type* sampleBuffer) {
    return readSampleFrames(numberOfFrames, sampleBuffer,
            frames2samples(numberOfFrames), false);
}

AudioSource::size_type AudioSourceMp3::readSampleFramesStereo(
        size_type numberOfFrames, sample_type* sampleBuffer,
        size_type sampleBufferSize) {
    return readSampleFrames(numberOfFrames, sampleBuffer, sampleBufferSize,
            true);
}

AudioSource::size_type AudioSourceMp3::readSampleFrames(
        size_type numberOfFrames, sample_type* sampleBuffer,
        size_type sampleBufferSize, bool readStereoSamples) {
    DEBUG_ASSERT(isValidFrameIndex(m_curFrameIndex));

    sample_type* pSampleBuffer = sampleBuffer;
    size_type numberOfFramesRemaining =
            math_min(numberOfFrames, getFrameCount() - m_curFrameIndex);
    numberOfFramesRemaining =
            math_min(numberOfFramesRemaining, samples2frames(sampleBufferSize));
    while (0 < numberOfFramesRemaining) {
        if (0 >= m_madSynthCount) {
            // When all decoded output data has been consumed...
            DEBUG_ASSERT(0 == m_madSynthCount);
            // ...decode the next MP3 frame
            DEBUG_ASSERT(NULL != m_madStream.buffer);
            DEBUG_ASSERT(NULL != m_madStream.this_frame);

            // Always decode the frame's header before the frame itself
            // to skip all non-audio frames with ID3 tag data.
            if (-1 == decodeFrameHeaderAndSkipId3Tags(&m_madFrame, &m_madStream)) {
                if (MAD_RECOVERABLE(m_madStream.error)) {
                    if (NULL != pSampleBuffer) {
                        qDebug() << "Recoverable MP3 header decoding error:"
                                << mad_stream_errorstr(&m_madStream);
                    }
                    continue;
                } else {
                    if (MAD_ERROR_BUFLEN != m_madStream.error) {
                        qDebug() << "Unrecoverable MP3 header decoding error:"
                                << mad_stream_errorstr(&m_madStream);
                    }
                    break;
                }
            }

            if (-1 == mad_frame_decode(&m_madFrame, &m_madStream)) {
                if (MAD_RECOVERABLE(m_madStream.error)) {
                    if (NULL != pSampleBuffer) {
                        qWarning() << "Recoverable MP3 frame decoding error:"
                                << mad_stream_errorstr(&m_madStream);
                    }
                    continue;
                } else {
                    if (MAD_ERROR_BUFLEN != m_madStream.error) {
                        qWarning() << "Unrecoverable MP3 frame decoding error:"
                                << mad_stream_errorstr(&m_madStream);
                    }
                    break;
                }
            }

            const mad_header* const pMadFrameHeader = &m_madFrame.header;
            DEBUG_ASSERT(getChannelCount() == MAD_NCHANNELS(pMadFrameHeader));
            // Once decoded the frame is synthesized to PCM samples
            mad_synth_frame(&m_madSynth, &m_madFrame);
            DEBUG_ASSERT(getFrameRate() == m_madSynth.pcm.samplerate);
            m_madSynthCount = m_madSynth.pcm.length;
        }
        const size_type framesRead = math_min(
                m_madSynthCount, numberOfFramesRemaining);
        if (NULL != pSampleBuffer) {
            const size_type madSynthOffset =
                    m_madSynth.pcm.length - m_madSynthCount;
            if (isChannelCountMono()) {
                for (size_type i = 0; i < framesRead; ++i) {
                    const sample_type sampleValue = madScale(
                            m_madSynth.pcm.samples[0][madSynthOffset + i]);
                    *(pSampleBuffer++) = sampleValue;
                    if (readStereoSamples) {
                        *(pSampleBuffer++) = sampleValue;
                    }
                }
            } else if (isChannelCountStereo() || readStereoSamples) {
                for (size_type i = 0; i < framesRead; ++i) {
                    *(pSampleBuffer++) = madScale(
                            m_madSynth.pcm.samples[0][madSynthOffset + i]);
                    *(pSampleBuffer++) = madScale(
                            m_madSynth.pcm.samples[1][madSynthOffset + i]);
                }
            } else {
                for (size_type i = 0; i < framesRead; ++i) {
                    for (size_type j = 0; j < getChannelCount(); ++j) {
                        *(pSampleBuffer++) = madScale(
                                m_madSynth.pcm.samples[j][madSynthOffset + i]);
                    }
                }
            }
        }
        // consume decoded output data
        m_madSynthCount -= framesRead;
        m_curFrameIndex += framesRead;
        numberOfFramesRemaining -= framesRead;
    }
    DEBUG_ASSERT(numberOfFrames >= numberOfFramesRemaining);
    return numberOfFrames - numberOfFramesRemaining;
}

} // namespace Mixxx
