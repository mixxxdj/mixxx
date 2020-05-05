#include <QDomNode>
#include <QPaintEvent>
#include <QPainter>

#include "waveform/renderers/waveformrenderbeat.h"

#include "control/controlobject.h"
#include "track/beats.h"
#include "track/track.h"
#include "waveform/renderers/waveformwidgetrenderer.h"
#include "widget/wskincolor.h"
#include "widget/wwidget.h"
#include "util/painterscope.h"

WaveformRenderBeat::WaveformRenderBeat(WaveformWidgetRenderer* waveformWidgetRenderer)
        : WaveformRendererAbstract(waveformWidgetRenderer) {
}

WaveformRenderBeat::~WaveformRenderBeat() {
}

void WaveformRenderBeat::setup(const QDomNode& node, const SkinContext& context) {
    m_beatColor.setNamedColor(context.selectString(node, "BeatColor"));
    m_beatColor = WSkinColor::getCorrectColor(m_beatColor).toRgb();
    m_barColor.setNamedColor(context.selectString(node, "BarColor"));
    m_barColor = WSkinColor::getCorrectColor(m_barColor).toRgb();
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
    m_beatColor.setAlphaF(alpha/100.0);

    const int trackSamples = m_waveformRenderer->getTrackSamples();
    if (trackSamples <= 0) {
        return;
    }

    const double firstDisplayedPosition =
            m_waveformRenderer->getFirstDisplayedPosition();
    const double lastDisplayedPosition =
            m_waveformRenderer->getLastDisplayedPosition();

    // qDebug() << "trackSamples" << trackSamples
    //         << "track begin" << trackBeats->getFirstBeatPosition()
    //         << "track end" << trackBeats->getLastBeatPosition()
    //         << "firstDisplayedPosition" << firstDisplayedPosition
    //         << "lastDisplayedPosition" << lastDisplayedPosition;

    std::unique_ptr<mixxx::BeatIterator> it(trackBeats->findBeats(
            firstDisplayedPosition * trackSamples,
            lastDisplayedPosition * trackSamples));

    // if no beat do not waste time saving/restoring painter
    if (!it || !it->hasNext()) {
        return;
    }

    PainterScope PainterScope(painter);

    painter->setRenderHint(QPainter::Antialiasing);

    QPen beatPen(m_beatColor);
    beatPen.setWidthF(std::max(1.0, scaleFactor()));
    QPen barPen(m_barColor);
    barPen.setWidthF(std::max(1.0, scaleFactor()));

    const Qt::Orientation orientation = m_waveformRenderer->getOrientation();
    const float rendererWidth = m_waveformRenderer->getWidth();
    const float rendererHeight = m_waveformRenderer->getHeight();

    while (it->hasNext()) {
        // Beats->next returns Frame number and we need Sample number
        double beatSamplePosition = it->next() * mixxx::kEngineChannelCount;
        double xBeatPoint =
                m_waveformRenderer->transformSamplePositionInRendererWorld(beatSamplePosition);

        xBeatPoint = qRound(xBeatPoint);

        // Set the color
        painter->setPen((it->isBar() ? barPen : beatPen));

        if (orientation == Qt::Horizontal) {
            painter->drawLine(xBeatPoint, 0.0f, xBeatPoint, rendererHeight);
        } else {
            painter->drawLine(0.0f, xBeatPoint, rendererWidth, xBeatPoint);
        }
    }
}
