#include "sources/youtube/soundsourceyoutube.h"

#include <QProcess>

#include "library/youtube/youtubeservice.h"
#include "util/logger.h"

namespace mixxx {

namespace {
const Logger kLogger("SoundSourceYouTube");
} // namespace

const QString SoundSourceProviderYouTube::kDisplayName = "YouTube";

SoundSourceYouTube::SoundSourceYouTube(const QUrl& url)
        : SoundSource(url),
          m_segmentsLoaded(false) {
}

SoundSourceYouTube::~SoundSourceYouTube() {
    close();
}

void SoundSourceYouTube::close() {
}

ReadableSampleFrames SoundSourceYouTube::readSampleFramesClamped(
        const WritableSampleFrames& sampleFrames) {
    return ReadableSampleFrames(sampleFrames.frameIndexRange());
}

SoundSource::OpenResult SoundSourceYouTube::tryOpen(
        OpenMode mode,
        const OpenParams& params) {
    Q_UNUSED(mode);
    Q_UNUSED(params);
    kLogger.info() << "Opening YouTube stream:" << getUrl().toString();

    // Fetch SponsorBlock segments. The service is parented to `this` so it
    // shares our lifetime; passing the SoundSource as the connect context
    // is what clazy wants here (and is what we logically need).
    QString videoId = getUrl().toString().split("v=").last();
    YouTubeService* service = new YouTubeService();
    QObject::connect(service,
            &YouTubeService::sponsorSegmentsFetched,
            service,
            [this, service](const QString& videoId,
                    const QList<::mixxx::SponsorSegment>& segments) {
                this->onSponsorSegmentsFetched(videoId, segments);
                service->deleteLater();
            });
    service->fetchSponsorSegments(videoId);

    return OpenResult::Succeeded;
}

void SoundSourceYouTube::onSponsorSegmentsFetched(const QString& videoId, const QList<::mixxx::SponsorSegment>& segments) {
    Q_UNUSED(videoId);
    m_sponsorSegments = segments;
    m_segmentsLoaded = true;
    kLogger.info() << "Loaded" << segments.size() << "SponsorBlock segments";
}

} // namespace mixxx
