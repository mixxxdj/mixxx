/***************************************************************************
 soundsourceoggvorbis.cpp  -  ogg vorbis decoder
 -------------------
 copyright            : (C) 2003 by Svein Magne Bang
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

#include "soundsourceoggvorbis.h"
#include "soundsourcetaglib.h"

#include <taglib/vorbisfile.h>

#include <vorbis/codec.h>

SoundSourceOggVorbis::SoundSourceOggVorbis(QString qFilename)
<<<<<<< HEAD
        : Mixxx::SoundSource(qFilename),
          channels(0),
          filelength(0),
          current_section(0) {
    setType("ogg");
    vf.datasource = NULL;
    vf.seekable = 0;
    vf.offset = 0;
    vf.end = 0;
    //vf.oy
    vf.links = 0;
    vf.offsets = NULL;
    vf.dataoffsets = NULL;
    vf.serialnos = NULL;
    vf.pcmlengths = NULL;
    vf.vi = NULL;
    vf.vc = NULL;
    vf.pcm_offset = 0;
    vf.ready_state = 0;
    vf.current_serialno = 0;
    vf.current_link = 0;
    vf.bittrack = 0;
    vf.samptrack = 0;
    //vf.os
    //vf.vd
    //vf.vb
    //vf.callbacks
=======
        : Super(qFilename, "ogg") {
    memset(&m_vf, 0, sizeof(m_vf));
>>>>>>> New SoundSource/AudioSource API
}

SoundSourceOggVorbis::~SoundSourceOggVorbis() {
    if (0 != ov_clear(&m_vf)) {
        qWarning() << "Failed to close OggVorbis file:" << getFilename();
    }
}

Result SoundSourceOggVorbis::open() {
    const QByteArray qBAFilename(getFilename().toLocal8Bit());

    if (0 != ov_fopen(qBAFilename.constData(), &m_vf)) {
        qWarning() << "Failed to open OggVorbis file:" << getFilename();
        return ERR;
    }

    if (!ov_seekable(&m_vf)) {
        qWarning() << "OggVorbis file is not seekable:" << getFilename();
        close();
        return ERR;
    }

    // lookup the ogg's channels and samplerate
    const vorbis_info* vi = ov_info(&m_vf, -1);
    if (!vi) {
        qWarning() << "Failed to read OggVorbis file:" << getFilename();
        close();
        return ERR;
    }
    setChannelCount(vi->channels);
    setFrameRate(vi->rate);

    ogg_int64_t frameCount = ov_pcm_total(&m_vf, -1);
    if (0 <= frameCount) {
        setFrameCount(frameCount);
    } else {
        qWarning() << "Failed to read OggVorbis file:" << getFilename();
        close();
        return ERR;
    }

    return OK;
}

void SoundSourceOggVorbis::close() {
    if (0 != ov_clear(&m_vf)) {
        qWarning() << "Failed to close OggVorbis file:" << getFilename();
    }
    Super::reset();
}

Mixxx::AudioSource::diff_type SoundSourceOggVorbis::seekFrame(
        diff_type frameIndex) {
    int seekResult = ov_pcm_seek(&m_vf, frameIndex);
    if (0 != seekResult) {
        qWarning() << "Failed to seek OggVorbis file:" << getFilename();
    }
    return ov_pcm_tell(&m_vf);
}

Mixxx::AudioSource::size_type SoundSourceOggVorbis::readFrameSamplesInterleaved(
        size_type frameCount, sample_type* sampleBuffer) {
    return readFrameSamplesInterleaved(frameCount, sampleBuffer, false);
}

Mixxx::AudioSource::size_type SoundSourceOggVorbis::readStereoFrameSamplesInterleaved(
        size_type frameCount, sample_type* sampleBuffer) {
    return readFrameSamplesInterleaved(frameCount, sampleBuffer, true);
}

Mixxx::AudioSource::size_type SoundSourceOggVorbis::readFrameSamplesInterleaved(
        size_type frameCount, sample_type* sampleBuffer,
        bool readStereoSamples) {
    size_type readCount = 0;
    sample_type* nextSample = sampleBuffer;
    while (readCount < frameCount) {
        float** pcmChannels;
        int currentSection;
        long readResult = ov_read_float(&m_vf, &pcmChannels,
                frameCount - readCount, &currentSection);
        if (0 == readResult) {
            break; // done
        }
        if (0 < readResult) {
            if (isChannelCountMono()) {
                if (readStereoSamples) {
                    for (long i = 0; i < readResult; ++i) {
                        *nextSample++ = pcmChannels[0][i];
                        *nextSample++ = pcmChannels[0][i];
                    }
                } else {
                    for (long i = 0; i < readResult; ++i) {
                        *nextSample++ = pcmChannels[0][i];
                    }
                }
            } else if (isChannelCountStereo() || readStereoSamples) {
                for (long i = 0; i < readResult; ++i) {
                    *nextSample++ = pcmChannels[0][i];
                    *nextSample++ = pcmChannels[1][i];
                }
            } else {
                for (long i = 0; i < readResult; ++i) {
                    for (size_type j = 0; j < getChannelCount(); ++j) {
                        *nextSample++ = pcmChannels[j][i];
                    }
                }
            }
            readCount += readResult;
        } else {
            qWarning() << "Failed to read sample data from OggVorbis file:"
                    << getFilename();
            break; // abort
        }
    }
    return readCount;
}

/*
 Parse the the file to get metadata
 */
Result SoundSourceOggVorbis::parseHeader() {
    QByteArray qBAFilename = getFilename().toLocal8Bit();
    TagLib::Ogg::Vorbis::File f(qBAFilename.constData());

    if (!readFileHeader(this, f)) {
        return ERR;
    }

    TagLib::Ogg::XiphComment *xiph = f.tag();
    if (xiph) {
        readXiphComment(this, *xiph);
    } else {
        // fallback
        const TagLib::Tag *tag(f.tag());
        if (tag) {
            readTag(this, *tag);
        } else {
            return ERR;
        }
    }

    return OK;
}

QImage SoundSourceOggVorbis::parseCoverArt() {
    TagLib::Ogg::Vorbis::File f(getFilename().toLocal8Bit().constData());
    TagLib::Ogg::XiphComment *xiph = f.tag();
    if (xiph) {
        return Mixxx::getCoverInXiphComment(*xiph);
    } else {
        return QImage();
    }
}

/*
 Return the length of the file in samples.
 */

QList<QString> SoundSourceOggVorbis::supportedFileExtensions() {
    QList<QString> list;
    list.push_back("ogg");
    return list;
}
