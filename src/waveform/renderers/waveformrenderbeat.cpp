#include "waveform/renderers/waveformrenderbeat.h"

#include <QPainter>

#include "track/track.h"
#include "util/painterscope.h"
#include "waveform/renderers/waveformwidgetrenderer.h"
#include "widget/wskincolor.h"

class QPaintEvent;

WaveformRenderBeat::WaveformRenderBeat(WaveformWidgetRenderer* waveformWidgetRenderer)
        : WaveformRendererAbstract(waveformWidgetRenderer) {
    m_beats.reserve(128);
    m_downbeats.reserve(32);
}

WaveformRenderBeat::~WaveformRenderBeat() {
}

void WaveformRenderBeat::setup(const QDomNode& node, const SkinContext& context) {
    m_beatColor = QColor(context.selectString(node, "BeatColor"));
    m_beatColor = WSkinColor::getCorrectColor(m_beatColor).toRgb();

    m_downbeatColor.setNamedColor(context.selectString(node, "DownbeatColor"));
    m_downbeatColor = WSkinColor::getCorrectColor(m_downbeatColor).toRgb();
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
    m_beatColor.setAlphaF(alpha/100.0);
    m_downbeatColor.setAlphaF(alpha / 100.0);
#endif

    const double trackSamples = m_waveformRenderer->getTrackSamples();
    if (trackSamples <= 0) {
        return;
    }

    const double firstDisplayedPosition =
            m_waveformRenderer->getFirstDisplayedPosition();
    const double lastDisplayedPosition =
            m_waveformRenderer->getLastDisplayedPosition();

    // qDebug() << "trackSamples" << trackSamples
    //          << "firstDisplayedPosition" << firstDisplayedPosition
    //          << "lastDisplayedPosition" << lastDisplayedPosition;

    const auto startPosition = mixxx::audio::FramePos::fromEngineSamplePos(
            firstDisplayedPosition * trackSamples);
    const auto endPosition = mixxx::audio::FramePos::fromEngineSamplePos(
            lastDisplayedPosition * trackSamples);
    auto it = trackBeats->iteratorFrom(startPosition);

    // if no beat do not waste time saving/restoring painter
    if (it == trackBeats->cend() || *it > endPosition) {
        return;
    }

    const Qt::Orientation orientation = m_waveformRenderer->getOrientation();
    const float rendererWidth = m_waveformRenderer->getWidth();
    const float rendererHeight = m_waveformRenderer->getHeight();

    m_beats.clear();
    m_downbeats.clear();

    for (; it != trackBeats->cend() && *it <= endPosition; ++it) {
        double beatPosition = it->toEngineSamplePos();
        double xBeatPoint =
                m_waveformRenderer->transformSamplePositionInRendererWorld(beatPosition);

        xBeatPoint = qRound(xBeatPoint);

        QLineF line;
        if (orientation == Qt::Horizontal) {
            line.setLine(xBeatPoint, 0.0f, xBeatPoint, rendererHeight);
        } else {
            line.setLine(0.0f, xBeatPoint, rendererWidth, xBeatPoint);
        }

        auto& lines = it.isDownbeat() ? m_downbeats : m_beats;

        // If we don't have enough space, double the capacity.
        if (lines.size() == lines.capacity()) {
            lines.reserve(lines.capacity() * 2);
        }

        lines.append(line);
    }

    PainterScope PainterScope(painter);
    painter->setRenderHint(QPainter::Antialiasing);

    if (!m_beats.isEmpty()) {
        QPen pen(m_beatColor);
        pen.setWidthF(std::max(1.0, scaleFactor()));
        painter->setPen(pen);

        // Make sure to use constData to prevent detaches!
        painter->drawLines(m_beats);
    }

    if (!m_downbeats.isEmpty()) {
        QPen pen(m_downbeatColor);
        pen.setWidthF(std::max(1.0, scaleFactor()));
        painter->setPen(pen);

        // Make sure to use constData to prevent detaches!
        painter->drawLines(m_downbeats);
    }
}
