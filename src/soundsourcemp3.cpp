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
#include "soundsourcetaglib.h"
#include "util/math.h"

#include <taglib/mpegfile.h>

namespace
{
    const int kSeekSyncFrameCount = 4; // required for synchronization

    const Mixxx::AudioSource::sample_type kMadScale =
            Mixxx::AudioSource::kSampleValuePeak
                    / Mixxx::AudioSource::sample_type(
                            mad_fixed_t(1) << MAD_F_FRACBITS);

    inline Mixxx::AudioSource::sample_type madScale(mad_fixed_t sample) {
        return sample * kMadScale;
    }

    // Optimization: Reserve initial capacity for seek frame list
    const size_t kMinutesPerFile = 10; // enough for the majority of files (tunable)
    const size_t kSecondsPerMinute = 60; // fixed
    const size_t kMaxMp3FramesPerSecond = 39; // fixed: 1 MP3 frame = 26 ms -> 1000 / 26
    const size_t kSeekFrameListCapacity = kMinutesPerFile * kSecondsPerMinute * kMaxMp3FramesPerSecond;
}

SoundSourceMp3::SoundSourceMp3(QString qFilename)
        : Super(qFilename, "mp3"),
          m_file(qFilename),
          m_fileSize(0),
          m_pFileData(NULL),
          m_iAvgFrameSize(0),
          m_currentSeekFrameIndex(0),
          m_madSynthOffset(0) {
    mad_stream_init(&m_madStream);
    mad_synth_init(&m_madSynth);
    mad_frame_init(&m_madFrame);
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
        if (mad_header_decode(&madHeader, &m_madStream) == -1) {
            if (MAD_ERROR_BUFLEN == m_madStream.error) {
                // EOF -> done
                break;
            }
            if (m_madStream.error == MAD_ERROR_LOSTSYNC) {
                // ignore LOSTSYNC due to ID3 tags
                int tagsize = id3_tag_query(m_madStream.this_frame,
                        m_madStream.bufend - m_madStream.this_frame);
                if (tagsize > 0) {
                    //qDebug() << "SSMP3::SSMP3() : skipping ID3 tag size " << tagsize;
                    mad_stream_skip(&m_madStream, tagsize);
                    mad_header_finish(&madHeader);
                    continue;
                }
            }
            qWarning() << "Unexpected error in MP3 frame header of file:" << getFilename() << mad_stream_errorstr(&m_madStream);
            // abort
            mad_header_finish(&madHeader);
            close();
            return ERR;
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

        mad_timer_add(&madFileDuration, madHeader.duration);
        sumBitrate += madHeader.bitrate;

        mad_header_finish(&madHeader);

        // Add frame to list of frames
        MadSeekFrameType seekFramePos;
        seekFramePos.pFrameData = m_madStream.this_frame;
        seekFramePos.pos = mad_timer_count(madFileDuration, madUnits);
        m_seekFrameList.push_back(seekFramePos);
        ++madFrameCount;
    }

    if (0 < madFrameCount) {
        m_iAvgFrameSize = getFrameCount() / madFrameCount;
        int avgBitrate = sumBitrate / madFrameCount;
        setBitrate(avgBitrate);
    } else {
        // This is not a working MP3 file.
        qWarning() << "SSMP3: This is not a working MP3 file:" << getFilename();
        // abort
        close();
        return ERR;
    }
    setFrameCount(mad_timer_count(madFileDuration, madUnits));

    // set the actual duration
    setDuration(getFrameCount() / getFrameRate());

    qDebug() << getChannelCount() << getFrameRate() << getFrameCount();

    // restart decoding
    seekFrame(0);

    return OK;
}

void SoundSourceMp3::close() {
    m_seekFrameList.clear();
    m_iAvgFrameSize = 0;
    m_currentSeekFrameIndex = 0;

    mad_synth_finish(&m_madSynth);
    m_madSynthOffset = 0;

    mad_frame_finish(&m_madFrame);

    mad_stream_finish(&m_madStream);

    m_file.unmap(m_pFileData);
    m_fileSize = 0;
    m_pFileData = NULL;

    m_file.close();
}

