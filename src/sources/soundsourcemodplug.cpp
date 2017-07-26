#include "sources/soundsourcemodplug.h"

#include "track/trackmetadata.h"
#include "util/timer.h"
#include "util/sample.h"
#include "util/logger.h"

#include <QFile>

#include <stdlib.h>
#include <unistd.h>

namespace mixxx {

namespace {

const Logger kLogger("SoundSourceModPlug");

/* read files in 512k chunks */
const SINT kChunkSizeInBytes = SINT(1) << 19;

QString getModPlugTypeFromUrl(QUrl url) {
    const QString fileExtension(SoundSource::getFileExtensionFromUrl(url));
    if (fileExtension == "mod") {
        return "Protracker";
    } else if (fileExtension == "med") {
        return "OctaMed";
    } else if (fileExtension == "okt") {
        return "Oktalyzer";
    } else if (fileExtension == "s3m") {
        return "Scream Tracker 3";
    } else if (fileExtension == "stm") {
        return "Scream Tracker";
    } else if (fileExtension == "xm") {
        return "FastTracker2";
    } else if (fileExtension == "it") {
        return "Impulse Tracker";
    } else {
        return "Module";
    }
}

} // anonymous namespace

/*static*/ constexpr SINT SoundSourceModPlug::kChannelCount;
/*static*/ constexpr SINT SoundSourceModPlug::kBitsPerSample;
/*static*/ constexpr SINT SoundSourceModPlug::kSamplingRate;

unsigned int SoundSourceModPlug::s_bufferSizeLimit = 0;

// reserve some static space for settings...
void SoundSourceModPlug::configure(unsigned int bufferSizeLimit,
        const ModPlug::ModPlug_Settings &settings) {
    s_bufferSizeLimit = bufferSizeLimit;
    ModPlug::ModPlug_SetSettings(&settings);
}

SoundSourceModPlug::SoundSourceModPlug(const QUrl& url)
        : SoundSource(url, getModPlugTypeFromUrl(url)),
          m_pModFile(nullptr),
          m_seekPos(0) {
}

SoundSourceModPlug::~SoundSourceModPlug() {
    close();
}

Result SoundSourceModPlug::parseTrackMetadataAndCoverArt(
        TrackMetadata* pTrackMetadata,
        QImage* pCoverArt) const {
    if (pTrackMetadata != nullptr) {
        QFile modFile(getLocalFileName());
        modFile.open(QIODevice::ReadOnly);
        const QByteArray fileBuf(modFile.readAll());
        modFile.close();

        ModPlug::ModPlugFile* pModFile = ModPlug::ModPlug_Load(fileBuf.constData(),
                fileBuf.length());
        if (pModFile == nullptr) {
            return ERR;
        }

        pTrackMetadata->setComment(QString(ModPlug::ModPlug_GetMessage(pModFile)));
        pTrackMetadata->setTitle(QString(ModPlug::ModPlug_GetName(pModFile)));
        pTrackMetadata->setDuration(ModPlug::ModPlug_GetLength(pModFile) / 1000.0);
        pTrackMetadata->setBitrate(8); // not really, but fill in something...
        ModPlug::ModPlug_Unload(pModFile);
    }

    // The modplug library currently does not support reading cover-art from
    // modplug files -- kain88 (Oct 2014)
    // NOTE(uklotzde, 2016-11-02): Leave the QImage passed by the caller untouched
    // if pCoverArt != nullptr.
    Q_UNUSED(pCoverArt);

    return OK;
}

SoundSource::OpenResult SoundSourceModPlug::tryOpen(const AudioSourceConfig& /*audioSrcCfg*/) {
    ScopedTimer t("SoundSourceModPlug::open()");

    // read module file to byte array
    const QString fileName(getLocalFileName());
    QFile modFile(fileName);
    kLogger.debug() << "Loading ModPlug module " << modFile.fileName();
    modFile.open(QIODevice::ReadOnly);
    m_fileBuf = modFile.readAll();
    modFile.close();

    // get ModPlugFile descriptor for later access
    m_pModFile = ModPlug::ModPlug_Load(m_fileBuf.constData(),
            m_fileBuf.length());
    if (m_pModFile == nullptr) {
        // an error occurred
        t.cancel();
        kLogger.debug() << "Could not load module file: " << fileName;
        return OpenResult::FAILED;
    }

    DEBUG_ASSERT(0 == (kChunkSizeInBytes % sizeof(m_sampleBuf[0])));
    const SINT chunkSizeInSamples = kChunkSizeInBytes / sizeof(m_sampleBuf[0]);

    const SampleBuffer::size_type bufferSizeLimitInSamples = s_bufferSizeLimit / sizeof(m_sampleBuf[0]);

    // Estimate size of sample buffer (for better performance) aligned
    // with the chunk size. Beware: Module length estimation is unreliable
    // due to loops!
    const SampleBuffer::size_type estimateMilliseconds =
            ModPlug::ModPlug_GetLength(m_pModFile);
    const SampleBuffer::size_type estimateSamples =
            estimateMilliseconds * kChannelCount * kSamplingRate;
    const SampleBuffer::size_type estimateChunks =
            (estimateSamples + (chunkSizeInSamples - 1)) / chunkSizeInSamples;
    const SampleBuffer::size_type sampleBufferCapacity = math_min(
            estimateChunks * chunkSizeInSamples, bufferSizeLimitInSamples);
    m_sampleBuf.reserve(sampleBufferCapacity);
    kLogger.debug() << "Reserved " << m_sampleBuf.capacity() << " #samples";

    // decode samples into sample buffer
    while (m_sampleBuf.size() < bufferSizeLimitInSamples) {
        // reserve enough space in sample buffer
        const SampleBuffer::size_type currentSize = m_sampleBuf.size();
        m_sampleBuf.resize(currentSize + chunkSizeInSamples);
        const int bytesRead = ModPlug::ModPlug_Read(m_pModFile,
                &m_sampleBuf[currentSize],
                kChunkSizeInBytes);
        // adjust size of sample buffer after reading
        if (0 < bytesRead) {
            DEBUG_ASSERT(0 == (bytesRead % sizeof(m_sampleBuf[0])));
            const SampleBuffer::size_type samplesRead = bytesRead / sizeof(m_sampleBuf[0]);
            m_sampleBuf.resize(currentSize + samplesRead);
        } else {
            // nothing read -> EOF
            m_sampleBuf.resize(currentSize);
            break; // exit loop
        }
    }
    kLogger.debug() << "Filled Sample buffer with " << m_sampleBuf.size()
            << " samples.";
    kLogger.debug() << "Sample buffer has "
            << m_sampleBuf.capacity() - m_sampleBuf.size()
            << " samples unused capacity.";

    setChannelCount(kChannelCount);
    setSamplingRate(kSamplingRate);
    setFrameCount(samples2frames(m_sampleBuf.size()));
    m_seekPos = 0;

    return OpenResult::SUCCEEDED;
}

void SoundSourceModPlug::close() {
    if (m_pModFile) {
        ModPlug::ModPlug_Unload(m_pModFile);
        m_pModFile = nullptr;
    }
}

SINT SoundSourceModPlug::seekSampleFrame(
        SINT frameIndex) {
    DEBUG_ASSERT(isValidFrameIndex(frameIndex));

    return m_seekPos = frameIndex;
}

SINT SoundSourceModPlug::readSampleFrames(
        SINT numberOfFrames, CSAMPLE* sampleBuffer) {
    DEBUG_ASSERT(0 <= numberOfFrames);
    DEBUG_ASSERT(isValidFrameIndex(m_seekPos));
    const SINT readFrames = math_min(getFrameCount() - m_seekPos, numberOfFrames);

    const SINT readSamples = frames2samples(readFrames);
    const SINT readOffset = frames2samples(m_seekPos);
    SampleUtil::convertS16ToFloat32(sampleBuffer, &m_sampleBuf[readOffset], readSamples);
    m_seekPos += readFrames;

    return readFrames;
}

QString SoundSourceProviderModPlug::getName() const {
    return "MODPlug";
}

QStringList SoundSourceProviderModPlug::getSupportedFileExtensions() const {
    QStringList supportedFileExtensions;
    // ModPlug supports more formats but file name
    // extensions are not always present with modules.
    supportedFileExtensions.append("mod");
    supportedFileExtensions.append("med");
    supportedFileExtensions.append("okt");
    supportedFileExtensions.append("s3m");
    supportedFileExtensions.append("stm");
    supportedFileExtensions.append("xm");
    supportedFileExtensions.append("it");
    return supportedFileExtensions;
}

} // namespace mixxx
