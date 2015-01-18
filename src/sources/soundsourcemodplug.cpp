#include "sources/soundsourcemodplug.h"

#include "sources/audiosourcemodplug.h"
#include "metadata/trackmetadata.h"
#include "util/timer.h"

#include <stdlib.h>
#include <unistd.h>

#include <QtDebug>

namespace {
QString getTypeFromFilename(QString fileName) {
    const QString fileExt(fileName.section(".", -1).toLower());
    if (fileExt == "mod") {
        return "Protracker";
    } else if (fileExt == "med") {
        return "OctaMed";
    } else if (fileExt == "okt") {
        return "Oktalyzer";
    } else if (fileExt == "s3m") {
        return "Scream Tracker 3";
    } else if (fileExt == "stm") {
        return "Scream Tracker";
    } else if (fileExt == "xm") {
        return "FastTracker2";
    } else if (fileExt == "it") {
        return "Impulse Tracker";
    } else {
        return "Module";
    }
}
}

QList<QString> SoundSourceModPlug::supportedFileExtensions() {
    QList<QString> list;
    // ModPlug supports more formats but file name
    // extensions are not always present with modules.
    list.push_back("mod");
    list.push_back("med");
    list.push_back("okt");
    list.push_back("s3m");
    list.push_back("stm");
    list.push_back("xm");
    list.push_back("it");
    return list;
}

SoundSourceModPlug::SoundSourceModPlug(QString fileName) :
        SoundSource(fileName, getTypeFromFilename(fileName)) {
}

Result SoundSourceModPlug::parseMetadata(
        Mixxx::TrackMetadata* pMetadata) const {
    QFile modFile(getFilename());
    modFile.open(QIODevice::ReadOnly);
    const QByteArray fileBuf(modFile.readAll());
    modFile.close();

    ModPlug::ModPlugFile* pModFile = ModPlug::ModPlug_Load(fileBuf.constData(),
            fileBuf.length());
    if (NULL != pModFile) {
        pMetadata->setComment(QString(ModPlug::ModPlug_GetMessage(pModFile)));
        pMetadata->setTitle(QString(ModPlug::ModPlug_GetName(pModFile)));
        pMetadata->setDuration(ModPlug::ModPlug_GetLength(pModFile) / 1000);
        pMetadata->setBitrate(8); // not really, but fill in something...
        ModPlug::ModPlug_Unload(pModFile);
    }

<<<<<<< HEAD
    // estimate size of sample buffer (for better performance)
    // beware: module length estimation is unreliable due to loops
    // song milliseconds * 2 (bytes per sample)
    //                   * 2 (channels)
    //                   * 44.1 (samples per millisecond)
    //                   + some more to accomodate short loops etc.
    // approximate and align with CHUNKSIZE yields:
    // (((milliseconds << 1) >> 10 /* to seconds */)
    //      div 11 /* samples to chunksize ratio */)
    //      << 19 /* align to chunksize */
    unsigned int estimate = ((ModPlug::ModPlug_GetLength(m_pModFile) >> 8) / 11) << 18;
    estimate = math_min(estimate, s_bufferSizeLimit);
    m_sampleBuf.reserve(estimate);
    qDebug() << "[ModPlug] Reserved " << m_sampleBuf.capacity()
            << " #samples";

    // decode samples to sample buffer
    int samplesRead = -1;
    int currentSize = 0;
    while ((samplesRead != 0) && (m_sampleBuf.size() < s_bufferSizeLimit)) {
        // reserve enough space in sample buffer
        m_sampleBuf.resize(currentSize + CHUNKSIZE);
        samplesRead = ModPlug::ModPlug_Read(m_pModFile,
                m_sampleBuf.data() + currentSize,
                CHUNKSIZE * 2) / 2;
        // adapt to actual size
        currentSize += samplesRead;
        if (samplesRead != CHUNKSIZE) {
            m_sampleBuf.resize(currentSize);
            samplesRead = 0; // we reached the end of the file
        }
    }
    qDebug() << "[ModPlug] Filled Sample buffer with " << m_sampleBuf.size()
            << " samples.";
    qDebug() << "[ModPlug] Sample buffer has "
            << m_sampleBuf.capacity() - m_sampleBuf.size()
            << " samples unused capacity.";

    setFrameCount(samples2frames(m_sampleBuf.size()));
    m_seekPos = 0;

<<<<<<< HEAD
    // read all other track information
    return parseHeader();
=======
    return OK;
>>>>>>> New SoundSource/AudioSource API
}

Mixxx::AudioSource::diff_type SoundSourceModPlug::seekFrame(
        diff_type frameIndex) {
    return m_seekPos = frameIndex;
}

Mixxx::AudioSource::size_type SoundSourceModPlug::readFrameSamplesInterleaved(
        size_type frameCount, sample_type* sampleBuffer) {
    const size_type maxFrames = samples2frames(m_sampleBuf.size());
    const size_type readFrames = math_min(maxFrames - m_seekPos, frameCount);

    const size_type readSamples = frames2samples(readFrames);
    const size_type readOffset = frames2samples(m_seekPos);
    for (size_type i = 0; i < readSamples; ++i) {
        sampleBuffer[i] = SAMPLE_clampSymmetric(m_sampleBuf[readOffset + i]) / sample_type(SAMPLE_MAX);
    }

    m_seekPos += readFrames;
    return readFrames;
}

Result SoundSourceModPlug::parseMetadata(Mixxx::TrackMetadata* pMetadata) {
    if (m_pModFile == NULL) {
        // an error occured
        qDebug() << "Could not parse module header of " << getFilename();
        return ERR;
    }

    pMetadata->setComment(QString(ModPlug::ModPlug_GetMessage(m_pModFile)));
    pMetadata->setTitle(QString(ModPlug::ModPlug_GetName(m_pModFile)));
    pMetadata->setDuration(ModPlug::ModPlug_GetLength(m_pModFile) / 1000);
    pMetadata->setBitrate(8); // not really, but fill in something...

    return OK;
=======
    return pModFile ? OK : ERR;
>>>>>>> Split AudioSource from SoundSource
}

QImage SoundSourceModPlug::parseCoverArt() const {
    // The modplug library currently does not support reading cover-art from
    // modplug files -- kain88 (Oct 2014)
    return QImage();
}

Mixxx::AudioSourcePointer SoundSourceModPlug::open() const {
    return Mixxx::AudioSourceModPlug::create(QUrl::fromLocalFile(getFilename()));
}
