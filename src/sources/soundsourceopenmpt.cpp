#include "sources/soundsourceopenmpt.h"

#include <libopenmpt/libopenmpt.hpp>

#include <QFile>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <sstream>
#include <string>
#include <vector>

#include "audio/streaminfo.h"
#include "audio/types.h"
#include "track/trackmetadata.h"
#include "util/logger.h"
#include "util/sample.h"
#include "util/timer.h"

namespace mixxx {

namespace {

const Logger kLogger("SoundSourceOpenMPT");

// Upper bound for the rendered length of a single module. Modules longer than
// this are almost certainly pathological (e.g. endless-loop artifacts) and are
// truncated to keep the in-memory PCM buffer bounded.
constexpr SINT kMaxDurationSeconds = 3600; // 1 hour

// Number of frames rendered per libopenmpt call while decoding.
constexpr std::size_t kRenderChunkFrames = 1024;

QString readModuleMetadata(const openmpt::module& module, const char* key) {
    return QString::fromStdString(module.get_metadata(key)).trimmed();
}

} // anonymous namespace

//static
std::mutex SoundSourceOpenMPT::s_settingsMutex;

//static
SoundSourceOpenMPT::Settings SoundSourceOpenMPT::s_settings;

//static
void SoundSourceOpenMPT::configure(const Settings& settings) {
    const std::scoped_lock lock(s_settingsMutex);
    s_settings = settings;
}

//static
SoundSourceOpenMPT::Settings SoundSourceOpenMPT::currentSettings() {
    const std::scoped_lock lock(s_settingsMutex);
    return s_settings;
}

//static
const QString SoundSourceProviderOpenMPT::kDisplayName = QStringLiteral("OpenMPT");

QStringList SoundSourceProviderOpenMPT::getSupportedFileTypes() const {
    // Register every module format the linked libopenmpt build supports. This
    // is self-maintaining across library versions and far exceeds the handful
    // of types the old libmodplug integration exposed.
    QStringList types;
    const std::vector<std::string> extensions = openmpt::get_supported_extensions();
    types.reserve(static_cast<int>(extensions.size()));
    for (const std::string& extension : extensions) {
        types.append(QString::fromStdString(extension));
    }
    return types;
}

SoundSourceProviderPriority SoundSourceProviderOpenMPT::getPriorityHint(
        const QString& supportedFileType) const {
    Q_UNUSED(supportedFileType)
    // libopenmpt is the dedicated, accurate decoder for tracker modules and
    // should win over any generic fallback (e.g. FFmpeg) for these file types.
    return SoundSourceProviderPriority::Higher;
}

SoundSourceOpenMPT::SoundSourceOpenMPT(const QUrl& url)
        : SoundSource(url) {
}

SoundSourceOpenMPT::~SoundSourceOpenMPT() {
    close();
}

std::pair<MetadataSource::ImportResult, QDateTime>
SoundSourceOpenMPT::importTrackMetadataAndCoverImage(
        TrackMetadata* pTrackMetadata,
        QImage* pCoverArt,
        bool resetMissingTagMetadata) const {
    if (pTrackMetadata == nullptr) {
        // Modules carry no embedded cover art. Defer to TagLib (which will
        // generally find nothing) so behavior matches the other sources.
        return MetadataSourceTagLib::importTrackMetadataAndCoverImage(
                nullptr, pCoverArt, resetMissingTagMetadata);
    }

    QFile moduleFile(getLocalFileName());
    if (!moduleFile.open(QIODevice::ReadOnly)) {
        return std::make_pair(ImportResult::Failed, QDateTime());
    }
    const QByteArray fileBuf = moduleFile.readAll();
    const auto sourceSynchronizedAt = getFileSynchronizedAt(moduleFile);
    moduleFile.close();
    if (fileBuf.isEmpty()) {
        return std::make_pair(ImportResult::Failed, QDateTime());
    }

    std::ostringstream moduleLog;
    double durationSeconds = 0.0;
    try {
        openmpt::module module(
                fileBuf.constData(),
                static_cast<std::size_t>(fileBuf.size()),
                moduleLog);
        durationSeconds = module.get_duration_seconds();

        const QString title = readModuleMetadata(module, "title");
        const QString artist = readModuleMetadata(module, "artist");
        const QString message = readModuleMetadata(module, "message");
        if (!title.isEmpty()) {
            pTrackMetadata->refTrackInfo().setTitle(title);
        }
        if (!artist.isEmpty()) {
            pTrackMetadata->refTrackInfo().setArtist(artist);
        }
        if (!message.isEmpty()) {
            pTrackMetadata->refTrackInfo().setComment(message);
        }
    } catch (const openmpt::exception& e) {
        kLogger.warning() << "libopenmpt could not read metadata from"
                          << getLocalFileName() << ":" << e.what();
        return std::make_pair(ImportResult::Failed, QDateTime());
    }

    // Module files are tiny, so a fixed/dummy bitrate would be meaningless.
    // Report the honest average data rate (file size over play length) instead.
    audio::Bitrate bitrate;
    if (durationSeconds > 0.0) {
        const double kbps =
                static_cast<double>(fileBuf.size()) * 8.0 / durationSeconds / 1000.0;
        bitrate = audio::Bitrate(
                static_cast<audio::Bitrate::value_t>(std::max(1.0, std::round(kbps))));
    }

    pTrackMetadata->setStreamInfo(audio::StreamInfo{
            audio::SignalInfo{
                    kChannelCount,
                    kSampleRate,
            },
            bitrate,
            Duration::fromMillis(
                    static_cast<qint64>(std::round(durationSeconds * 1000.0))),
    });
    return std::make_pair(ImportResult::Succeeded, sourceSynchronizedAt);
}

SoundSource::OpenResult SoundSourceOpenMPT::tryOpen(
        OpenMode /*mode*/,
        const OpenParams& /*params*/) {
    ScopedTimer t(QStringLiteral("SoundSourceOpenMPT::open()"));

    const QString fileName = getLocalFileName();
    QFile moduleFile(fileName);
    if (!moduleFile.open(QIODevice::ReadOnly)) {
        kLogger.warning() << "Failed to open module file:" << fileName;
        return OpenResult::Failed;
    }
    const QByteArray fileBuf = moduleFile.readAll();
    moduleFile.close();
    if (fileBuf.isEmpty()) {
        kLogger.warning() << "Module file is empty:" << fileName;
        return OpenResult::Failed;
    }

    const Settings settings = currentSettings();
    const std::size_t maxFrames =
            static_cast<std::size_t>(kMaxDurationSeconds) * kSampleRate;

    std::ostringstream moduleLog;
    bool truncated = false;
    try {
        openmpt::module module(
                fileBuf.constData(),
                static_cast<std::size_t>(fileBuf.size()),
                moduleLog);

        // Render the song exactly once; without looping the decode terminates
        // at the end of the song (read() returns 0).
        module.set_repeat_count(0);
        module.set_render_param(
                openmpt::module::RENDER_INTERPOLATIONFILTER_LENGTH,
                settings.interpolationFilterLength);
        module.set_render_param(
                openmpt::module::RENDER_STEREOSEPARATION_PERCENT,
                settings.stereoSeparationPercent);

        // get_duration_seconds() returns the exact non-looping length; use it
        // to size the buffer. The decode still stops on the authoritative
        // read()==0 end-of-song signal, not on this estimate.
        const double durationSeconds = module.get_duration_seconds();
        const std::size_t estimateFrames = (durationSeconds > 0.0)
                ? static_cast<std::size_t>(durationSeconds * kSampleRate)
                : 0;
        m_sampleBuf.reserve(
                std::min(estimateFrames, maxFrames) * kChannelCount);

        std::vector<CSAMPLE> chunk(kRenderChunkFrames * kChannelCount);
        while (m_sampleBuf.size() / kChannelCount < maxFrames) {
            const std::size_t framesRead = module.read_interleaved_stereo(
                    static_cast<std::int32_t>(kSampleRate.value()),
                    kRenderChunkFrames,
                    chunk.data());
            if (framesRead == 0) {
                break; // end of song
            }
            m_sampleBuf.insert(
                    m_sampleBuf.end(),
                    chunk.begin(),
                    chunk.begin() + framesRead * kChannelCount);
        }
        truncated = m_sampleBuf.size() / kChannelCount >= maxFrames;
        // All PCM is now buffered; the openmpt::module (and its decoder state)
        // is released here as it goes out of scope.
    } catch (const openmpt::exception& e) {
        // The file carries a module extension but is not actually a module
        // libopenmpt can read. Abort so lower-priority decoders may still try.
        kLogger.warning() << "libopenmpt could not load" << fileName << ":" << e.what();
        return OpenResult::Aborted;
    }

    const QString moduleLogStr = QString::fromStdString(moduleLog.str()).trimmed();
    if (!moduleLogStr.isEmpty()) {
        kLogger.debug() << "libopenmpt log for" << fileName << ":" << moduleLogStr;
    }
    if (truncated) {
        kLogger.warning() << "Module decode hit the" << kMaxDurationSeconds
                          << "second safety limit and was truncated:" << fileName;
    }
    if (m_sampleBuf.empty()) {
        kLogger.warning() << "Module decoded to an empty stream:" << fileName;
        return OpenResult::Failed;
    }

    initChannelCountOnce(kChannelCount);
    initSampleRateOnce(kSampleRate);
    initFrameIndexRangeOnce(
            IndexRange::forward(
                    0,
                    getSignalInfo().samples2frames(
                            static_cast<SINT>(m_sampleBuf.size()))));
    return OpenResult::Succeeded;
}

void SoundSourceOpenMPT::close() {
    m_sampleBuf.clear();
    m_sampleBuf.shrink_to_fit();
}

ReadableSampleFrames SoundSourceOpenMPT::readSampleFramesClamped(
        const WritableSampleFrames& writableSampleFrames) {
    const SINT readOffset = getSignalInfo().frames2samples(
            writableSampleFrames.frameIndexRange().start());
    const SINT readSamples = getSignalInfo().frames2samples(
            writableSampleFrames.frameLength());
    SampleUtil::copy(
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
