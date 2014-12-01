#include "soundsourcemodplug.h"

#include <stdlib.h>
#include <unistd.h>

#include <QFile>
#include <QtDebug>

#include "util/timer.h"

/* read files in 512k chunks */
#define CHUNKSIZE (1 << 18)

// reserve some static space for settings...
namespace
{
    static unsigned int s_bufferSizeLimit; // max track buffer length (bytes)
    static ModPlug::ModPlug_Settings s_settings; // modplug decoder parameters
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

void SoundSourceModPlug::configure(unsigned int bufferSizeLimit,
        const ModPlug::ModPlug_Settings &settings) {
    s_bufferSizeLimit = bufferSizeLimit;
    s_settings = settings;
    ModPlug::ModPlug_SetSettings(&s_settings);
}

namespace
{
    QString getTypeFromFilename(QString filename) {
        const QString fileext(filename.section(".", -1).toLower());
        if (fileext == "mod") {
            return "Protracker";
        } else if (fileext == "med") {
            return "OctaMed";
        } else if (fileext == "okt") {
            return "Oktalyzer";
        } else if (fileext == "s3m") {
            return "Scream Tracker 3";
        } else if (fileext == "stm") {
            return "Scream Tracker";
        } else if (fileext == "xm") {
            return "FastTracker2";
        } else if (fileext == "it") {
            return "Impulse Tracker";
        } else {
            return "Module";
        }
    }
}

SoundSourceModPlug::SoundSourceModPlug(QString qFilename)
        : Super(qFilename, getTypeFromFilename(qFilename)), m_pModFile(NULL), m_fileLength(0), m_seekPos(0) {
    setChannelCount(2); // always stereo
    setFrameRate(44100); // always 44.1kHz
}

SoundSourceModPlug::~SoundSourceModPlug() {
    if (m_pModFile) {
        ModPlug::ModPlug_Unload(m_pModFile);
    }
}

Result SoundSourceModPlug::open() {
    ScopedTimer t("SoundSourceModPlug::open()");

    qDebug() << "[ModPlug] Loading ModPlug module " << getFilename();

    // read module file to byte array
    QFile modFile(getFilename());
    modFile.open(QIODevice::ReadOnly);
    m_fileBuf = modFile.readAll();
    modFile.close();
    // get ModPlugFile descriptor for later access
    m_pModFile = ModPlug::ModPlug_Load(m_fileBuf.constData(),
            m_fileBuf.length());

    if (m_pModFile == NULL) {
        // an error occured
        t.cancel();
        qDebug() << "[ModPlug] Could not load module file: " << getFilename();
        return ERR;
    }

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

Result SoundSourceModPlug::parseHeader() {
    if (m_pModFile == NULL) {
        // an error occured
        qDebug() << "Could not parse module header of " << getFilename();
        return ERR;
    }

    setComment(QString(ModPlug::ModPlug_GetMessage(m_pModFile)));
    setTitle(QString(ModPlug::ModPlug_GetName(m_pModFile)));
    setDuration(ModPlug::ModPlug_GetLength(m_pModFile) / 1000);
    setBitrate(8); // not really, but fill in something...
    return OK;
}

QImage SoundSourceModPlug::parseCoverArt() {
    // The modplug library currently does not support reading cover-art from
    // modplug files -- kain88 (Oct 2014)
    return QImage();
}
