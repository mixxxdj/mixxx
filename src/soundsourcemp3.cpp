/***************************************************************************
                          soundsourcemp3.cpp  -  description
                             -------------------
    copyright            : (C) 2002 by Tue and Ken Haste Andersen
    email                :
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "soundsourcemp3.h"

#include "trackmetadatataglib.h"
#include "util/math.h"

#include <taglib/mpegfile.h>

namespace
{
    const Mixxx::AudioSource::size_type kSeekFramePrefetchCount = 2; // required for synchronization

    const Mixxx::AudioSource::size_type kMaxSamplesPerMp3Frame = 1152;

    const Mixxx::AudioSource::diff_type kMaxSkipFrameSamplesWhenSeeking = 2 * kSeekFramePrefetchCount * kMaxSamplesPerMp3Frame;

    const Mixxx::AudioSource::sample_type kMadScale =
            Mixxx::AudioSource::kSampleValuePeak
                    / Mixxx::AudioSource::sample_type(
                            mad_fixed_t(1) << MAD_F_FRACBITS);

    inline Mixxx::AudioSource::sample_type madScale(mad_fixed_t sample) {
        return sample * kMadScale;
    }

    // Optimization: Reserve initial capacity for seek frame list
    const Mixxx::AudioSource::size_type kMinutesPerFile = 10; // enough for the majority of files (tunable)
    const Mixxx::AudioSource::size_type kSecondsPerMinute = 60; // fixed
    const Mixxx::AudioSource::size_type kMaxMp3FramesPerSecond = 39; // fixed: 1 MP3 frame = 26 ms -> ~ 1000 / 26
    const Mixxx::AudioSource::size_type kSeekFrameListCapacity = kMinutesPerFile * kSecondsPerMinute * kMaxMp3FramesPerSecond;

    bool mad_skip_id3_tag(mad_stream* pStream) {
        long tagsize = id3_tag_query(pStream->this_frame,
                pStream->bufend - pStream->this_frame);
        if (0 < tagsize) {
            mad_stream_skip(pStream, tagsize);
            return true;
        } else {
            return false;
        }
    }
}

SoundSourceMp3::SoundSourceMp3(QString qFilename)
        : Super(qFilename, "mp3"),
          m_file(qFilename),
          m_fileSize(0),
          m_pFileData(NULL),
          m_avgSeekFrameCount(0),
          m_curFrameIndex(0),
          m_madSynthCount(0) {
    mad_stream_init(&m_madStream);
    mad_frame_init(&m_madFrame);
    mad_synth_init(&m_madSynth);
}

SoundSourceMp3::~SoundSourceMp3()
{
    close();
}

QList<QString> SoundSourceMp3::supportedFileExtensions()
{
    QList<QString> list;
    list.push_back("mp3");
    return list;
}

Result SoundSourceMp3::open() {
    if (!m_file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open file:" << getFilename();
        return ERR;
    }

    // Get a pointer to the file using memory mapped IO
    m_fileSize = m_file.size();
    m_pFileData = m_file.map(0, m_fileSize);

    // Transfer it to the mad stream-buffer:
    mad_stream_init(&m_madStream);
    mad_stream_options(&m_madStream, MAD_OPTION_IGNORECRC);
    mad_stream_buffer(&m_madStream, m_pFileData, m_fileSize);

    m_seekFrameList.reserve(kSeekFrameListCapacity);

    /*
     Decode all the headers, and fill in stats:
     */
    unsigned long sumBitrate = 0;
    unsigned int madFrameCount = 0;
    setChannelCount(kChannelCountDefault);
    setFrameRate(kFrameRateDefault);
    mad_timer_t madFileDuration = mad_timer_zero;
    mad_units madUnits;
    while ((m_madStream.bufend - m_madStream.this_frame) > 0) {
        mad_header madHeader;
        mad_header_init(&madHeader);
        if (0 != mad_header_decode(&madHeader, &m_madStream)) {
            if (MAD_RECOVERABLE(m_madStream.error)) {
                if (MAD_ERROR_LOSTSYNC == m_madStream.error) {
                    // Ignore LOSTSYNC due to ID3 tags
                    mad_skip_id3_tag(&m_madStream);
                } else {
                    qDebug() << "Recoverable MP3 header error:" << mad_stream_errorstr(&m_madStream);
                }
                mad_header_finish(&madHeader);
                continue;
            } else {
                if (MAD_ERROR_BUFLEN == m_madStream.error) {
                    // EOF -> done
                    break;
                } else {
                    qWarning() << "Unrecoverable MP3 header error:" << mad_stream_errorstr(&m_madStream);
                    // abort
                    mad_header_finish(&madHeader);
                    close();
                    return ERR;
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
                qWarning() << "Differing number of channels in some headers:"
                        << getFilename() << getChannelCount() << "<>"
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
                        << getFilename() << madSampleRate;
                // abort
                close();
                return ERR;
            }
            setFrameRate(madSampleRate);
        } else {
            // check for consistent frame/sample rate
            if ((0 < madSampleRate) && (getFrameRate() != madSampleRate)) {
                qWarning() << "Differing sample rate in some headers:"
                        << getFilename() << getFrameRate() << "<>"
                        << madSampleRate;
                // abort
                close();
                return ERR;
            }
        }

        // Add frame to list of frames
        MadSeekFrameType seekFrame;
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
        setBitrate(avgBitrate);
    } else {
        // This is not a working MP3 file.
        qWarning() << "SSMP3: This is not a working MP3 file:" << getFilename();
        // abort
        close();
        return ERR;
    }

    // restart decoding
    m_curFrameIndex = getFrameCount();
    seekFrame(0);

    return OK;
}