Mixxx::AudioSource::diff_type SoundSourceMp3::seekFrame(diff_type frameIndex) {
    const MadSeekFrameType* cur = NULL;
    int framePos;
    if (frameIndex == 0) {
        // Seek to beginning of file
        framePos = 0;
    } else {
        framePos = findFrame(frameIndex);
        if (framePos == 0 || framePos > frameIndex
                || m_currentSeekFrameIndex <= kSeekSyncFrameCount) {
            qDebug() << "Problem finding good seek frame (wanted " << frameIndex
                    << ", got " << framePos << "), starting from 0";
            framePos = 0;
        }
    }
    if (framePos == 0) {
        // Re-init buffer:
        mad_stream_finish(&m_madStream);
        mad_stream_init(&m_madStream);
        mad_stream_options(&m_madStream, MAD_OPTION_IGNORECRC);
        mad_stream_buffer(&m_madStream, m_pFileData, m_fileSize);
        mad_frame_init(&m_madFrame);
        mad_synth_init(&m_madSynth);
        m_madSynthOffset = 0;
        m_currentSeekFrameIndex = 0;
        cur = getSeekFrame(m_currentSeekFrameIndex);
    } else {
        //qDebug() << "frame pos " << cur->pos;

        // Start four frames before wanted frame to get in sync...
        m_currentSeekFrameIndex -= kSeekSyncFrameCount;
        cur = getSeekFrame(m_currentSeekFrameIndex);
        if (cur != NULL) {
            // Start from the new frame
            mad_stream_finish(&m_madStream);
            mad_stream_init(&m_madStream);
            mad_stream_options(&m_madStream, MAD_OPTION_IGNORECRC);
            //        qDebug() << "mp3 restore " << cur->m_pStreamPos;
            mad_stream_buffer(&m_madStream, cur->pFrameData,
                    m_fileSize - (cur->pFrameData - m_pFileData));

            // Mute'ing is done here to eliminate potential pops/clicks from skipping
            // Rob Leslie explains why here:
            // http://www.mars.org/mailman/public/mad-dev/2001-August/000321.html
            mad_synth_mute(&m_madSynth);
            mad_frame_mute(&m_madFrame);

            // Decode the three frames before
            mad_frame_decode(&m_madFrame, &m_madStream);
            mad_frame_decode(&m_madFrame, &m_madStream);
            mad_frame_decode(&m_madFrame, &m_madStream);
            mad_frame_decode(&m_madFrame, &m_madStream);

            // this is also explained in the above mad-dev post
            mad_synth_frame(&m_madSynth, &m_madFrame);

            // Set current position
            m_madSynthOffset = 0;
            m_currentSeekFrameIndex += 4;
            cur = getSeekFrame(m_currentSeekFrameIndex);
        }
        // Synthesize the samples from the frame which should be discard to reach the requested position
        if (cur != NULL) { //the "if" prevents crashes on bad files.
            discardFrames(frameIndex - cur->pos);
        }
    }

    // Unfortunately we don't know the exact file position. The returned position is thus an
    // approximation only:
    return frameIndex;
}

/*
 decode the chosen number of samples and discard
 */
