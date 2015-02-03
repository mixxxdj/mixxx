#include "sources/soundsourceoggvorbis.h"

#include "sources/audiosourceoggvorbis.h"
#include "metadata/trackmetadatataglib.h"

#include <taglib/vorbisfile.h>

QList<QString> SoundSourceOggVorbis::supportedFileExtensions() {
    QList<QString> list;
    list.push_back("ogg");
    return list;
}

<<<<<<< HEAD
<<<<<<< HEAD
SoundSourceOggVorbis::SoundSourceOggVorbis(QString qFilename)
<<<<<<< HEAD
<<<<<<< HEAD
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
<<<<<<< HEAD
    memset(&m_vf, 0, sizeof(m_vf));
>>>>>>> New SoundSource/AudioSource API
}

SoundSourceOggVorbis::~SoundSourceOggVorbis() {
    if (0 != ov_clear(&m_vf)) {
        qWarning() << "Failed to close OggVorbis file:" << getFilename();
    }
}

Result SoundSourceOggVorbis::open() {

    if (0 != ov_fopen(getFilename().toLocal8Bit().constData(), &m_vf)) {
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
=======
>>>>>>> Split AudioSource from SoundSource
=======
        : SoundSource(qFilename, "ogg") {
>>>>>>> Delete typedef Super (review comments)
=======
    : SoundSource(qFilename, "ogg") {
>>>>>>> Fix coding style issues (indentation)
=======
SoundSourceOggVorbis::SoundSourceOggVorbis(QString qFilename) :
        SoundSource(qFilename, "ogg") {
>>>>>>> Reformat source code (roughly K&R + spaces)
=======
SoundSourceOggVorbis::SoundSourceOggVorbis(QUrl url) :
        SoundSource(url, "ogg") {
>>>>>>> Create SoundSource from URL
}

/*
 Parse the the file to get metadata
 */
Result SoundSourceOggVorbis::parseMetadata(
        Mixxx::TrackMetadata* pMetadata) const {
    TagLib::Ogg::Vorbis::File f(getLocalFileNameBytes().constData());

    if (!readAudioProperties(pMetadata, f)) {
        return ERR;
    }

    TagLib::Ogg::XiphComment *xiph = f.tag();
    if (xiph) {
        readXiphComment(pMetadata, *xiph);
    } else {
        // fallback
        const TagLib::Tag *tag(f.tag());
        if (tag) {
            readTag(pMetadata, *tag);
        } else {
            return ERR;
        }
    }

    return OK;
}

QImage SoundSourceOggVorbis::parseCoverArt() const {
    TagLib::Ogg::Vorbis::File f(getLocalFileNameBytes().constData());
    TagLib::Ogg::XiphComment *xiph = f.tag();
    if (xiph) {
        return Mixxx::readXiphCommentCover(*xiph);
    } else {
        return QImage();
    }
}

Mixxx::AudioSourcePointer SoundSourceOggVorbis::open() const {
    return Mixxx::AudioSourceOggVorbis::create(getUrl());
}
