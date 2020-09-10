#include "waveform/renderers/waveformrenderbeat.h"

#include <QDomNode>
#include <QPaintEvent>
#include <QPainter>

#include "track/beats.h"
#include "track/track.h"
#include "util/frameadapter.h"
#include "util/painterscope.h"
#include "waveform/renderers/waveformbeat.h"
#include "waveform/renderers/waveformwidgetrenderer.h"
#include "widget/wskincolor.h"

namespace {
constexpr int kMaxZoomFactorToDisplayBeats = 15;
const QList<double> kWaveformZoomToTakeOutDownbeats({35, 70});
inline int opacityPercentageToAlpha(int opacityPercentageOnHundredScale) {
    return opacityPercentageOnHundredScale * 255.0 / 100;
}
} // namespace

WaveformRenderBeat::WaveformRenderBeat(WaveformWidgetRenderer* waveformWidgetRenderer)
        : WaveformRendererAbstract(waveformWidgetRenderer) {
    m_beats.resize(128);
    m_beatMarkers.resize(8);
}

WaveformRenderBeat::~WaveformRenderBeat() {
}

void WaveformRenderBeat::setup(const QDomNode& node, const SkinContext& context) {
    m_beatColor.setNamedColor(context.selectString(node, "BeatColor"));
    m_beatColor = WSkinColor::getCorrectColor(m_beatColor).toRgb();
}

