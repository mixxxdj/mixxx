#include "visualizer/visualizerdeck.h"

#include <QPixmap>
#include <QtConcurrentRun>
#include <algorithm>
#include <cmath>
#include <numbers>

#include "library/coverart.h"
#include "library/coverartcache.h"
#include "library/coverartutils.h"
#include "mixer/basetrackplayer.h"
#include "moc_visualizerdeck.cpp"
#include "track/track.h"
#include "util/imageutils.h"

namespace mixxx {

namespace {

// [AI-generated] The band meters report the mean absolute sample value of a
// band. These gains scale the levels of typical music at full volume to
// about 1. The highs carry much less energy than the bass. [/AI-generated]
constexpr float kBassGain = 6.0f;
constexpr float kMidGain = 15.0f;
constexpr float kHighGain = 40.0f;

// [AI-generated] Levels below this are treated as silence. This hides what
// leaks into a band from its neighbors while the band is killed with the EQ.
// [/AI-generated]
constexpr float kLevelFloor = 0.08f;

constexpr double kLevelReleaseSeconds = 0.15;
constexpr double kLoudnessAttackSeconds = 0.1;
constexpr double kLoudnessReleaseSeconds = 1.5;

// [AI-generated] Larger covers are scaled down to keep the textures small.
// [/AI-generated]
constexpr int kMaxCoverSize = 1024;

// [AI-generated] The background flow repeats after this many beats.
// [/AI-generated]
constexpr double kBeatsPerFlowCycle = 8.0;
constexpr double kUnknownTempoBpm = 120.0;

float releasedLevel(float currentLevel, float newLevel, double elapsedSeconds) {
    const float decayed = currentLevel *
            static_cast<float>(std::exp(-elapsedSeconds / kLevelReleaseSeconds));
    return std::max(newLevel, decayed);
}

float scaledLevel(const PollingControlProxy& meter, float gain) {
    return std::min(static_cast<float>(meter.get()) * gain, 1.0f);
}

} // namespace

VisualizerDeck::VisualizerDeck(BaseTrackPlayer* pPlayer, QObject* pParent)
        : QObject(pParent),
          m_bassMeter(pPlayer->getGroup(), QStringLiteral("band_meter_low")),
          m_midMeter(pPlayer->getGroup(), QStringLiteral("band_meter_mid")),
          m_highMeter(pPlayer->getGroup(), QStringLiteral("band_meter_high")),
          m_bpm(pPlayer->getGroup(), QStringLiteral("bpm")),
          m_bassLevel(0.0f),
          m_midLevel(0.0f),
          m_highLevel(0.0f),
          m_loudness(0.0f),
          m_coverRevision(0),
          m_active(false),
          m_coverRequested(false),
          m_coverRequest(0) {
    connect(&m_preparedCoverWatcher,
            &QFutureWatcher<PreparedCover>::finished,
            this,
            &VisualizerDeck::slotCoverPrepared);
    CoverArtCache* pCache = CoverArtCache::instance();
    if (pCache) {
        connect(pCache,
                &CoverArtCache::coverFound,
                this,
                &VisualizerDeck::slotCoverFound);
    }
    connect(pPlayer,
            &BaseTrackPlayer::newTrackLoaded,
            this,
            &VisualizerDeck::slotLoadTrack);
    connect(pPlayer,
            &BaseTrackPlayer::loadingTrack,
            this,
            &VisualizerDeck::slotLoadingTrack);
    slotLoadTrack(pPlayer->getLoadedTrack());
}

void VisualizerDeck::update(double elapsedSeconds) {
    const float bassLevel = scaledLevel(m_bassMeter, kBassGain);
    const float midLevel = scaledLevel(m_midMeter, kMidGain);
    const float highLevel = scaledLevel(m_highMeter, kHighGain);
    m_bassLevel = releasedLevel(m_bassLevel, bassLevel, elapsedSeconds);
    m_midLevel = releasedLevel(m_midLevel, midLevel, elapsedSeconds);
    m_highLevel = releasedLevel(m_highLevel, highLevel, elapsedSeconds);

    const float loudness = static_cast<float>(
            m_bassMeter.get() + m_midMeter.get() + m_highMeter.get());
    const double timeConstant = loudness > m_loudness
            ? kLoudnessAttackSeconds
            : kLoudnessReleaseSeconds;
    m_loudness += (loudness - m_loudness) *
            static_cast<float>(1.0 - std::exp(-elapsedSeconds / timeConstant));
}

// static
float VisualizerDeck::bandAmount(double intensity, float level) {
    const float audibleLevel = (level - kLevelFloor) / (1.0f - kLevelFloor);
    return std::clamp(static_cast<float>(intensity), 0.0f, 1.0f) *
            std::clamp(audibleLevel, 0.0f, 1.0f);
}

// static
float VisualizerDeck::advancedFlowPhase(float flowPhase, double bpm, double elapsedSeconds) {
    const double beatsPerSecond = (bpm > 0.0 ? bpm : kUnknownTempoBpm) / 60.0;
    const double cycles = beatsPerSecond * elapsedSeconds / kBeatsPerFlowCycle;
    return static_cast<float>(
            std::fmod(flowPhase + cycles * 2 * std::numbers::pi, 2 * std::numbers::pi));
}

void VisualizerDeck::setActive(bool active) {
    if (m_active == active) {
        return;
    }
    m_active = active;
    if (m_active && !m_coverRequested) {
        requestCover();
    }
}

void VisualizerDeck::slotLoadTrack(TrackPointer pTrack) {
    if (m_pLoadedTrack) {
        disconnect(m_pLoadedTrack.get(),
                &Track::coverArtUpdated,
                this,
                &VisualizerDeck::slotTrackCoverArtUpdated);
    }
    clearCover();
    m_pLoadedTrack = pTrack;
    if (m_pLoadedTrack) {
        connect(m_pLoadedTrack.get(),
                &Track::coverArtUpdated,
                this,
                &VisualizerDeck::slotTrackCoverArtUpdated);
    }
    slotTrackCoverArtUpdated();
}

void VisualizerDeck::slotLoadingTrack(TrackPointer, TrackPointer pOldTrack) {
    if (m_pLoadedTrack && pOldTrack == m_pLoadedTrack) {
        disconnect(m_pLoadedTrack.get(),
                &Track::coverArtUpdated,
                this,
                &VisualizerDeck::slotTrackCoverArtUpdated);
    }
    m_pLoadedTrack.reset();
    clearCover();
}

void VisualizerDeck::slotTrackCoverArtUpdated() {
    // [AI-generated] While the deck is inactive the cover is only marked as
    // outdated. It is requested when the deck becomes active.
    // [/AI-generated]
    m_coverRequested = false;
    if (m_active) {
        requestCover();
    }
}

void VisualizerDeck::requestCover() {
    if (!m_pLoadedTrack) {
        return;
    }
    m_coverRequested = true;
    CoverArtCache::requestTrackCover(this, m_pLoadedTrack);
}

void VisualizerDeck::slotCoverFound(
        const QObject* pRequester,
        const CoverInfo& coverInfo,
        const QPixmap& pixmap) {
    if (pRequester != this ||
            !m_pLoadedTrack ||
            m_pLoadedTrack->getLocation() != coverInfo.trackLocation) {
        return;
    }
    if (!m_active) {
        m_coverRequested = false;
        return;
    }
    // [AI-generated] Scaling and blurring a large cover takes too long for
    // the GUI thread, which also draws the waveforms. [/AI-generated]
    ++m_coverRequest;
    m_preparedCoverWatcher.setFuture(
            QtConcurrent::run(&VisualizerDeck::prepareCover, m_coverRequest, pixmap.toImage()));
}

// static
VisualizerDeck::PreparedCover VisualizerDeck::prepareCover(int request, QImage cover) {
    if (cover.isNull()) {
        cover = QImage(CoverArtUtils::defaultCoverLocation());
    }
    if (cover.width() > kMaxCoverSize || cover.height() > kMaxCoverSize) {
        cover = cover.scaled(kMaxCoverSize,
                kMaxCoverSize,
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation);
    }
    return PreparedCover{request, cover, blurredThumbnail(cover)};
}

void VisualizerDeck::slotCoverPrepared() {
    const PreparedCover prepared = m_preparedCoverWatcher.result();
    // [AI-generated] Ignore the cover of a track that has been replaced or
    // unloaded in the meantime. [/AI-generated]
    if (prepared.m_request != m_coverRequest) {
        return;
    }
    m_cover = prepared.m_cover;
    m_blurredCover = prepared.m_blurredCover;
    ++m_coverRevision;
}

void VisualizerDeck::clearCover() {
    // [AI-generated] The new request number invalidates a cover that is
    // still being prepared. [/AI-generated]
    ++m_coverRequest;
    m_coverRequested = false;
    m_cover = QImage();
    m_blurredCover = QImage();
    ++m_coverRevision;
}

} // namespace mixxx
