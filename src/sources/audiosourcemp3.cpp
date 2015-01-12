#include "sources/audiosourcemp3.h"

#include "util/math.h"

#include <id3tag.h>

namespace Mixxx
{

namespace
{
    const AudioSource::size_type kSeekFramePrefetchCount =
        2; // required for synchronization

    const AudioSource::size_type kMaxSamplesPerMp3Frame =
        1152;

    const AudioSource::diff_type kMaxSkipFrameSamplesWhenSeeking =
        2 * kSeekFramePrefetchCount * kMaxSamplesPerMp3Frame;

    const AudioSource::sample_type kMadScale =
        AudioSource::kSampleValuePeak / AudioSource::sample_type(MAD_F_ONE);

    inline
    AudioSource::sample_type madScale(mad_fixed_t sample) {
        return sample * kMadScale;
    }

    // Optimization: Reserve initial capacity for seek frame list
    const AudioSource::size_type kMinutesPerFile =
        10; // enough for the majority of files (tunable)
    const AudioSource::size_type kSecondsPerMinute =
        60; // fixed
    const AudioSource::size_type kMaxMp3FramesPerSecond =
        39; // fixed: 1 MP3 frame = 26 ms -> ~ 1000 / 26
    const AudioSource::size_type kSeekFrameListCapacity =
        kMinutesPerFile * kSecondsPerMinute * kMaxMp3FramesPerSecond;

    bool mad_skip_id3_tag(mad_stream* pStream) {
        long tagsize = id3_tag_query(
            pStream->this_frame, pStream->bufend - pStream->this_frame);
        if (0 < tagsize) {
            mad_stream_skip(pStream, tagsize);
            return true;
        } else {
            return false;
        }
    }
}

AudioSourceMp3::AudioSourceMp3(QString fileName)
    : m_file(fileName),
      m_fileSize(0),
      m_pFileData(NULL),
      m_avgSeekFrameCount(0),
      m_curFrameIndex(0),
      m_madSynthCount(0) {
    initDecoding();
    m_seekFrameList.reserve(kSeekFrameListCapacity);
}

AudioSourceMp3::~AudioSourceMp3() {
    close();
}

AudioSourcePointer AudioSourceMp3::create(QString fileName) {
    QSharedPointer<AudioSourceMp3> pAudioSource(
        new AudioSourceMp3(fileName));
    if (OK == pAudioSource->open()) {
        // success
        return pAudioSource;
    } else {
        // failure
        return AudioSourcePointer();
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

    // Decode all the headers and calculate audio properties
    unsigned long sumBitrate = 0;
    unsigned int madFrameCount = 0;
    setChannelCount(kChannelCountDefault);
    setFrameRate(kFrameRateDefault);
    mad_timer_t madFileDuration = mad_timer_zero;
    mad_units madUnits = MAD_UNITS_44100_HZ; // default value
    while ((m_madStream.bufend - m_madStream.this_frame) > 0) {
        mad_header madHeader;
        mad_header_init(&madHeader);
        if (0 != mad_header_decode(&madHeader, &m_madStream)) {
            if (MAD_RECOVERABLE(m_madStream.error)) {
                if (MAD_ERROR_LOSTSYNC == m_madStream.error) {
                    // Ignore LOSTSYNC due to ID3 tags
                    mad_skip_id3_tag(&m_madStream);
                } else {
                    qDebug()
                        << "Recoverable MP3 header error:"
                        << mad_stream_errorstr(&m_madStream);
                }
                mad_header_finish(&madHeader);
                continue;
            } else {
                if (MAD_ERROR_BUFLEN == m_madStream.error) {
                    // EOF
                    break; // done
                } else {
                    qWarning()
                        << "Unrecoverable MP3 header error:"
                        << mad_stream_errorstr(&m_madStream);
                    mad_header_finish(&madHeader);
                    return ERR; // abort
                }
            }
        }

        // Grab data from madHeader
        const size_type madChannelCount = MAD_NCHANNELS(&madHeader);
        if (kChannelCountDefault == getChannelCount()) {
            // initially set the number of channels
            setChannelCount(madChannelCount);
        } else {
            // check for consistent number of channels
            if ((0 < madChannelCount) && getChannelCount() != madChannelCount) {
                qWarning()
                    << "Differing number of channels in some headers:"
                    << m_file.fileName() << getChannelCount() << "<>"
                    << madChannelCount;
            }
        }
        const size_type madSampleRate = madHeader.samplerate;
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
                qWarning() << "Invalid sample rate:"
                        << m_file.fileName() << madSampleRate;
                return ERR; // abort
            }
            setFrameRate(madSampleRate);
        } else {
            // check for consistent frame/sample rate
            if ((0 < madSampleRate) && (getFrameRate() != madSampleRate)) {
                qWarning()
                    << "Differing sample rate in some headers:"
                    << m_file.fileName()
                    << getFrameRate() << "<>" << madSampleRate;
                return ERR; // abort
            }
        }

        // Add frame to list of frames
        SeekFrameType seekFrame;
        seekFrame.pFileData = m_madStream.this_frame;
        seekFrame.frameIndex = mad_timer_count(madFileDuration, madUnits);
        m_seekFrameList.push_back(seekFrame);

        mad_timer_add(&madFileDuration, madHeader.duration);
        sumBitrate += madHeader.bitrate;

        mad_header_finish(&madHeader);

        ++madFrameCount;
    }

