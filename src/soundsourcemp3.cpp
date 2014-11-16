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

#include <QtDebug>


SoundSourceMp3::SoundSourceMp3(QString qFilename)
    : Mixxx::SoundSource(qFilename)
    , m_file(qFilename)
    , inputbuf(NULL)
    , inputbuf_len(0)
    , framecount(0)
    , currentframe(0)
    , pos(mad_timer_zero)
    , filelength(mad_timer_zero)
    , units(MAD_UNITS_44100_HZ)
    , m_madSynthOffset(0)
    , m_currentSeekFrameIndex(0)
    , m_iAvgFrameSize(0)
{
    setType("mp3");
    mad_stream_init(&madStream);
    mad_synth_init(&m_madSynth);
    mad_frame_init(&madFrame);
}

SoundSourceMp3::~SoundSourceMp3()
{
    closeThis();
}

QList<QString> SoundSourceMp3::supportedFileExtensions()
{
    QList<QString> list;
    list.push_back("mp3");
    return list;
}

Result SoundSourceMp3::open() {
    m_file.setFileName(getFilename());
    if (!m_file.open(QIODevice::ReadOnly)) {
        //qDebug() << "MAD: Open failed:" << getFilename();
        return ERR;
    }

    // Get a pointer to the file using memory mapped IO
    inputbuf_len = m_file.size();
    inputbuf = m_file.map(0, inputbuf_len);

    // Transfer it to the mad stream-buffer:
    mad_stream_init(&madStream);
    mad_stream_options(&madStream, MAD_OPTION_IGNORECRC);
    mad_stream_buffer(&madStream, inputbuf, inputbuf_len);

    /*
       Decode all the headers, and fill in stats:
     */
    mad_header madHeader;
    mad_header_init(&madHeader);
    mad_timer_t filelength = mad_timer_zero;
    int sumBitrate = 0;
    currentframe = 0;
    pos = mad_timer_zero;

    while ((madStream.bufend - madStream.this_frame) > 0)
    {
        if (mad_header_decode (&madHeader, &madStream) == -1) {
            if (!MAD_RECOVERABLE (madStream.error)) {
                break;
            }
            if (madStream.error == MAD_ERROR_LOSTSYNC) {
                // ignore LOSTSYNC due to ID3 tags
                int tagsize = id3_tag_query (madStream.this_frame, madStream.bufend - madStream.this_frame);
                if (tagsize > 0) {
                    //qDebug() << "SSMP3::SSMP3() : skipping ID3 tag size " << tagsize;
                    mad_stream_skip (&madStream, tagsize);
                    continue;
                }
            }

            // qDebug() << "MAD: ERR decoding header "
            //          << currentframe << ": "
            //          << mad_stream_errorstr(Stream)
            //          << " (len=" << mad_timer_count(filelength,MAD_UNITS_MILLISECONDS)
            //          << ")";
            continue;
        }

        // Grab data from madHeader

        setChannelCount(MAD_NCHANNELS(&madHeader));

        // This warns us only when the reported sample rate changes. (and when
        // it is first set)
        if (getSampleRate() == 0 && madHeader.samplerate > 0) {
            setSampleRate(madHeader.samplerate);
        } else if (getSampleRate() != madHeader.samplerate) {
            qDebug() << "SSMP3: file has differing samplerate in some headers:"
                     << getFilename()
                     << getSampleRate() << "vs" << madHeader.samplerate;
        }

        switch (getSampleRate())
        {
        case 8000:
            units = MAD_UNITS_8000_HZ;
            break;
        case 11025:
            units = MAD_UNITS_11025_HZ;
            break;
        case 12000:
            units = MAD_UNITS_12000_HZ;
            break;
        case 16000:
            units = MAD_UNITS_16000_HZ;
            break;
        case 22050:
            units = MAD_UNITS_22050_HZ;
            break;
        case 24000:
            units = MAD_UNITS_24000_HZ;
            break;
        case 32000:
            units = MAD_UNITS_32000_HZ;
            break;
        case 44100:
            units = MAD_UNITS_44100_HZ;
            break;
        case 48000:
            units = MAD_UNITS_48000_HZ;
            break;
        default:             //By the MP3 specs, an MP3 _has_ to have one of the above samplerates...
            qWarning() << "MP3 with invalid sample rate (" << getSampleRate() << "), defaulting to 44100";
            setSampleRate(44100); //Prevents division by zero errors.
            units = MAD_UNITS_44100_HZ;
        }

        mad_timer_add (&filelength, madHeader.duration);
        sumBitrate += madHeader.bitrate;

        // Add frame to list of frames
        MadSeekFrameType * p = new MadSeekFrameType;
        p->m_pStreamPos = (unsigned char *)madStream.this_frame;
        p->pos = mad_timer_count(filelength, units);
        m_qSeekList.append(p);
        currentframe++;
    }
    //qDebug() << "channels " << m_iChannels;

    mad_header_finish (&Header); // This is a macro for nothing.

    // This is not a working MP3 file.
    if (currentframe == 0) {
        qDebug() << "SSMP3: This is not a working MP3 file:" << getFilename();
        return ERR;
    }

    framecount = currentframe;
    setFrameCount(mad_timer_count(filelength, units));

    // Recalculate the duration by using the average frame size. Our first guess at
    // the duration of VBR MP3s in parseHeader() goes for speed over accuracy
    // since it runs during a library scan. When we open() an MP3 for playback,
    // we had to seek through the entire thing to build a seek table, so we've
    // also counted the number of frames in it. We need that to better estimate
    // the length of VBR MP3s.
    if (0 < getSampleRate()) {
        int exactDuration = getFrameCount() / getSampleRate();
        setDuration(exactDuration);
    }
    if (0 < framecount) {
        m_iAvgFrameSize = getFrameCount() / framecount;
        int avgBitrate = sumBitrate / framecount;
        setBitrate(avgBitrate);
    } else {
        m_iAvgFrameSize = 0;
    }
    //TODO: Emit metadata updated signal?

    // Re-init buffer:
    seekFrame(0);
    currentframe = 0;

    return OK;
}

