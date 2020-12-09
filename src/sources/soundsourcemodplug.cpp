#include "sources/soundsourcemodplug.h"

#include "audio/streaminfo.h"
#include "track/trackmetadata.h"
#include "util/logger.h"
#include "util/sample.h"
#include "util/timer.h"

#include <QFile>

#include <stdlib.h>
#include <unistd.h>

namespace mixxx {

namespace {

const Logger kLogger("SoundSourceModPlug");

const QStringList kSupportedFileExtensions = {
        // ModPlug supports more formats but file name
        // extensions are not always present with modules.
        QStringLiteral("mod"),
        QStringLiteral("med"),
        QStringLiteral("okt"),
        QStringLiteral("s3m"),
        QStringLiteral("stm"),
        QStringLiteral("xm"),
        QStringLiteral("it"),
};

/* read files in 512k chunks */
constexpr SINT kChunkSizeInBytes = SINT(1) << 19;

QString getModPlugTypeFromUrl(const QUrl& url) {
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

//static
constexpr SINT SoundSourceModPlug::kChannelCount;

//static
constexpr SINT SoundSourceModPlug::kBitsPerSample;

//static
constexpr SINT SoundSourceModPlug::kSampleRate;

//static
unsigned int SoundSourceModPlug::s_bufferSizeLimit = 0;

//static
void SoundSourceModPlug::configure(unsigned int bufferSizeLimit,
        const ModPlug::ModPlug_Settings& settings) {
    s_bufferSizeLimit = bufferSizeLimit;
    ModPlug::ModPlug_SetSettings(&settings);
}

//static
const QString SoundSourceProviderModPlug::kDisplayName = QStringLiteral("MODPlug");

QStringList SoundSourceProviderModPlug::getSupportedFileExtensions() const {
    return kSupportedFileExtensions;
}

SoundSourceModPlug::SoundSourceModPlug(const QUrl& url)
        : SoundSource(url, getModPlugTypeFromUrl(url)),
          m_pModFile(nullptr) {
}

SoundSourceModPlug::~SoundSourceModPlug() {
    close();
}

std::pair<MetadataSource::ImportResult, QDateTime>
SoundSourceModPlug::importTrackMetadataAndCoverImage(
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
            return std::make_pair(ImportResult::Failed, QDateTime());
        }

        pTrackMetadata->refTrackInfo().setComment(QString(ModPlug::ModPlug_GetMessage(pModFile)));
        pTrackMetadata->refTrackInfo().setTitle(QString(ModPlug::ModPlug_GetName(pModFile)));
        pTrackMetadata->setStreamInfo(audio::StreamInfo{
                audio::SignalInfo{
                        audio::ChannelCount(kChannelCount),
                        audio::SampleRate(kSampleRate),
                },
                audio::Bitrate(8),
                Duration::fromMillis(ModPlug::ModPlug_GetLength(pModFile)),
        });

        return std::make_pair(ImportResult::Succeeded, QFileInfo(modFile).lastModified());
    }

    // The modplug library currently does not support reading cover-art from
    // modplug files -- kain88 (Oct 2014)
    return MetadataSourceTagLib::importTrackMetadataAndCoverImage(nullptr, pCoverArt);
}

SoundSource::OpenResult SoundSourceModPlug::tryOpen(
        OpenMode /*mode*/,
        const OpenParams& /*config*/) {
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
        return OpenResult::Failed;
    }

    DEBUG_ASSERT(0 == (kChunkSizeInBytes % sizeof(m_sampleBuf[0])));
    const SINT chunkSizeInSamples = kChunkSizeInBytes / sizeof(m_sampleBuf[0]);

    const ModSampleBuffer::size_type bufferSizeLimitInSamples = s_bufferSizeLimit / sizeof(m_sampleBuf[0]);

    // Estimate size of sample buffer (for better performance) aligned
    // with the chunk size. Beware: Module length estimation is unreliable
    // due to loops!
    const ModSampleBuffer::size_type estimateMilliseconds =
            ModPlug::ModPlug_GetLength(m_pModFile);
    const ModSampleBuffer::size_type estimateSamples =
            estimateMilliseconds * kChannelCount * kSampleRate;
    const ModSampleBuffer::size_type estimateChunks =
            (estimateSamples + (chunkSizeInSamples - 1)) / chunkSizeInSamples;
    const ModSampleBuffer::size_type sampleBufferCapacity = math_min(
            estimateChunks * chunkSizeInSamples, bufferSizeLimitInSamples);
    m_sampleBuf.reserve(sampleBufferCapacity);
    kLogger.debug() << "Reserved " << m_sampleBuf.capacity() << " #samples";

    // decode samples into sample buffer
    while (m_sampleBuf.size() < bufferSizeLimitInSamples) {
        // reserve enough space in sample buffer
        const ModSampleBuffer::size_type currentSize = m_sampleBuf.size();
        m_sampleBuf.resize(currentSize + chunkSizeInSamples);
        const int bytesRead = ModPlug::ModPlug_Read(m_pModFile,
                &m_sampleBuf[currentSize],
                kChunkSizeInBytes);
        // adjust size of sample buffer after reading
        if (0 < bytesRead) {
            DEBUG_ASSERT(0 == (bytesRead % sizeof(m_sampleBuf[0])));
            const ModSampleBuffer::size_type samplesRead = bytesRead / sizeof(m_sampleBuf[0]);
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

    initChannelCountOnce(kChannelCount);
    initSampleRateOnce(kSampleRate);
    initFrameIndexRangeOnce(
            IndexRange::forward(
                    0,
                    getSignalInfo().samples2frames(m_sampleBuf.size())));

    return OpenResult::Succeeded;
}

void SoundSourceModPlug::close() {
    if (m_pModFile) {
        ModPlug::ModPlug_Unload(m_pModFile);
        m_pModFile = nullptr;
    }
}

ReadableSampleFrames SoundSourceModPlug::readSampleFramesClamped(
        const WritableSampleFrames& writableSampleFrames) {
    const SINT readOffset = getSignalInfo().frames2samples(writableSampleFrames.frameIndexRange().start());
    const SINT readSamples = getSignalInfo().frames2samples(writableSampleFrames.frameLength());
    SampleUtil::convertS16ToFloat32(
            writableSampleFrames.writableData(),
            &m_sampleBuf[readOffset],
            readSamples);

    return ReadableSampleFrames(
            writableSampleFrames.frameIndexRange(),
            SampleBuffer::ReadableSlice(
                    writableSampleFrames.writableData(),
                    writableSampleFrames.writableLength()));
}

} // namespace mixxx
