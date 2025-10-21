#include "sources/soundsourceopenmpt.h"

#include <QFile>
#include <libopenmpt/libopenmpt.hpp>

#include "audio/streaminfo.h"
#include "audio/types.h"
#include "track/trackmetadata.h"
#include "util/logger.h"
#include "util/sample.h"
#include "util/timer.h"

namespace mixxx {

namespace {

const Logger kLogger("SoundSourceOpenMPT");

const QStringList kSupportedFileTypes = {
        // classic formats
        QStringLiteral("mod"),
        QStringLiteral("s3m"),
        QStringLiteral("xm"),
        QStringLiteral("it"),
        // additional formats supported by openmpt
        QStringLiteral("mptm"),
        QStringLiteral("okt"),
        QStringLiteral("stm"),
        QStringLiteral("669"),
        QStringLiteral("amf"),
        QStringLiteral("ams"),
        QStringLiteral("dbm"),
        QStringLiteral("dmf"),
        QStringLiteral("dsm"),
        QStringLiteral("far"),
        QStringLiteral("mdl"),
        QStringLiteral("med"),
        QStringLiteral("mtm"),
        QStringLiteral("mt2"),
        QStringLiteral("psm"),
        QStringLiteral("ptm"),
        QStringLiteral("ult"),
        QStringLiteral("umx"),
};

/* read files in 512k chunks */
constexpr SINT kChunkSizeInBytes = SINT(1) << 19;

QString getModuleTypeFromUrl(const QUrl& url) {
    const QString fileType = SoundSource::getTypeFromUrl(url);
    if (fileType == "mod") {
        return "ProTracker";
    } else if (fileType == "s3m") {
        return "Scream Tracker 3";
    } else if (fileType == "xm") {
        return "FastTracker 2";
    } else if (fileType == "it") {
        return "Impulse Tracker";
    } else if (fileType == "mptm") {
        return "OpenMPT";
    } else if (fileType == "okt") {
        return "Oktalyzer";
    } else if (fileType == "stm") {
        return "Scream Tracker 2";
    } else if (fileType == "669") {
        return "Composer 669";
    } else if (fileType == "amf") {
        return "ASYLUM Music Format";
    } else if (fileType == "ams") {
        return "Velvet Studio";
    } else if (fileType == "dbm") {
        return "DigiBooster Pro";
    } else if (fileType == "dmf") {
        return "X-Tracker";
    } else if (fileType == "dsm") {
        return "DSIK";
    } else if (fileType == "far") {
        return "Farandole Composer";
    } else if (fileType == "mdl") {
        return "DigiTrakker";
    } else if (fileType == "med") {
        return "OctaMED";
    } else if (fileType == "mtm") {
        return "MultiTracker";
    } else if (fileType == "mt2") {
        return "MadTracker 2";
    } else if (fileType == "psm") {
        return "Epic MegaGames MASI";
    } else if (fileType == "ptm") {
        return "PolyTracker";
    } else if (fileType == "ult") {
        return "UltraTracker";
    } else if (fileType == "umx") {
        return "Unreal Music";
    } else {
        return "Module";
    }
}

} // anonymous namespace

unsigned int SoundSourceOpenMPT::s_bufferSizeLimit = 0;
TrackerDSP::Settings SoundSourceOpenMPT::s_dspSettings;

void SoundSourceOpenMPT::configure(
        unsigned int bufferSizeLimit,
        const TrackerDSP::Settings& dspSettings) {
    s_bufferSizeLimit = bufferSizeLimit;
    s_dspSettings = dspSettings;
}

const QString SoundSourceProviderOpenMPT::kDisplayName = QStringLiteral("OpenMPT");

QStringList SoundSourceProviderOpenMPT::getSupportedFileTypes() const {
    return kSupportedFileTypes;
}

SoundSourceOpenMPT::SoundSourceOpenMPT(const QUrl& url)
        : SoundSource(url, getModuleTypeFromUrl(url)),
          m_pModule(nullptr) {
}

SoundSourceOpenMPT::~SoundSourceOpenMPT() {
    close();
}

std::pair<MetadataSource::ImportResult, QDateTime>
SoundSourceOpenMPT::importTrackMetadataAndCoverImage(
        TrackMetadata* pTrackMetadata,
        QImage* pCoverArt,
        bool resetMissingTagMetadata) const {
    if (pTrackMetadata != nullptr) {
        QFile modFile(getLocalFileName());
        modFile.open(QIODevice::ReadOnly);
        const QByteArray fileBuf(modFile.readAll());
        modFile.close();

        try {
            std::unique_ptr<openmpt::module> pModule =
                    std::make_unique<openmpt::module>(
                            fileBuf.constData(),
                            fileBuf.size());

            // get metadata
            QString title = QString::fromStdString(pModule->get_metadata("title"));
            QString message = QString::fromStdString(pModule->get_metadata("message"));
            QString artist = QString::fromStdString(pModule->get_metadata("artist"));

            if (!title.isEmpty()) {
                pTrackMetadata->refTrackInfo().setTitle(title);
            }
            if (!message.isEmpty()) {
                pTrackMetadata->refTrackInfo().setComment(message);
            }
            if (!artist.isEmpty()) {
                pTrackMetadata->refTrackInfo().setArtist(artist);
            }

            // get duration
            double durationSeconds = pModule->get_duration_seconds();

            pTrackMetadata->setStreamInfo(audio::StreamInfo{
                    audio::SignalInfo{
                            kChannelCount,
                            kSampleRate,
                    },
                    audio::Bitrate(8),
                    Duration::fromMillis(static_cast<qint64>(durationSeconds * 1000)),
            });

            const auto sourceSynchronizedAt = getFileSynchronizedAt(modFile);
            return std::make_pair(ImportResult::Succeeded, sourceSynchronizedAt);
        } catch (const openmpt::exception& e) {
            kLogger.warning() << "failed to load module for metadata:" << e.what();
            return std::make_pair(ImportResult::Failed, QDateTime());
        }
    }

    // openmpt doesn't support reading cover art from module files
    return MetadataSourceTagLib::importTrackMetadataAndCoverImage(
            nullptr, pCoverArt, resetMissingTagMetadata);
}

SoundSource::OpenResult SoundSourceOpenMPT::tryOpen(
        OpenMode /*mode*/,
        const OpenParams& /*config*/) {
    ScopedTimer t(QStringLiteral("SoundSourceOpenMPT::open()"));

    // read module file to byte array
    const QString fileName(getLocalFileName());
    QFile modFile(fileName);
    kLogger.debug() << "loading openmpt module" << modFile.fileName();
    modFile.open(QIODevice::ReadOnly);
    m_fileBuf = modFile.readAll();
    modFile.close();

    // create openmpt module
    try {
        m_pModule = std::make_unique<openmpt::module>(
                m_fileBuf.constData(),
                m_fileBuf.size());
    } catch (const openmpt::exception& e) {
        kLogger.debug() << "could not load module file:" << fileName << "-" << e.what();
        return OpenResult::Failed;
    }

    // configure DSP
    m_dsp.configure(s_dspSettings, kSampleRate);

    DEBUG_ASSERT(0 == (kChunkSizeInBytes % sizeof(m_sampleBuf[0])));
    const SINT chunkSizeInSamples = kChunkSizeInBytes / sizeof(m_sampleBuf[0]);

    const ModSampleBuffer::size_type bufferSizeLimitInSamples =
            s_bufferSizeLimit / sizeof(m_sampleBuf[0]);

    // estimate size of sample buffer aligned with chunk size
    const ModSampleBuffer::size_type estimateSeconds =
            static_cast<ModSampleBuffer::size_type>(m_pModule->get_duration_seconds());
    const ModSampleBuffer::size_type estimateSamples =
            estimateSeconds * kChannelCount * kSampleRate;
    const ModSampleBuffer::size_type estimateChunks =
            (estimateSamples + (chunkSizeInSamples - 1)) / chunkSizeInSamples;
    const ModSampleBuffer::size_type sampleBufferCapacity = math_min(
            estimateChunks * chunkSizeInSamples, bufferSizeLimitInSamples);
    m_sampleBuf.reserve(sampleBufferCapacity);
    kLogger.debug() << "reserved" << m_sampleBuf.capacity() << "#samples";

    // decode samples into sample buffer
    std::vector<float> tempBuffer(chunkSizeInSamples);
    while (m_sampleBuf.size() < bufferSizeLimitInSamples) {
        const ModSampleBuffer::size_type currentSize = m_sampleBuf.size();

        // read interleaved stereo from openmpt
        const size_t framesRead = m_pModule->read_interleaved_stereo(
                kSampleRate,
                chunkSizeInSamples / kChannelCount,
                tempBuffer.data());

        if (framesRead > 0) {
            const size_t samplesRead = framesRead * kChannelCount;
            m_sampleBuf.resize(currentSize + samplesRead);
            std::copy(tempBuffer.begin(),
                    tempBuffer.begin() + samplesRead,
                    m_sampleBuf.begin() + currentSize);
        } else {
            // eof
            break;
        }
    }

    kLogger.debug() << "filled sample buffer with" << m_sampleBuf.size() << "samples";
    kLogger.debug() << "sample buffer has"
                    << m_sampleBuf.capacity() - m_sampleBuf.size()
                    << "samples unused capacity";

    // apply DSP effects to entire buffer
    if (s_dspSettings.reverbEnabled ||
            s_dspSettings.megabassEnabled ||
            s_dspSettings.surroundEnabled ||
            s_dspSettings.noiseReductionEnabled) {
        kLogger.debug() << "applying DSP effects to decoded buffer";
        const SINT frameCount = m_sampleBuf.size() / kChannelCount;
        m_dsp.processStereo(m_sampleBuf.data(), frameCount);
    }

    initChannelCountOnce(kChannelCount);
    initSampleRateOnce(kSampleRate);
    initFrameIndexRangeOnce(
            IndexRange::forward(
                    0,
                    getSignalInfo().samples2frames(static_cast<SINT>(m_sampleBuf.size()))));

    return OpenResult::Succeeded;
}

void SoundSourceOpenMPT::close() {
    m_pModule.reset();
}

ReadableSampleFrames SoundSourceOpenMPT::readSampleFramesClamped(
        const WritableSampleFrames& writableSampleFrames) {
    const SINT readOffset = getSignalInfo().frames2samples(
            writableSampleFrames.frameIndexRange().start());
    const SINT readSamples = getSignalInfo().frames2samples(
            writableSampleFrames.frameLength());

    // copy from our pre-decoded buffer
    std::copy(m_sampleBuf.begin() + readOffset,
            m_sampleBuf.begin() + readOffset + readSamples,
            writableSampleFrames.writableData());

    return ReadableSampleFrames(
            writableSampleFrames.frameIndexRange(),
            SampleBuffer::ReadableSlice(
                    writableSampleFrames.writableData(),
                    writableSampleFrames.writableLength()));
}

} // namespace mixxx