void SoundSourceMp3::close() {
    m_seekFrameList.clear();
    m_avgSeekFrameCount = 0;
    m_curFrameIndex = 0;

    mad_synth_finish(&m_madSynth);
    mad_frame_finish(&m_madFrame);
    mad_stream_finish(&m_madStream);
    m_madSynthCount = 0;

    m_file.unmap(m_pFileData);
    m_fileSize = 0;
    m_pFileData = NULL;
    m_file.close();

    Super::reset();
}

SoundSourceMp3::MadSeekFrameList::size_type SoundSourceMp3::findSeekFrameIndex(diff_type frameIndex) const {
    if ((0 >= frameIndex) || m_seekFrameList.empty()) {
        return 0;
    }
    // Guess position of frame in m_seekFrameList based on average frame size
    SoundSourceMp3::MadSeekFrameList::size_type seekFrameIndex = frameIndex / m_avgSeekFrameCount;
    if (seekFrameIndex >= m_seekFrameList.size()) {
        seekFrameIndex = m_seekFrameList.size() - 1;
    }
    // binary search starting at seekFrameIndex
    SoundSourceMp3::MadSeekFrameList::size_type lowerBound = 0;
    SoundSourceMp3::MadSeekFrameList::size_type upperBound = m_seekFrameList.size();
    while ((upperBound - lowerBound) > 1) {
        Q_ASSERT(seekFrameIndex >= lowerBound);
        Q_ASSERT(seekFrameIndex < upperBound);
        if (m_seekFrameList[seekFrameIndex].frameIndex > frameIndex) {
            upperBound = seekFrameIndex;
        } else {
            lowerBound = seekFrameIndex;
        }
        seekFrameIndex = lowerBound + (upperBound - lowerBound) / 2;
    }
    Q_ASSERT(m_seekFrameList.size() > seekFrameIndex);
    Q_ASSERT(m_seekFrameList[seekFrameIndex].frameIndex <= frameIndex);
    Q_ASSERT(((seekFrameIndex + 1) >= m_seekFrameList.size()) || (m_seekFrameList[seekFrameIndex + 1].frameIndex > frameIndex));
    return seekFrameIndex;
}