void SoundSourceMp3::closeThis()
{
    mad_stream_finish(&madStream);
    mad_frame_finish(&madFrame);
    mad_synth_finish(&m_madSynth);

    m_file.unmap(inputbuf);
    inputbuf = 0;
    inputbuf_len = 0;

    m_file.close();

    //Free the pointers in our seek list, LIBERATE THEM!!!
    for (int i = 0; i < m_qSeekList.count(); i++)
    {
        delete m_qSeekList[i];
    }
    m_qSeekList.clear();
}

void SoundSourceMp3::close()
{
    closeThis();
    Super::close();
}

MadSeekFrameType* SoundSourceMp3::getSeekFrame(long frameIndex) const {
    if (frameIndex < 0 || frameIndex >= m_qSeekList.size()) {
        return NULL;
    }
    return m_qSeekList.at(frameIndex);
}

namespace
{
    const int kSeekSyncFrameCount = 4; // required for synchronization
}

Mixxx::AudioSource::diff_type SoundSourceMp3::seekFrame(diff_type frameIndex) {
    MadSeekFrameType* cur = NULL;
    if (frameIndex == 0) {
        // Seek to beginning of file

        // Re-init buffer:
        mad_stream_finish(&madStream);
        mad_stream_init(&madStream);
        mad_stream_options(&madStream, MAD_OPTION_IGNORECRC);
        mad_stream_buffer(&madStream, (unsigned char *) inputbuf, inputbuf_len);
        mad_frame_init(&madFrame);
        mad_synth_init(&m_madSynth);
        m_madSynthOffset = 0;
        m_currentSeekFrameIndex = 0;
        cur = getSeekFrame(m_currentSeekFrameIndex);
    } else {
        int framePos = findFrame(frameIndex);
        if (framePos == 0 || framePos > frameIndex || m_currentSeekFrameIndex <= kSeekSyncFrameCount) {
            qDebug() << "Problem finding good seek frame (wanted " << frameIndex << ", got " << framePos << "), starting from 0";

            // Re-init buffer:
            mad_stream_finish(&madStream);
            mad_stream_init(&madStream);
            mad_stream_options(&madStream, MAD_OPTION_IGNORECRC);
            mad_stream_buffer(&madStream, (unsigned char *) inputbuf, inputbuf_len);
            mad_frame_init(&madFrame);
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
                mad_stream_finish(&madStream);
                mad_stream_init(&madStream);
                mad_stream_options(&madStream, MAD_OPTION_IGNORECRC);
                //        qDebug() << "mp3 restore " << cur->m_pStreamPos;
                mad_stream_buffer(&madStream, (const unsigned char *)cur->m_pStreamPos,
                                  inputbuf_len-(long int)(cur->m_pStreamPos-(unsigned char *)inputbuf));

                // Mute'ing is done here to eliminate potential pops/clicks from skipping
                // Rob Leslie explains why here:
                // http://www.mars.org/mailman/public/mad-dev/2001-August/000321.html
                mad_synth_mute(&m_madSynth);
                mad_frame_mute(&madFrame);

                // Decode the three frames before
                mad_frame_decode(&madFrame, &madStream);
                mad_frame_decode(&madFrame, &madStream);
                mad_frame_decode(&madFrame, &madStream);
                mad_frame_decode(&madFrame, &madStream);

                // this is also explained in the above mad-dev post
                mad_synth_frame(&m_madSynth, &madFrame);

                // Set current position
                m_madSynthOffset = 0;
                m_currentSeekFrameIndex += 4;
                cur = getSeekFrame(m_currentSeekFrameIndex);
            }
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
Mixxx::AudioSource::size_type SoundSourceMp3::discardFrames(size_type frameCount) {
    size_type framesDiscarded = 0;

    while (framesDiscarded < frameCount) {
        if (0 == m_madSynthOffset) {
            if (mad_frame_decode(&madFrame, &madStream)) {
                if (MAD_RECOVERABLE(madStream.error)) {
                    continue;
                } else if(madStream.error == MAD_ERROR_BUFLEN) {
                    break;
                } else {
                    break;
                }
            }
            mad_synth_frame(&m_madSynth, &madFrame);
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

Mixxx::AudioSource::size_type SoundSourceMp3::readFrameSamplesInterleaved(size_type frameCount, sample_type* sampleBuffer) {
    return readFrameSamplesInterleaved(frameCount, sampleBuffer, false);
}

Mixxx::AudioSource::size_type SoundSourceMp3::readStereoFrameSamplesInterleaved(size_type frameCount, sample_type* sampleBuffer) {
    return readFrameSamplesInterleaved(frameCount, sampleBuffer, true);
}

namespace
{
    const Mixxx::AudioSource::sample_type kMadScale = 1.0f / Mixxx::AudioSource::sample_type(mad_fixed_t(1) << MAD_F_FRACBITS);

    inline
    Mixxx::AudioSource::sample_type madScale(mad_fixed_t sample) {
        return sample * kMadScale;
    }
}

Mixxx::AudioSource::size_type SoundSourceMp3::readFrameSamplesInterleaved(size_type frameCount, sample_type* sampleBuffer, bool readStereoSamples) {
    size_type framesRead = 0;
    sample_type* sampleBufferPtr = sampleBuffer;

//     qDebug() << "Decoding";
    while (framesRead < frameCount) {
        // qDebug() << "no " << framesRead;
        if (0 == m_madSynthOffset) {
            if (mad_frame_decode(&madFrame, &madStream)) {
                if (MAD_RECOVERABLE(madStream.error)) {
                    if (madStream.error == MAD_ERROR_LOSTSYNC) {
                        // Ignore LOSTSYNC due to ID3 tags
                        int tagsize = id3_tag_query(madStream.this_frame, madStream.bufend - madStream.this_frame);
                        if (tagsize > 0) {
                            //qDebug() << "SSMP3::Read Skipping ID3 tag size: " << tagsize;
                            mad_stream_skip(&madStream, tagsize);
                        }
                        continue;
                    }
                    //qDebug() << "MAD: Recoverable frame level ERR (" << mad_stream_errorstr(Stream) << ")";
                    continue;
                } else if (madStream.error == MAD_ERROR_BUFLEN) {
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
            mad_synth_frame(&m_madSynth, &madFrame);
            //qDebug() << "synthlen " << m_madSynth.pcm.length << ", remain " << (frameCount - framesRead);
        }

        size_type no = math_min(size_type(m_madSynth.pcm.length - m_madSynthOffset), frameCount - framesRead);
        if (isChannelCountMono()) {
            for (size_type i = 0; i < no; ++i) {
                *(sampleBufferPtr++) = madScale(m_madSynth.pcm.samples[0][m_madSynthOffset + i]);
                if (readStereoSamples) {
                    *(sampleBufferPtr++) = madScale(m_madSynth.pcm.samples[0][m_madSynthOffset + i]);
                }
            }
        } else if (isChannelCountStereo() || readStereoSamples) {
            for (size_type i = 0; i < no; ++i) {
                *(sampleBufferPtr++) = madScale(m_madSynth.pcm.samples[0][m_madSynthOffset + i]);
                *(sampleBufferPtr++) = madScale(m_madSynth.pcm.samples[1][m_madSynthOffset + i]);
            }
        } else {
            for (size_type i = 0; i < no; ++i) {
                for (size_type j = 0; j < getChannelCount(); ++j) {
                    *(sampleBufferPtr++) = madScale(m_madSynth.pcm.samples[j][m_madSynthOffset + i]);
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

int SoundSourceMp3::findFrame(int pos)
{
    // Guess position of frame in m_qSeekList based on average frame size
    m_currentSeekFrameIndex = math_min((unsigned int) m_qSeekList.count()-1,
                                       m_iAvgFrameSize ? (unsigned int)(pos/m_iAvgFrameSize) : 0);
    MadSeekFrameType* temp = getSeekFrame(m_currentSeekFrameIndex);

/*
    if (temp!=0)
        qDebug() << "find " << pos << ", got " << temp->pos;
    else
        qDebug() << "find " << pos << ", tried idx " << math_min(m_qSeekList.count()-1 << ", total " << pos/m_iAvgFrameSize);
 */

    // Ensure that the list element is not at a greater position than pos
    while (temp != NULL && temp->pos > pos)
    {
        m_currentSeekFrameIndex--;
        temp = getSeekFrame(m_currentSeekFrameIndex);
//        if (temp!=0) qDebug() << "backing " << pos << ", got " << temp->pos;
    }

    // Ensure that the following position is also not smaller than pos
    if (temp != NULL)
    {
        temp = getSeekFrame(m_currentSeekFrameIndex);
        while (temp != NULL && temp->pos < pos)
        {
            m_currentSeekFrameIndex++;
            temp = getSeekFrame(m_currentSeekFrameIndex);
//            if (temp!=0) qDebug() << "fwd'ing " << pos << ", got " << temp->pos;
        }

        if (temp == NULL)
            m_currentSeekFrameIndex = m_qSeekList.count()-1;
        else
            m_currentSeekFrameIndex--;

        temp = getSeekFrame(m_currentSeekFrameIndex);
    }

    if (temp != NULL)
    {
//        qDebug() << "ended at " << pos << ", got " << temp->pos;
        return temp->pos;
    }
    else
    {
//        qDebug() << "ended at 0";
        return 0;
    }
}
