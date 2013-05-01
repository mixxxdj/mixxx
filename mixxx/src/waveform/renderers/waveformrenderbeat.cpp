#include <QDomNode>
#include <QPaintEvent>
#include <QPainter>

#include "waveform/renderers/waveformrenderbeat.h"

#include "controlobject.h"
#include "controlobjectthreadmain.h"
#include "track/beats.h"
#include "trackinfoobject.h"
#include "waveform/renderers/waveformwidgetrenderer.h"
#include "widget/wskincolor.h"
#include "widget/wwidget.h"

WaveformRenderBeat::WaveformRenderBeat(WaveformWidgetRenderer* waveformWidgetRenderer)
        : WaveformRendererAbstract(waveformWidgetRenderer),
          m_beatActive(0) {
}

WaveformRenderBeat::~WaveformRenderBeat() {
    if (m_beatActive)
        delete m_beatActive;
}

bool WaveformRenderBeat::init() {
    m_beatActive = new ControlObjectThreadMain(
                ControlObject::getControl(ConfigKey(m_waveformRenderer->getGroup(),"beat_active")));
    return true;
}

void WaveformRenderBeat::setup(const QDomNode& node) {
    m_beatColor.setNamedColor(WWidget::selectNodeQString(node, "BeatColor"));
    m_beatColor = WSkinColor::getCorrectColor(m_beatColor);

    if (m_beatColor.alphaF() > 0.99)
        m_beatColor.setAlphaF(0.9);
}

void WaveformRenderBeat::draw(QPainter* painter, QPaintEvent* /*event*/) {
    TrackPointer trackInfo = m_waveformRenderer->getTrackInfo();

    if(!trackInfo)
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
    beatPen.setWidth(1.5);

    while (it->hasNext()) {
        int beatPosition = it->next();
        // m_waveformRenderer->regulateVisualSample(beatPosition);
        double xBeatPoint = m_waveformRenderer->transformSampleIndexInRendererWorld(beatPosition);

        painter->setPen(beatPen);

        painter->drawLine(QPointF(xBeatPoint, 0.f),
                          QPointF(xBeatPoint,
                                  (float)m_waveformRenderer->getHeight()));
    }

    painter->restore();
}