Mixxx::AudioSource::diff_type SoundSourceMp3::seekFrame(diff_type frameIndex) {
    if (m_curFrameIndex == frameIndex) {
        return m_curFrameIndex;
    }
    if (0 > frameIndex) {
        return seekFrame(0);
    }
    // simply skip frames when jumping no more than kMaxSkipFrameSamplesWhenSeeking frames forward
    if ((frameIndex < m_curFrameIndex) || ((frameIndex - m_curFrameIndex) > kMaxSkipFrameSamplesWhenSeeking)) {
        MadSeekFrameList::size_type seekFrameIndex = findSeekFrameIndex(frameIndex);
        if (seekFrameIndex <= kSeekFramePrefetchCount) {
            seekFrameIndex = 0;
        } else {
            seekFrameIndex -= kSeekFramePrefetchCount;
        }
        Q_ASSERT(seekFrameIndex < m_seekFrameList.size());
        const MadSeekFrameType& seekFrame(m_seekFrameList[seekFrameIndex]);
        // restart decoder
        mad_synth_finish(&m_madSynth);
        mad_frame_finish(&m_madFrame);
        mad_stream_finish(&m_madStream);
        mad_stream_init(&m_madStream);
        mad_stream_options(&m_madStream, MAD_OPTION_IGNORECRC);
        mad_stream_buffer(&m_madStream, seekFrame.pFileData, m_fileSize - (seekFrame.pFileData - m_pFileData));
        mad_frame_init(&m_madFrame);
        mad_synth_init(&m_madSynth);
        m_curFrameIndex = seekFrame.frameIndex;
        m_madSynthCount = 0;
    }
    // decode and discard prefetch data
    Q_ASSERT(m_curFrameIndex <= frameIndex);
    skipFrameSamples(frameIndex - m_curFrameIndex);
    Q_ASSERT(m_curFrameIndex == frameIndex);
    return m_curFrameIndex;
}

Mixxx::AudioSource::size_type SoundSourceMp3::readFrameSamplesInterleaved(
        size_type frameCount, sample_type* sampleBuffer) {
    return readFrameSamplesInterleaved(frameCount, sampleBuffer, false);
}

Mixxx::AudioSource::size_type SoundSourceMp3::readStereoFrameSamplesInterleaved(
        size_type frameCount, sample_type* sampleBuffer) {
    return readFrameSamplesInterleaved(frameCount, sampleBuffer, true);
}

Mixxx::AudioSource::size_type SoundSourceMp3::readFrameSamplesInterleaved(
        size_type frameCount, sample_type* sampleBuffer,
        bool readStereoSamples) {
    size_type framesRemaining = frameCount;
    sample_type* pSampleBuffer = sampleBuffer;
    while (0 < framesRemaining) {
        if (0 >= m_madSynthCount) {
            if (0 != mad_frame_decode(&m_madFrame, &m_madStream)) {
                if (MAD_RECOVERABLE(m_madStream.error)) {
                    if (MAD_ERROR_LOSTSYNC == m_madStream.error) {
                        // Ignore LOSTSYNC due to ID3 tags
                        mad_skip_id3_tag(&m_madStream);
                    } else {
                        qDebug() << "Recoverable MP3 decoding error:" << mad_stream_errorstr(&m_madStream);
                    }
                    continue;
                } else {
                    if (MAD_ERROR_BUFLEN != m_madStream.error) {
                        qWarning() << "Unrecoverable MP3 decoding error:" << mad_stream_errorstr(&m_madStream);
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
        const size_type madSynthOffset = m_madSynth.pcm.length - m_madSynthCount;
        const size_type framesRead = math_min(m_madSynthCount, framesRemaining);
        m_madSynthCount -= framesRead;
        m_curFrameIndex += framesRead;
        framesRemaining -= framesRead;
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
    return frameCount - framesRemaining;
}

Result SoundSourceMp3::parseMetadata(Mixxx::TrackMetadata* pMetadata) {
    QByteArray qBAFilename(getFilename().toLocal8Bit());
    TagLib::MPEG::File f(qBAFilename.constData());

    if (!readAudioProperties(pMetadata, f)) {
        return ERR;
    }

    // Now look for MP3 specific metadata (e.g. BPM)
    TagLib::ID3v2::Tag* id3v2 = f.ID3v2Tag();
    if (id3v2) {
        readID3v2Tag(pMetadata, *id3v2);
    } else {
        TagLib::APE::Tag *ape = f.APETag();
        if (ape) {
            readAPETag(pMetadata, *ape);
        } else {
            // fallback
            const TagLib::Tag *tag(f.tag());
            if (tag) {
                readTag(pMetadata, *tag);
            } else {
                return ERR;
            }
        }
    }

    return OK;
}

QImage SoundSourceMp3::parseCoverArt() {
    QImage coverArt;
    TagLib::MPEG::File f(getFilename().toLocal8Bit().constData());
    TagLib::ID3v2::Tag* id3v2 = f.ID3v2Tag();
    if (id3v2) {
        coverArt = Mixxx::readID3v2TagCover(*id3v2);
    }
    if (coverArt.isNull()) {
        TagLib::APE::Tag *ape = f.APETag();
        if (ape) {
            coverArt = Mixxx::readAPETagCover(*ape);
        }
    }
    return coverArt;
}