Mixxx::AudioSource::size_type SoundSourceMp3::discardFrames(
        size_type frameCount) {
    size_type framesDiscarded = 0;

    while (framesDiscarded < frameCount) {
        if (0 == m_madSynthOffset) {
            if (mad_frame_decode(&m_madFrame, &m_madStream)) {
                if (MAD_RECOVERABLE(m_madStream.error)) {
                    continue;
                } else if (m_madStream.error == MAD_ERROR_BUFLEN) {
                    break;
                } else {
                    break;
                }
            }
            mad_synth_frame(&m_madSynth, &m_madFrame);
        }
        size_type no = math_min(size_type(m_madSynth.pcm.length - m_madSynthOffset), frameCount - framesDiscarded);
        m_madSynthOffset += no;
        if (m_madSynthOffset >= m_madSynth.pcm.length) {
            m_madSynthOffset = 0;
        }
        framesDiscarded += no;
    }

    return framesDiscarded;
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
    size_type framesRead = 0;
    sample_type* sampleBufferPtr = sampleBuffer;

//     qDebug() << "Decoding";
    while (framesRead < frameCount) {
        // qDebug() << "no " << framesRead;
        if (0 == m_madSynthOffset) {
            if (mad_frame_decode(&m_madFrame, &m_madStream)) {
                if (MAD_RECOVERABLE(m_madStream.error)) {
                    if (m_madStream.error == MAD_ERROR_LOSTSYNC) {
                        // Ignore LOSTSYNC due to ID3 tags
                        int tagsize = id3_tag_query(m_madStream.this_frame,
                                m_madStream.bufend - m_madStream.this_frame);
                        if (tagsize > 0) {
                            //qDebug() << "SSMP3::Read Skipping ID3 tag size: " << tagsize;
                            mad_stream_skip(&m_madStream, tagsize);
                        }
                        continue;
                    }
                    //qDebug() << "MAD: Recoverable frame level ERR (" << mad_stream_errorstr(Stream) << ")";
                    continue;
                } else if (m_madStream.error == MAD_ERROR_BUFLEN) {
                    // qDebug() << "MAD: buflen ERR";
                    break;
                } else {
                    // qDebug() << "MAD: Unrecoverable frame level ERR (" << mad_stream_errorstr(Stream) << ").";
                    break;
                }
            }
            /* Once decoded the frame is synthesized to PCM samples. No ERRs
             * are reported by mad_synth_frame();
             */
            mad_synth_frame(&m_madSynth, &m_madFrame);
            //qDebug() << "synthlen " << m_madSynth.pcm.length << ", remain " << (frameCount - framesRead);
        }

        size_type no = math_min(size_type(m_madSynth.pcm.length - m_madSynthOffset), frameCount - framesRead);
        if (isChannelCountMono()) {
            for (size_type i = 0; i < no; ++i) {
                const sample_type sampleValue = madScale(
                        m_madSynth.pcm.samples[0][m_madSynthOffset + i]);
                *(sampleBufferPtr++) = sampleValue;
                if (readStereoSamples) {
                    *(sampleBufferPtr++) = sampleValue;
                }
            }
        } else if (isChannelCountStereo() || readStereoSamples) {
            for (size_type i = 0; i < no; ++i) {
                *(sampleBufferPtr++) = madScale(
                        m_madSynth.pcm.samples[0][m_madSynthOffset + i]);
                *(sampleBufferPtr++) = madScale(
                        m_madSynth.pcm.samples[1][m_madSynthOffset + i]);
            }
        } else {
            for (size_type i = 0; i < no; ++i) {
                for (size_type j = 0; j < getChannelCount(); ++j) {
                    *(sampleBufferPtr++) = madScale(
                            m_madSynth.pcm.samples[j][m_madSynthOffset + i]);
                }
            }
        }
        m_madSynthOffset += no;
        if (m_madSynthOffset >= m_madSynth.pcm.length) {
            m_madSynthOffset = 0;
        }
        framesRead += no;
        //qDebug() << "decoded: " << framesRead << ", wanted: " << frameCount;
    }
    return framesRead;
}

Result SoundSourceMp3::parseHeader() {
    QByteArray qBAFilename(getFilename().toLocal8Bit());
    TagLib::MPEG::File f(qBAFilename.constData());

    if (!readFileHeader(this, f)) {
        return ERR;
    }

    // Now look for MP3 specific metadata (e.g. BPM)
    TagLib::ID3v2::Tag* id3v2 = f.ID3v2Tag();
    if (id3v2) {
        readID3v2Tag(this, *id3v2);
    } else {
        TagLib::APE::Tag *ape = f.APETag();
        if (ape) {
            readAPETag(this, *ape);
        } else {
            // fallback
            const TagLib::Tag *tag(f.tag());
            if (tag) {
                readTag(this, *tag);
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
        coverArt = Mixxx::getCoverInID3v2Tag(*id3v2);
    }
    if (coverArt.isNull()) {
        TagLib::APE::Tag *ape = f.APETag();
        if (ape) {
            coverArt = Mixxx::getCoverInAPETag(*ape);
        }
    }
    return coverArt;
}

int SoundSourceMp3::findFrame(int pos) {
    // Guess position of frame in m_seekFrameList based on average frame size
    m_currentSeekFrameIndex = math_min((unsigned int) m_seekFrameList.size() - 1,
            m_iAvgFrameSize ? (unsigned int)(pos/m_iAvgFrameSize) : 0);

    // Ensure that the list element is not at a greater position than pos
    const MadSeekFrameType* temp = getSeekFrame(m_currentSeekFrameIndex);
    while (temp != NULL && temp->pos > pos) {
        temp = getSeekFrame(--m_currentSeekFrameIndex);
    }

    // Ensure that the following position is also not smaller than pos
    if (temp != NULL) {
        temp = getSeekFrame(m_currentSeekFrameIndex);
        while (temp != NULL && temp->pos < pos) {
            temp = getSeekFrame(++m_currentSeekFrameIndex);
        }
        if (temp == NULL) {
            m_currentSeekFrameIndex = m_seekFrameList.size();
        }
        temp = getSeekFrame(--m_currentSeekFrameIndex);
    }

    if (temp != NULL) {
        return temp->pos;
    } else {
        return 0;
    }
}
