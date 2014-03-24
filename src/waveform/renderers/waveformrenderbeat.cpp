#include <QDomNode>
#include <QPaintEvent>
#include <QPainter>

#include "waveform/renderers/waveformrenderbeat.h"

#include "controlobject.h"
#include "controlobjectthread.h"
#include "track/beats.h"
#include "trackinfoobject.h"
#include "waveform/renderers/waveformwidgetrenderer.h"
#include "widget/wskincolor.h"
#include "widget/wwidget.h"

WaveformRenderBeat::WaveformRenderBeat(WaveformWidgetRenderer* waveformWidgetRenderer)
        : WaveformRendererAbstract(waveformWidgetRenderer),
          m_pBeatActive(NULL) {
}

WaveformRenderBeat::~WaveformRenderBeat() {
    if (m_pBeatActive)
        delete m_pBeatActive;
}

bool WaveformRenderBeat::init() {
    m_pBeatActive = new ControlObjectThread(
            m_waveformRenderer->getGroup(), "beat_active");
    return m_pBeatActive->valid();
}

void WaveformRenderBeat::setup(const QDomNode& node, const SkinContext& context) {
    m_beatColor.setNamedColor(context.selectString(node, "BeatColor"));
    m_beatColor = WSkinColor::getCorrectColor(m_beatColor);

    if (m_beatColor.alphaF() > 0.99)
        m_beatColor.setAlphaF(0.9);
}

void WaveformRenderBeat::draw(QPainter* painter, QPaintEvent* /*event*/) {
    TrackPointer trackInfo = m_waveformRenderer->getTrackInfo();

    if (!trackInfo)
        return;

    BeatsPointer trackBeats = trackInfo->getBeats();
    if (!trackBeats)
        return;

    const int trackSamples = m_waveformRenderer->getTrackSamples();
    if (trackSamples <= 0) {
        return;
    }

    const double firstDisplayedPosition = m_waveformRenderer->getFirstDisplayedPosition();
    const double lastDisplayedPosition = m_waveformRenderer->getLastDisplayedPosition();

    // qDebug() << "trackSamples" << trackSamples
    //          << "firstDisplayedPosition" << firstDisplayedPosition
    //          << "lastDisplayedPosition" << lastDisplayedPosition;

    QScopedPointer<BeatIterator> it(trackBeats->findBeats(
        firstDisplayedPosition * trackSamples, lastDisplayedPosition * trackSamples));

    // if no beat do not waste time saving/restoring painter
    if (!it || !it->hasNext()) {
        return;
    }

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    QPen beatPen(m_beatColor);
    beatPen.setWidthF(1.5);

    while (it->hasNext()) {
        int beatPosition = it->next();
        double xBeatPoint = m_waveformRenderer->transformSampleIndexInRendererWorld(beatPosition);

        painter->setPen(beatPen);

        painter->drawLine(QPointF(xBeatPoint, 0.f),
                          QPointF(xBeatPoint,
                          (float)m_waveformRenderer->getHeight()));
    }

    painter->restore();
}
