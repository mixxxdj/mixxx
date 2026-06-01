#include "waveform/renderers/waveformrenderbeat.h"

#include <QPainter>

#include <cmath>

#include "track/beats.h"
#include "track/track.h"
#include "util/painterscope.h"
#include "waveform/renderers/waveformwidgetrenderer.h"
#include "widget/wskincolor.h"

class QPaintEvent;

WaveformRenderBeat::WaveformRenderBeat(WaveformWidgetRenderer* waveformWidgetRenderer)
        : WaveformRendererAbstract(waveformWidgetRenderer) {
    m_beats.resize(128);
    m_downbeats.resize(32);
}

WaveformRenderBeat::~WaveformRenderBeat() {
}

void WaveformRenderBeat::setup(const QDomNode& node, const SkinContext& context) {
    m_beatColor = QColor(context.selectString(node, "BeatColor"));
    m_beatColor = WSkinColor::getCorrectColor(m_beatColor).toRgb();

    // Downbeat (bar-start) line. Defaults to a bright rekordbox-style red when
    // the skin doesn't specify a <DownbeatColor>.
    m_downbeatColor = QColor(context.selectString(node, "DownbeatColor"));
    if (!m_downbeatColor.isValid()) {
        m_downbeatColor = QColor(255, 0, 0);
    }
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
#else
    m_beatColor.setAlphaF(alpha/100.0);
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

    PainterScope PainterScope(painter);

    painter->setRenderHint(QPainter::Antialiasing);

    // Downbeat anchor: the grid's first marker sits on a downbeat, and bars are
    // 4 beats, so every 4th beat from the anchor is a bar start (drawn red).
    const auto firstMarker = trackBeats->cfirstmarker();
    const double firstMarkerFrames = (*firstMarker).value();
    const double beatLengthFrames = firstMarker.beatLengthFrames();

    const Qt::Orientation orientation = m_waveformRenderer->getOrientation();
    const float rendererWidth = m_waveformRenderer->getWidth();
    const float rendererHeight = m_waveformRenderer->getHeight();

    int beatCount = 0;
    int downbeatCount = 0;

    for (; it != trackBeats->cend() && *it <= endPosition; ++it) {
        double beatPosition = it->toEngineSamplePos();
        double xBeatPoint =
                m_waveformRenderer->transformSamplePositionInRendererWorld(beatPosition);

        xBeatPoint = qRound(xBeatPoint * devicePixelRatio) / devicePixelRatio;

        bool isDownbeat = false;
        if (beatLengthFrames > 0.0) {
            const long long idx = std::llround(
                    (it->value() - firstMarkerFrames) / beatLengthFrames);
            isDownbeat = (((idx % 4) + 4) % 4) == 0;
        }

        // If we don't have enough space, double the size.
        QVector<QLineF>& lines = isDownbeat ? m_downbeats : m_beats;
        int& count = isDownbeat ? downbeatCount : beatCount;
        if (count >= lines.size()) {
            lines.resize(lines.size() * 2);
        }
        if (orientation == Qt::Horizontal) {
            lines[count++].setLine(xBeatPoint, 0.0f, xBeatPoint, rendererHeight);
        } else {
            lines[count++].setLine(0.0f, xBeatPoint, rendererWidth, xBeatPoint);
        }
    }

    QColor downbeatColor = m_downbeatColor;
#ifdef MIXXX_USE_QOPENGL
    downbeatColor.setAlphaF(1.f);
#else
    downbeatColor.setAlphaF(alpha / 100.0);
#endif

    // Regular beats first, then the red downbeats on top. Make sure to use
    // constData to prevent detaches!
    QPen beatPen(m_beatColor);
    beatPen.setWidthF(std::max(1.0, scaleFactor()));
    painter->setPen(beatPen);
    painter->drawLines(m_beats.constData(), beatCount);

    QPen downbeatPen(downbeatColor);
    downbeatPen.setWidthF(std::max(1.0, scaleFactor()));
    painter->setPen(downbeatPen);
    painter->drawLines(m_downbeats.constData(), downbeatCount);
}
