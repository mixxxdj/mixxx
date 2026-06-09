#include "waveform/renderers/waveformrenderbeat.h"

#include <QPainter>

#include "track/track.h"
#include "util/painterscope.h"
#include "waveform/renderers/waveformwidgetrenderer.h"
#include "waveform/waveformwidgetfactory.h"
#include "widget/wskincolor.h"

class QPaintEvent;

WaveformRenderBeat::WaveformRenderBeat(WaveformWidgetRenderer* waveformWidgetRenderer)
        : WaveformRendererAbstract(waveformWidgetRenderer),
          m_introStartPosCO(m_waveformRenderer->getGroup(),
                  QStringLiteral("intro_start_position")) {
    m_beats.resize(128);
    m_downbeats.resize(32);
}

int WaveformRenderBeat::computeAnchorBeatIndex(
        const mixxx::BeatsPointer& pBeats) const {
    const double introStartSample = m_introStartPosCO.get();
    if (introStartSample <= 0.0) {
        return 0;
    }
    const auto introPos = mixxx::audio::FramePos::fromEngineSamplePos(introStartSample);
    if (!introPos.isValid()) {
        return 0;
    }
    const auto closestBeat = pBeats->findClosestBeat(introPos);
    if (!closestBeat.isValid()) {
        return 0;
    }
    const auto anchorIt = pBeats->iteratorFrom(closestBeat);
    if (anchorIt != pBeats->cend()) {
        return anchorIt - pBeats->cfirstmarker();
    }
    return 0;
}

WaveformRenderBeat::~WaveformRenderBeat() {
}

void WaveformRenderBeat::setup(const QDomNode& node, const SkinContext& context) {
    m_beatColor = QColor(context.selectString(node, "BeatColor"));
    m_beatColor = WSkinColor::getCorrectColor(m_beatColor).toRgb();

    const QString downbeatColorStr = context.selectString(node, "DownbeatColor");
    if (downbeatColorStr.isEmpty()) {
        m_downbeatColor = m_beatColor;
    } else {
        m_downbeatColor = QColor(downbeatColorStr);
        m_downbeatColor = WSkinColor::getCorrectColor(m_downbeatColor).toRgb();
    }
}

void WaveformRenderBeat::draw(QPainter* painter, QPaintEvent* /*event*/) {
    TrackPointer pTrackInfo = m_waveformRenderer->getTrackInfo();

    if (!pTrackInfo) {
        return;
    }

    mixxx::BeatsPointer trackBeats = pTrackInfo->getBeats();
    if (!trackBeats) {
        return;
    }

    int alpha = m_waveformRenderer->getBeatGridAlpha();
    if (alpha == 0) {
        return;
    }
#ifdef MIXXX_USE_QOPENGL
    // Using alpha transparency with drawLines causes a graphical issue when
    // drawing with QPainter on the QOpenGLWindow: instead of individual lines
    // a large rectangle encompassing all beatlines is drawn.
    m_beatColor.setAlphaF(1.f);
    m_downbeatColor.setAlphaF(1.f);
#else
    m_beatColor.setAlphaF(alpha / 100.0);
    m_downbeatColor.setAlphaF(alpha / 100.0);
#endif

    const double trackSamples = m_waveformRenderer->getTrackSamples();
    if (trackSamples <= 0) {
        return;
    }

    const float devicePixelRatio = m_waveformRenderer->getDevicePixelRatio();

    const double firstDisplayedPosition =
            m_waveformRenderer->getFirstDisplayedPosition();
    const double lastDisplayedPosition =
            m_waveformRenderer->getLastDisplayedPosition();

    const auto startPosition = mixxx::audio::FramePos::fromEngineSamplePos(
            firstDisplayedPosition * trackSamples);
    const auto endPosition = mixxx::audio::FramePos::fromEngineSamplePos(
            lastDisplayedPosition * trackSamples);
    auto it = trackBeats->iteratorFrom(startPosition);

    // if no beat do not waste time saving/restoring painter
    if (it == trackBeats->cend() || *it > endPosition) {
        return;
    }

    PainterScope PainterScope(painter);

    painter->setRenderHint(QPainter::Antialiasing);

    const Qt::Orientation orientation = m_waveformRenderer->getOrientation();
    const float rendererWidth = m_waveformRenderer->getWidth();
    const float rendererHeight = m_waveformRenderer->getHeight();

    int beatCount = 0;
    int downbeatCount = 0;

    const auto firstMarker = trackBeats->cfirstmarker();
    const int anchorBeatIndex = computeAnchorBeatIndex(trackBeats);
    auto* pFactory = WaveformWidgetFactory::instance();
    const int beatsPerBar = pFactory->getBeatsPerBar();
    const bool downbeatsEnabled = pFactory->getDownbeatsEnabled();

    for (; it != trackBeats->cend() && *it <= endPosition; ++it) {
        double beatPosition = it->toEngineSamplePos();
        double xBeatPoint =
                m_waveformRenderer->transformSamplePositionInRendererWorld(beatPosition);

        xBeatPoint = qRound(xBeatPoint * devicePixelRatio) / devicePixelRatio;

        bool isDownbeat = false;
        if (downbeatsEnabled && beatsPerBar > 0) {
            const int globalBeatIndex = (it - firstMarker) - anchorBeatIndex;
            isDownbeat = ((globalBeatIndex % beatsPerBar) + beatsPerBar) % beatsPerBar == 0;
        }

        if (isDownbeat) {
            // If we don't have enough space, double the size.
            if (downbeatCount >= m_downbeats.size()) {
                m_downbeats.resize(m_downbeats.size() * 2);
            }

            if (orientation == Qt::Horizontal) {
                m_downbeats[downbeatCount++].setLine(
                        xBeatPoint, 0.0f, xBeatPoint, rendererHeight);
            } else {
                m_downbeats[downbeatCount++].setLine(
                        0.0f, xBeatPoint, rendererWidth, xBeatPoint);
            }
        } else {
            // If we don't have enough space, double the size.
            if (beatCount >= m_beats.size()) {
                m_beats.resize(m_beats.size() * 2);
            }

            if (orientation == Qt::Horizontal) {
                m_beats[beatCount++].setLine(
                        xBeatPoint, 0.0f, xBeatPoint, rendererHeight);
            } else {
                m_beats[beatCount++].setLine(
                        0.0f, xBeatPoint, rendererWidth, xBeatPoint);
            }
        }
    }

    // Draw regular beats with thin pen
    if (beatCount > 0) {
        QPen beatPen(m_beatColor);
        beatPen.setWidthF(std::max(1.0, scaleFactor()));
        painter->setPen(beatPen);
        // Make sure to use constData to prevent detaches!
        painter->drawLines(m_beats.constData(), beatCount);
    }

    // Draw downbeats with thick pen
    if (downbeatCount > 0) {
        QPen downbeatPen(m_downbeatColor);
        downbeatPen.setWidthF(std::max(1.0, 2.0 * scaleFactor()));
        painter->setPen(downbeatPen);
        // Make sure to use constData to prevent detaches!
        painter->drawLines(m_downbeats.constData(), downbeatCount);
    }
}
