#include "visualizer/visualizerdeck.h"

#include <QPixmap>
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

// [AI-generated] The summed loudness above which the covers are drawn at
// full brightness. [/AI-generated]
constexpr float kFullBrightnessLoudness = 0.01f;

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
          m_flowPhase(0.0f),
          m_coverRevision(0) {
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

    m_flowPhase = advancedFlowPhase(m_flowPhase, m_bpm.get(), elapsedSeconds);
}

// static
float VisualizerDeck::bandAmount(double intensity, float level) {
    const float audibleLevel = (level - kLevelFloor) / (1.0f - kLevelFloor);
    return std::clamp(static_cast<float>(intensity), 0.0f, 1.0f) *
            std::clamp(audibleLevel, 0.0f, 1.0f);
}

// static
float VisualizerDeck::mixWeight(float loudness, float totalLoudness) {
    if (loudness <= 0.0f) {
        return 0.0f;
    }
    return loudness / std::max(totalLoudness, kFullBrightnessLoudness);
}

// static
float VisualizerDeck::advancedFlowPhase(float flowPhase, double bpm, double elapsedSeconds) {
    const double beatsPerSecond = (bpm > 0.0 ? bpm : kUnknownTempoBpm) / 60.0;
    const double cycles = beatsPerSecond * elapsedSeconds / kBeatsPerFlowCycle;
    return static_cast<float>(
            std::fmod(flowPhase + cycles * 2 * std::numbers::pi, 2 * std::numbers::pi));
}

void VisualizerDeck::slotLoadTrack(TrackPointer pTrack) {
    if (m_pLoadedTrack) {
        disconnect(m_pLoadedTrack.get(),
                &Track::coverArtUpdated,
                this,
                &VisualizerDeck::slotTrackCoverArtUpdated);
    }
    setCover(QImage());
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
    setCover(QImage());
}

void VisualizerDeck::slotTrackCoverArtUpdated() {
    if (m_pLoadedTrack) {
        CoverArtCache::requestTrackCover(this, m_pLoadedTrack);
    }
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
    QImage cover = pixmap.toImage();
    if (cover.isNull()) {
        cover = QImage(CoverArtUtils::defaultCoverLocation());
    }
    if (cover.width() > kMaxCoverSize || cover.height() > kMaxCoverSize) {
        cover = cover.scaled(kMaxCoverSize,
                kMaxCoverSize,
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation);
    }
    setCover(cover);
}

void VisualizerDeck::setCover(const QImage& cover) {
    m_cover = cover;
    m_blurredCover = blurredThumbnail(cover);
    ++m_coverRevision;
}

} // namespace mixxx