    if (0 < madFrameCount) {
        setFrameCount(mad_timer_count(madFileDuration, madUnits));
        m_avgSeekFrameCount = getFrameCount() / madFrameCount;
        int avgBitrate = sumBitrate / madFrameCount;
        setBitrate(avgBitrate / 1000);
    } else {
        // This is not a working MP3 file.
        qWarning()
            << "SSMP3: This is not a working MP3 file:" << m_file.fileName();
        return ERR; // abort
    }

    restartDecoding(m_seekFrameList.front());

    return OK;
}

void AudioSourceMp3::initDecoding() {
    mad_stream_init(&m_madStream);
    mad_stream_options(&m_madStream, MAD_OPTION_IGNORECRC);
    mad_frame_init(&m_madFrame);
    mad_synth_init(&m_madSynth);
}

void AudioSourceMp3::finishDecoding() {
    mad_synth_finish(&m_madSynth);
    mad_frame_finish(&m_madFrame);
    mad_stream_finish(&m_madStream);
    m_madSynthCount = 0;
    m_curFrameIndex = getFrameCount(); // invalidate
}

void AudioSourceMp3::restartDecoding(const SeekFrameType& seekFrame) {
    finishDecoding();
    initDecoding();
    mad_stream_buffer(
        &m_madStream,
        seekFrame.pFileData, m_fileSize - (seekFrame.pFileData - m_pFileData));
    m_curFrameIndex = seekFrame.frameIndex;
}

void AudioSourceMp3::close() {
    finishDecoding();

    m_seekFrameList.clear();
    m_avgSeekFrameCount = 0;
    m_curFrameIndex = 0;

    m_file.unmap(m_pFileData);
    m_fileSize = 0;
    m_pFileData = NULL;
    m_file.close();

    reset();
}

AudioSourceMp3::SeekFrameList::size_type AudioSourceMp3::findSeekFrameIndex(diff_type frameIndex) const {
    if ((0 >= frameIndex) || m_seekFrameList.empty()) {
        return 0;
    }
    // Guess position of frame in m_seekFrameList based on average frame size
    AudioSourceMp3::SeekFrameList::size_type seekFrameIndex =
        frameIndex / m_avgSeekFrameCount;
    if (seekFrameIndex >= m_seekFrameList.size()) {
        seekFrameIndex = m_seekFrameList.size() - 1;
    }
    // binary search starting at seekFrameIndex
    AudioSourceMp3::SeekFrameList::size_type lowerBound = 0;
    AudioSourceMp3::SeekFrameList::size_type upperBound = m_seekFrameList.size();
    while ((upperBound - lowerBound) > 1) {
        DEBUG_ASSERT(seekFrameIndex >= lowerBound);
        DEBUG_ASSERT(seekFrameIndex < upperBound);
        if (m_seekFrameList[seekFrameIndex].frameIndex > frameIndex) {
            upperBound = seekFrameIndex;
        } else {
            lowerBound = seekFrameIndex;
        }
        seekFrameIndex = lowerBound + (upperBound - lowerBound) / 2;
    }
    DEBUG_ASSERT(m_seekFrameList.size() > seekFrameIndex);
    DEBUG_ASSERT(m_seekFrameList[seekFrameIndex].frameIndex <= frameIndex);
    DEBUG_ASSERT(((seekFrameIndex + 1) >= m_seekFrameList.size()) ||
        (m_seekFrameList[seekFrameIndex + 1].frameIndex > frameIndex));
    return seekFrameIndex;
}

