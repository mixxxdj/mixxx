#include "soundsourcewv.h"

#include "trackmetadatataglib.h"
#include "sampleutil.h"

#include <taglib/wavpackfile.h>

namespace Mixxx {

QList<QString> SoundSourceWV::supportedFileExtensions() {
    QList<QString> list;
    list.push_back("wv");
    return list;
}

SoundSourceWV::SoundSourceWV(QString qFilename)
        : Super(qFilename, "wv"), m_wpc(NULL), m_sampleScale(0.0f) {
}

SoundSourceWV::~SoundSourceWV() {
    if (m_wpc) {
        WavpackCloseFile(m_wpc);
    }
}

Result SoundSourceWV::open() {
    char msg[80]; // hold possible error message
    m_wpc = WavpackOpenFileInput(getFilename().toLocal8Bit().constData(), msg,
            OPEN_2CH_MAX | OPEN_WVC | OPEN_NORMALIZE, 0);
    if (!m_wpc) {
        qDebug() << "SSWV::open: failed to open file : " << msg;
        return ERR;
    }

    setChannelCount(WavpackGetReducedChannels(m_wpc));
    setFrameRate(WavpackGetSampleRate(m_wpc));
    setFrameCount(WavpackGetNumSamples(m_wpc));

    if (WavpackGetMode(m_wpc) & MODE_FLOAT) {
        m_sampleScale = 1.0f;
    } else {
        const int bitsPerSample = WavpackGetBitsPerSample(m_wpc);
        const uint32_t maxSampleValue = uint32_t(1) << (bitsPerSample - 1);
        m_sampleScale = 1.0f / (0.5f + sample_type(maxSampleValue));
    }

    return OK;
}

AudioSource::diff_type SoundSourceWV::seekFrame(diff_type frameIndex) {
    if (WavpackSeekSample(m_wpc, frameIndex) == TRUE) {
        return frameIndex;
    } else {
        qDebug() << "SSWV::seek : could not seek to frame #" << frameIndex;
        return WavpackGetSampleIndex(m_wpc);
    }
}

AudioSource::size_type SoundSourceWV::readFrameSamplesInterleaved(
        size_type frameCount, sample_type* sampleBuffer) {
    // static assert: sizeof(sample_type) == sizeof(int32_t)
    size_type unpackCount = WavpackUnpackSamples(m_wpc,
            reinterpret_cast<int32_t*>(sampleBuffer), frameCount);
    if (!(WavpackGetMode(m_wpc) & MODE_FLOAT)) {
        // signed integer -> float
        const size_type sampleCount = frames2samples(unpackCount);
        for (size_type i = 0; i < sampleCount; ++i) {
            const int32_t sampleValue =
                    reinterpret_cast<int32_t*>(sampleBuffer)[i];
            sampleBuffer[i] = SampleUtil::clampSample(
                    sampleValue * m_sampleScale);
        }
    }
    return unpackCount;
}

Result SoundSourceWV::parseMetadata(Mixxx::TrackMetadata* pMetadata) {
    TagLib::WavPack::File f(getFilename().toLocal8Bit().constData());

    if (!readAudioProperties(pMetadata, f)) {
        return ERR;
    }

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

    return OK;
}

QImage SoundSourceWV::parseCoverArt() {
    TagLib::WavPack::File f(getFilename().toLocal8Bit().constData());
    TagLib::APE::Tag *ape = f.APETag();
    if (ape) {
        return Mixxx::readAPETagCover(*ape);
    } else {
        return QImage();
    }
}

}  // namespace Mixxx
