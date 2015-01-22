#include "sources/audiosourcemodplug.h"

#include "util/timer.h"

#include <stdlib.h>
#include <unistd.h>

#include <QFile>
#include <QtDebug>

/* read files in 512k chunks */
#define CHUNKSIZE (1 << 18)

namespace Mixxx {

// reserve some static space for settings...
namespace {
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
}

unsigned int AudioSourceModPlug::s_bufferSizeLimit = 0;

void AudioSourceModPlug::configure(unsigned int bufferSizeLimit,
        const ModPlug::ModPlug_Settings &settings) {
    s_bufferSizeLimit = bufferSizeLimit;
    ModPlug::ModPlug_SetSettings(&settings);
}

AudioSourceModPlug::AudioSourceModPlug(QUrl url)
        : AudioSource(url),
          m_pModFile(NULL),
          m_fileLength(0),
          m_seekPos(0) {
}

AudioSourceModPlug::~AudioSourceModPlug() {
    preDestroy();
}

AudioSourcePointer AudioSourceModPlug::create(QUrl url) {
    return onCreate(new AudioSourceModPlug(url));
}

Result AudioSourceModPlug::postConstruct() {
    const QString fileName(getUrl().toLocalFile());

    ScopedTimer t("AudioSourceModPlug::open()");

    qDebug() << "[ModPlug] Loading ModPlug module " << fileName;

    // read module file to byte array
    QFile modFile(fileName);
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

void AudioSourceModPlug::preDestroy() {
    if (m_pModFile) {
        ModPlug::ModPlug_Unload(m_pModFile);
        m_pModFile = NULL;
    }
}

AudioSource::diff_type AudioSourceModPlug::seekSampleFrame(
        diff_type frameIndex) {
    return m_seekPos = frameIndex;
}

AudioSource::size_type AudioSourceModPlug::readSampleFrames(
        size_type numberOfFrames, sample_type* sampleBuffer) {
    const size_type maxFrames = samples2frames(m_sampleBuf.size());
    const size_type readFrames = math_min(maxFrames - m_seekPos, numberOfFrames);

    const size_type readSamples = frames2samples(readFrames);
    const size_type readOffset = frames2samples(m_seekPos);
    for (size_type i = 0; i < readSamples; ++i) {
        sampleBuffer[i] = SAMPLE_clampSymmetric(m_sampleBuf[readOffset + i])
                / sample_type(SAMPLE_MAX);
    }

    m_seekPos += readFrames;
    return readFrames;
}

} // namespace Mixxx
