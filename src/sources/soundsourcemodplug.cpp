#include "sources/soundsourcemodplug.h"

#include <QFile>

#include "audio/streaminfo.h"
#include "audio/types.h"
#include "track/trackmetadata.h"
#include "util/logger.h"
#include "util/sample.h"
#include "util/timer.h"

namespace mixxx {

namespace {

const Logger kLogger("SoundSourceModPlug");

const QStringList kSupportedFileTypes = {
        // ModPlug supports more formats but file name
        // extensions are not always present with modules.
        QStringLiteral("mod"),
        QStringLiteral("okt"),
        QStringLiteral("s3m"),
        QStringLiteral("stm"),
        QStringLiteral("xm"),
        QStringLiteral("it"),
};

/* read files in 512k chunks */
constexpr SINT kChunkSizeInBytes = SINT(1) << 19;

QString getModPlugTypeFromUrl(const QUrl& url) {
    const QString fileType = SoundSource::getTypeFromUrl(url);
    if (fileType == "mod") {
        return "Protracker";
    } else if (fileType == "med") {
        return "OctaMed"; // audio/x-mod
    } else if (fileType == "okt") {
        return "Oktalyzer";
    } else if (fileType == "s3m") {
        return "Scream Tracker 3";
    } else if (fileType == "stm") {
        return "Scream Tracker";
    } else if (fileType == "xm") {
        return "FastTracker2";
    } else if (fileType == "it") {
        return "Impulse Tracker";
    } else {
        return "Module";
    }
}

} // anonymous namespace

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

QStringList SoundSourceProviderModPlug::getSupportedFileTypes() const {
    return kSupportedFileTypes;
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
        QImage* pCoverArt,
        bool resetMissingTagMetadata) const {
    if (pTrackMetadata != nullptr) {
        QFile modFile(getLocalFileName());
        [[maybe_unused]] auto result = modFile.open(QIODevice::ReadOnly);
        DEBUG_ASSERT(result);
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
                        kChannelCount,
                        kSampleRate,
                },
                audio::Bitrate(8),
                Duration::fromMillis(ModPlug::ModPlug_GetLength(pModFile)),
        });
        const auto sourceSynchronizedAt = getFileSynchronizedAt(modFile);
        return std::make_pair(ImportResult::Succeeded, sourceSynchronizedAt);
    }

    // The modplug library currently does not support reading cover-art from
    // modplug files -- kain88 (Oct 2014)
    return MetadataSourceTagLib::importTrackMetadataAndCoverImage(
            nullptr, pCoverArt, resetMissingTagMetadata);
}

SoundSource::OpenResult SoundSourceModPlug::tryOpen(
        OpenMode /*mode*/,
        const OpenParams& /*config*/) {
    ScopedTimer t(QStringLiteral("SoundSourceModPlug::open()"));

    // read module file to byte array
    const QString fileName(getLocalFileName());
    QFile modFile(fileName);
    kLogger.debug() << "Loading ModPlug module " << modFile.fileName();
    [[maybe_unused]] auto result = modFile.open(QIODevice::ReadOnly);
    DEBUG_ASSERT(result);
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
                    getSignalInfo().samples2frames(static_cast<SINT>(m_sampleBuf.size()))));

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
