#include "sources/soundsourcemodplug.h"

#include "metadata/trackmetadata.h"
#include "util/timer.h"

#include <QFile>

#include <stdlib.h>
#include <unistd.h>

/* read files in 512k chunks */
#define CHUNKSIZE (1 << 18)

namespace Mixxx {

namespace {

QString getModPlugTypeFromUrl(QUrl url) {
    const QString type(SoundSource::getTypeFromUrl(url));
    if (type == "mod") {
        return "Protracker";
    } else if (type == "med") {
        return "OctaMed";
    } else if (type == "okt") {
        return "Oktalyzer";
    } else if (type == "s3m") {
        return "Scream Tracker 3";
    } else if (type == "stm") {
        return "Scream Tracker";
    } else if (type == "xm") {
        return "FastTracker2";
    } else if (type == "it") {
        return "Impulse Tracker";
    } else {
        return "Module";
    }
}

// identification of modplug module type
enum ModuleTypes {
    NONE = 0x00,
    MOD = 0x01,
    S3M = 0x02,
    XM = 0x04,
    MED = 0x08,
    IT = 0x20,
    STM = 0x100,
    OKT = 0x8000
};

} // anonymous namespace

const SINT SoundSourceModPlug::kChannelCount = 2; // always stereo
const SINT SoundSourceModPlug::kFrameRate = 44100; // always 44.1 kHz

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

unsigned int SoundSourceModPlug::s_bufferSizeLimit = 0;

// reserve some static space for settings...
void SoundSourceModPlug::configure(unsigned int bufferSizeLimit,
        const ModPlug::ModPlug_Settings &settings) {
    s_bufferSizeLimit = bufferSizeLimit;
    ModPlug::ModPlug_SetSettings(&settings);
}

SoundSourceModPlug::SoundSourceModPlug(QUrl url)
        : SoundSource(url, getModPlugTypeFromUrl(url)),
          m_pModFile(NULL),
          m_fileLength(0),
          m_seekPos(0) {
}

SoundSourceModPlug::~SoundSourceModPlug() {
    close();
}

Result SoundSourceModPlug::parseTrackMetadata(
        TrackMetadata* pMetadata) const {
    QFile modFile(getLocalFileNameBytes());
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
                &m_sampleBuf[currentSize],
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

Result SoundSourceModPlug::tryOpen(SINT /*channelCountHint*/) {
    ScopedTimer t("SoundSourceModPlug::open()");

    // read module file to byte array
    const QString fileName(getLocalFileName());
    QFile modFile(fileName);
    qDebug() << "[ModPlug] Loading ModPlug module " << modFile.fileName();
    modFile.open(QIODevice::ReadOnly);
    m_fileBuf = modFile.readAll();
    modFile.close();
    // get ModPlugFile descriptor for later access
    m_pModFile = ModPlug::ModPlug_Load(m_fileBuf.constData(),
            m_fileBuf.length());

    if (m_pModFile == NULL) {
        // an error occured
        t.cancel();
        qDebug() << "[ModPlug] Could not load module file: " << fileName;
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
    unsigned int estimate = ((ModPlug::ModPlug_GetLength(m_pModFile) >> 8) / 11)
            << 18;
    estimate = math_min(estimate, s_bufferSizeLimit);
    m_sampleBuf.reserve(estimate);
    qDebug() << "[ModPlug] Reserved " << m_sampleBuf.capacity() << " #samples";

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

    setChannelCount(kChannelCount);
    setFrameRate(kFrameRate);
    setFrameCount(samples2frames(m_sampleBuf.size()));
    m_seekPos = 0;

    return OK;
}

void SoundSourceModPlug::close() {
    if (m_pModFile) {
        ModPlug::ModPlug_Unload(m_pModFile);
        m_pModFile = NULL;
    }
}

SINT SoundSourceModPlug::seekSampleFrame(
        SINT frameIndex) {
    return m_seekPos = frameIndex;
}

SINT SoundSourceModPlug::readSampleFrames(
        SINT numberOfFrames, CSAMPLE* sampleBuffer) {
    const SINT maxFrames = samples2frames(m_sampleBuf.size());
    const SINT readFrames = math_min(maxFrames - m_seekPos, numberOfFrames);

    const SINT readSamples = frames2samples(readFrames);
    const SINT readOffset = frames2samples(m_seekPos);
    for (SINT i = 0; i < readSamples; ++i) {
        sampleBuffer[i] = SAMPLE_clampSymmetric(m_sampleBuf[readOffset + i])
                / CSAMPLE(SAMPLE_MAX);
    }

    m_seekPos += readFrames;
    return readFrames;
}

} // namespace Mixxx
