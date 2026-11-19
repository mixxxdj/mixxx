#include "visualizer/visualizerdeck.h"

#include <QImageReader>
#include <QPixmap>
#include <QtConcurrentRun>
#include <algorithm>
#include <cmath>
#include <numbers>

#include "library/coverartutils.h"
#include "library/playercoverartwatcher.h"
#include "mixer/basetrackplayer.h"
#include "moc_visualizerdeck.cpp"
#include "util/imageutils.h"

namespace mixxx {

namespace {

using namespace std::chrono_literals;

// The band meters report the mean absolute sample value of a band. These gains
// scale the levels of typical music at full volume to about 1. The highs carry
// much less energy than the bass.
constexpr float kBassGain = 6.0f;
constexpr float kMidGain = 15.0f;
constexpr float kHighGain = 40.0f;

// Levels below this are treated as silence. This hides what leaks into a band
// from its neighbors while the band is killed with the EQ.
constexpr float kLevelFloor = 0.08f;

constexpr std::chrono::duration<double> kLevelRelease = 150ms;
constexpr std::chrono::duration<double> kLoudnessAttack = 100ms;
constexpr std::chrono::duration<double> kLoudnessRelease = 1500ms;

// The size and the blur radius of the blurred copy of a cover. It is drawn
// enlarged to the size of the window, which blurs it further.
constexpr int kBlurredCoverSize = 64;
constexpr int kBlurredCoverRadius = 5;

constexpr double kUnknownTempoBpm = 120.0;

float releasedLevel(
        float currentLevel, float newLevel, std::chrono::duration<double> elapsed) {
    const float decayed = currentLevel *
            static_cast<float>(std::exp(-(elapsed / kLevelRelease)));
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
          m_pCoverArtWatcher(make_parented<PlayerCoverArtWatcher>(pPlayer, this)),
          m_coverRevision(0),
          m_active(false),
          m_coverRequested(false),
          m_coverRequest(0) {
    connect(&m_preparedCoverWatcher,
            &QFutureWatcher<PreparedCover>::finished,
            this,
            &VisualizerDeck::slotCoverPrepared);
    connect(m_pCoverArtWatcher,
            &PlayerCoverArtWatcher::trackChanged,
            this,
            &VisualizerDeck::slotTrackChanged);
    connect(m_pCoverArtWatcher,
            &PlayerCoverArtWatcher::coverOutdated,
            this,
            &VisualizerDeck::slotCoverOutdated);
    connect(m_pCoverArtWatcher,
            &PlayerCoverArtWatcher::coverFound,
            this,
            &VisualizerDeck::slotCoverFound);
    m_pCoverArtWatcher->slotLoadTrack(pPlayer->getLoadedTrack());
}

void VisualizerDeck::update(std::chrono::duration<double> elapsed) {
    const float bassLevel = scaledLevel(m_bassMeter, kBassGain);
    const float midLevel = scaledLevel(m_midMeter, kMidGain);
    const float highLevel = scaledLevel(m_highMeter, kHighGain);
    m_bassLevel = releasedLevel(m_bassLevel, bassLevel, elapsed);
    m_midLevel = releasedLevel(m_midLevel, midLevel, elapsed);
    m_highLevel = releasedLevel(m_highLevel, highLevel, elapsed);

    const float loudness = static_cast<float>(
            m_bassMeter.get() + m_midMeter.get() + m_highMeter.get());
    const std::chrono::duration<double> timeConstant = loudness > m_loudness
            ? kLoudnessAttack
            : kLoudnessRelease;
    m_loudness += (loudness - m_loudness) *
            static_cast<float>(1.0 - std::exp(-(elapsed / timeConstant)));
}

// static
float VisualizerDeck::bandAmount(double intensity, float level) {
    const float audibleLevel = (level - kLevelFloor) / (1.0f - kLevelFloor);
    return std::clamp(static_cast<float>(intensity), 0.0f, 1.0f) *
            std::clamp(audibleLevel, 0.0f, 1.0f);
}

// static
float VisualizerDeck::advancedTempoPhase(float phase,
        double bpm,
        double beatsPerCycle,
        std::chrono::duration<double> elapsed) {
    const double beatsPerSecond = (bpm > 0.0 ? bpm : kUnknownTempoBpm) / 60.0;
    const double cycles = beatsPerSecond * elapsed.count() / beatsPerCycle;
    return static_cast<float>(
            std::fmod(phase + cycles * 2 * std::numbers::pi, 2 * std::numbers::pi));
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

void VisualizerDeck::slotTrackChanged() {
    // The new request number invalidates a cover that is still being
    // prepared.
    ++m_coverRequest;
    m_coverRequested = false;
    m_cover = QImage();
    m_blurredCover = QImage();
    ++m_coverRevision;
}

void VisualizerDeck::slotCoverOutdated() {
    // While the deck is inactive the cover is only marked as outdated. It
    // is requested when the deck becomes active.
    m_coverRequested = false;
    if (m_active) {
        requestCover();
    }
}

void VisualizerDeck::requestCover() {
    if (!m_pCoverArtWatcher->loadedTrack()) {
        return;
    }
    m_coverRequested = true;
    m_pCoverArtWatcher->requestCover();
}

void VisualizerDeck::slotCoverFound(const QPixmap& pixmap) {
    if (!m_active) {
        m_coverRequested = false;
        return;
    }
    // Scaling and blurring a large cover takes too long for the GUI thread,
    // which also draws the waveforms.
    ++m_coverRequest;
    m_preparedCoverWatcher.setFuture(
            QtConcurrent::run(&VisualizerDeck::prepareCover, m_coverRequest, pixmap.toImage()));
}

// static
VisualizerDeck::PreparedCover VisualizerDeck::prepareCover(int request, QImage cover) {
    if (cover.isNull()) {
        // The default cover is a small vector image. Render it in the size
        // that is used for drawing instead of enlarging it afterwards.
        QImageReader reader(CoverArtUtils::defaultCoverLocation());
        reader.setScaledSize(QSize(kMaxCoverSize, kMaxCoverSize));
        cover = reader.read();
    }
    if (cover.width() > kMaxCoverSize || cover.height() > kMaxCoverSize) {
        cover = cover.scaled(kMaxCoverSize,
                kMaxCoverSize,
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation);
    }
    return PreparedCover{request,
            cover,
            blurredThumbnail(cover, kBlurredCoverSize, kBlurredCoverRadius)};
}

void VisualizerDeck::slotCoverPrepared() {
    const PreparedCover prepared = m_preparedCoverWatcher.result();
    // Ignore the cover of a track that has been replaced or unloaded in the
    // meantime.
    if (prepared.m_request != m_coverRequest) {
        return;
    }
    m_cover = prepared.m_cover;
    m_blurredCover = prepared.m_blurredCover;
    ++m_coverRevision;
}

} // namespace mixxx