void WaveformRenderBeat::draw(QPainter* painter, QPaintEvent* /*event*/) {
    TrackPointer trackInfo = m_waveformRenderer->getTrackInfo();
    if (!trackInfo)
        return;
    mixxx::BeatsPointer trackBeats = trackInfo->getBeats();

    if (!trackBeats)
        return;

    int alpha = m_waveformRenderer->beatGridAlpha();
    if (alpha == 0)
        return;
    m_beatColor.setAlphaF(alpha / 100.0);

    const int trackSamples = m_waveformRenderer->getTrackSamples();
    if (trackSamples <= 0) {
        return;
    }

    const double firstDisplayedPosition =
            m_waveformRenderer->getFirstDisplayedPosition();
    const double lastDisplayedPosition =
            m_waveformRenderer->getLastDisplayedPosition();

    const auto leftLimit = samplePosToFramePos(firstDisplayedPosition * trackSamples);
    const auto rightLimit = samplePosToFramePos(lastDisplayedPosition * trackSamples);
    int displayBeatsStartIdxInclusive = trackBeats->findNextBeat(leftLimit)->beatIndex();
    int displayBeatsEndIdxInclusive = trackBeats->findPrevBeat(rightLimit)->beatIndex();

    // We perform explicit limit check due to fuzzy beat boundaries used by Beats class.
    // So it returns those beats which are slightly outside screen boundaries and we
    // will remove them.
    const auto leftMostBeatPosition =
            trackBeats->getBeatAtIndex(displayBeatsStartIdxInclusive)
                    ->framePosition();
    if (leftMostBeatPosition < leftLimit) {
        displayBeatsStartIdxInclusive++;
    }
    const auto rightMostBeatPosition =
            trackBeats->getBeatAtIndex(displayBeatsEndIdxInclusive)
                    ->framePosition();
    if (rightMostBeatPosition > rightLimit) {
        displayBeatsEndIdxInclusive--;
    }

    // if no beat do not waste time saving/restoring painter
    if (displayBeatsStartIdxInclusive > displayBeatsEndIdxInclusive) {
        return;
    }

    PainterScope PainterScope(painter);

    painter->setRenderHint(QPainter::Antialiasing);

    QPen beatPen(m_beatColor);
    beatPen.setWidthF(std::max(1.0, scaleFactor()));
    painter->setPen(beatPen);

    const Qt::Orientation orientation = m_waveformRenderer->getOrientation();
    const float rendererWidth = m_waveformRenderer->getWidth();
    const float rendererHeight = m_waveformRenderer->getHeight();

    int beatCount = 0;
    int beatMarkerCount = 0;
    QList<WaveformBeat> beatsOnScreen;

    for (int i = displayBeatsStartIdxInclusive; i <= displayBeatsEndIdxInclusive; i++) {
        auto beat = trackBeats->getBeatAtIndex(i).value();
        double beatSamplePosition = framePosToSamplePos(beat.framePosition());
        int beatPixelPositionInWidgetSpace = qRound(
                m_waveformRenderer->transformSamplePositionInRendererWorld(
                        beatSamplePosition));

        // If we don't have enough space, double the size.
        if (beatCount >= m_beats.size()) {
            m_beats.resize(m_beats.size() * 2);
        }

        auto* waveformBeat = &m_beats[beatCount];
        waveformBeat->setPositionPixels(beatPixelPositionInWidgetSpace);
        waveformBeat->setBeatGridMode(m_waveformRenderer->beatGridMode());
        waveformBeat->setBeat(beat);
        waveformBeat->setOrientation(orientation);
        waveformBeat->setLength((orientation == Qt::Horizontal) ? rendererHeight : rendererWidth);
        double zoomFactor = m_waveformRenderer->getZoomFactor();
        bool isVisible = beat.type() == mixxx::BeatType::Downbeat ||
                (beat.type() == mixxx::BeatType::Beat &&
                        zoomFactor <
                                kMaxZoomFactorToDisplayBeats);
        // TODO: Calculate adjacent beat distance to decide which beats to hide.
        if (isVisible && zoomFactor >= kWaveformZoomToTakeOutDownbeats.at(0)) {
            DEBUG_ASSERT(beat.type() == mixxx::BeatType::Downbeat);
            const int zoomLevelIdx =
                    std::lower_bound(
                            kWaveformZoomToTakeOutDownbeats.constBegin(),
                            kWaveformZoomToTakeOutDownbeats.constEnd(),
                            zoomFactor)
                            .i -
                    kWaveformZoomToTakeOutDownbeats.constBegin().i;
            const int allowedDownbeatIndexFactor = std::pow(2, zoomLevelIdx);
            if ((beat.barIndex() + 1) % allowedDownbeatIndexFactor != 0) {
                isVisible = false;
            }
        }
        waveformBeat->setVisible(isVisible);
        waveformBeat->setAlpha(opacityPercentageToAlpha(alpha));
        beatsOnScreen.append(*waveformBeat);
        beatCount++;

        if (beatMarkerCount >= m_beatMarkers.size()) {
            m_beatMarkers.resize(m_beatMarkers.size() * 2);
        }
        if (beat.markers()) {
            m_beatMarkers[beatMarkerCount].setPositionPixels(beatPixelPositionInWidgetSpace);
            m_beatMarkers[beatMarkerCount].setLength(
                    (orientation == Qt::Horizontal) ? rendererHeight
                                                    : rendererWidth);
            QStringList displayItems;
            bool markerIsDisplayed = false;
            if (beat.markers().testFlag(mixxx::BeatMarker::TimeSignature)) {
                QString timeSignatureString =
                        QString::number(beat.timeSignature().getBeatsPerBar()) +
                        "/" +
                        QString::number(beat.timeSignature().getNoteValue());
                displayItems.append(timeSignatureString);
                markerIsDisplayed = true;
            }
            if (markerIsDisplayed) {
                m_beatMarkers[beatMarkerCount].setTextDisplayItems(displayItems);
                beatMarkerCount++;
            }
        }
    }

    // Make sure to use constData to prevent detaches!
    for (int i = 0; i < beatCount; i++) {
        const auto currentBeat = m_beats.constData() + i;
        painter->setPen(beatPen);
        currentBeat->draw(painter);
    }
    for (int i = 0; i < beatMarkerCount; i++) {
        const auto currentBeatMarker = m_beatMarkers.constData() + i;
        currentBeatMarker->draw(painter);
    }
    m_waveformRenderer->setBeatsOnScreen(beatsOnScreen);
}