AudioSource::diff_type AudioSourceMp3::seekSampleFrame(diff_type frameIndex) {
    if (m_curFrameIndex == frameIndex) {
        return m_curFrameIndex;
    }
    if (0 > frameIndex) {
        return seekSampleFrame(0);
    }
    // simply skip frames when jumping no more than kMaxSkipFrameSamplesWhenSeeking frames forward
    if ((frameIndex < m_curFrameIndex) ||
        ((frameIndex - m_curFrameIndex) > kMaxSkipFrameSamplesWhenSeeking)) {
        SeekFrameList::size_type seekFrameIndex = findSeekFrameIndex(frameIndex);
        if (seekFrameIndex <= kSeekFramePrefetchCount) {
            seekFrameIndex = 0;
        } else {
            seekFrameIndex -= kSeekFramePrefetchCount;
        }
        DEBUG_ASSERT(seekFrameIndex < m_seekFrameList.size());
        restartDecoding(m_seekFrameList[seekFrameIndex]);
    }
    // decode and discard prefetch data
    DEBUG_ASSERT(m_curFrameIndex <= frameIndex);
    skipFrameSamples(frameIndex - m_curFrameIndex);
    DEBUG_ASSERT(m_curFrameIndex == frameIndex);
    return m_curFrameIndex;
}

AudioSource::size_type AudioSourceMp3::readSampleFrames(
    size_type numberOfFrames, sample_type* sampleBuffer) {
    return readSampleFrames(numberOfFrames, sampleBuffer, frames2samples(numberOfFrames), false);
}

AudioSource::size_type AudioSourceMp3::readSampleFramesStereo(
    size_type numberOfFrames, sample_type* sampleBuffer, size_type sampleBufferSize) {
    return readSampleFrames(numberOfFrames, sampleBuffer, sampleBufferSize, true);
}

AudioSource::size_type AudioSourceMp3::readSampleFrames(
    size_type numberOfFrames,
    sample_type* sampleBuffer,
    size_type sampleBufferSize,
    bool readStereoSamples) {
    sample_type* pSampleBuffer = sampleBuffer;
    const size_type numberOfFramesTotal =
        math_min(numberOfFrames, samples2frames(sampleBufferSize));
    size_type numberOfFramesRead = 0;
    while (numberOfFramesTotal > numberOfFramesRead) {
        if (0 >= m_madSynthCount) {
            if (0 != mad_frame_decode(&m_madFrame, &m_madStream)) {
                if (MAD_RECOVERABLE(m_madStream.error)) {
                    if (MAD_ERROR_LOSTSYNC == m_madStream.error) {
                        // Ignore LOSTSYNC due to ID3 tags
                        mad_skip_id3_tag(&m_madStream);
                    } else {
                        qDebug()
                            << "Recoverable MP3 decoding error:"
                            << mad_stream_errorstr(&m_madStream);
                    }
                    continue;
                } else {
                    if (MAD_ERROR_BUFLEN != m_madStream.error) {
                        qWarning()
                            << "Unrecoverable MP3 decoding error:"
                            << mad_stream_errorstr(&m_madStream);
                    }
                    break;
                }
            }
            /* Once decoded the frame is synthesized to PCM samples. No ERRs
             * are reported by mad_synth_frame();
             */
            mad_synth_frame(&m_madSynth, &m_madFrame);
            m_madSynthCount = m_madSynth.pcm.length;
        }
        const size_type madSynthOffset =
            m_madSynth.pcm.length - m_madSynthCount;
        const size_type framesRead =
            math_min(m_madSynthCount, numberOfFramesTotal - numberOfFramesRead);
        m_madSynthCount -= framesRead;
        m_curFrameIndex += framesRead;
        numberOfFramesRead += framesRead;
        if (NULL != pSampleBuffer) {
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
    }
    return numberOfFramesRead;
}

} // namespace Mixxx
