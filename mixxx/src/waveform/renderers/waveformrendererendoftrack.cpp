#include <QDomNode>
#include <QPaintEvent>
#include <QPainter>

#include "waveformrendererendoftrack.h"

#include "defs.h"
#include "waveformwidgetrenderer.h"

#include "controlobject.h"
#include "controlobjectthreadmain.h"

#include "widget/wskincolor.h"
#include "widget/wwidget.h"

#include "util/timer.h"

WaveformRendererEndOfTrack::WaveformRendererEndOfTrack(
        WaveformWidgetRenderer* waveformWidgetRenderer)
    : WaveformRendererAbstract(waveformWidgetRenderer),
      m_endOfTrackControl(NULL),
      m_endOfTrackEnabled(false),
      m_trackSampleRate(NULL),
      m_playControl(NULL),
      m_loopControl(NULL),
      m_color(200, 25, 20),
      m_blinkingPeriodMillis(1000),
      m_remainingTimeTriggerSeconds(30.0) {
}

WaveformRendererEndOfTrack::~WaveformRendererEndOfTrack() {
    delete m_endOfTrackControl;
    delete m_trackSampleRate;
    delete m_playControl;
    delete m_loopControl;
}

bool WaveformRendererEndOfTrack::init() {
    m_timer.restart();

    m_endOfTrackControl = new ControlObjectThreadMain(
                ControlObject::getControl(ConfigKey(m_waveformRenderer->getGroup(), "end_of_track")));
    m_endOfTrackControl->setExtern(0.);
    m_endOfTrackEnabled = false;

    m_trackSampleRate = new ControlObjectThreadMain(
                ControlObject::getControl(ConfigKey(m_waveformRenderer->getGroup(), "track_samplerate")));
    m_playControl = new ControlObjectThreadMain(
                ControlObject::getControl(ConfigKey(m_waveformRenderer->getGroup(), "play")));
    m_loopControl = new ControlObjectThreadMain(
                ControlObject::getControl(ConfigKey(m_waveformRenderer->getGroup(), "loop_enabled")));
    return true;
}

void WaveformRendererEndOfTrack::setup(const QDomNode& node) {
    m_color = QColor(200, 25, 20);
    const QString endOfTrackColorName = WWidget::selectNodeQString(node, "EndOfTrackColor");
    if (!endOfTrackColorName.isNull()) {
        m_color.setNamedColor(endOfTrackColorName);
        m_color = WSkinColor::getCorrectColor(m_color);
    }
    m_pen = QPen(QBrush(m_color), 2.5);
}

void WaveformRendererEndOfTrack::onResize() {
    m_backRects.resize(4);
    for (int i = 0; i < 4; i++) {
        m_backRects[i].setTop(0);
        m_backRects[i].setBottom(m_waveformRenderer->getHeight());
        m_backRects[i].setLeft(m_waveformRenderer->getWidth()/2 +
                i*m_waveformRenderer->getWidth()/8);
        m_backRects[i].setRight(m_waveformRenderer->getWidth());
    }
    /*
    m_gradient.setStart(m_waveformRenderer->getWidth()/2, 0);
    m_gradient.setFinalStop(m_waveformRenderer->getWidth(), 0);
    m_gradient.setColorAt(0, Qt::transparent);
    m_gradient.setColorAt(1, m_color);
    */
}

void WaveformRendererEndOfTrack::draw(QPainter* painter,
                                      QPaintEvent* /*event*/) {

    const double trackSamples = m_waveformRenderer->getTrackSamples();
    const double sampleRate = m_trackSampleRate->get();

    /*qDebug() << "WaveformRendererEndOfTrack :: "
             << "trackSamples" << trackSamples
             << "sampleRate" << sampleRate
             << "m_playControl->get()" << m_playControl->get()
             << "m_loopControl->get()" << m_loopControl->get();*/

    m_endOfTrackEnabled = m_endOfTrackControl->get() > 0.5;

    //special case of track not long enougth
    const double trackLength = 0.5 * trackSamples / sampleRate;

    if (sampleRate < 0.1 //not ready
            || trackSamples < 0.1 //not ready
            || m_playControl->get() < 0.5 //not playing
            || m_loopControl->get() > 0.5 //in loop
            || trackLength <= m_remainingTimeTriggerSeconds //track too short
            ) {
        if (m_endOfTrackEnabled) {
            m_endOfTrackControl->slotSet(0.0);
            m_endOfTrackEnabled = false;
        }
        return;
    }

    const double dPlaypos = m_waveformRenderer->getPlayPos();
    const double remainingFrames = (1.0 - dPlaypos) * 0.5 * trackSamples;
    const double remainingTime = remainingFrames / sampleRate;

    if (remainingTime > m_remainingTimeTriggerSeconds) {
        if (m_endOfTrackEnabled) {
            m_endOfTrackControl->slotSet(0.);
            m_endOfTrackEnabled = false;
        }
        return;
    }

    // end of track is on
    if (!m_endOfTrackEnabled) {
        m_endOfTrackControl->slotSet(1.);
        m_endOfTrackEnabled = true;

        //qDebug() << "EndOfTrack ON";
    }

    ScopedTimer t("WaveformRendererEndOfTrack::draw");

    const int elapsed = m_timer.elapsed() % m_blinkingPeriodMillis;

    const double blickIntensity = (double)(2 * abs(elapsed - m_blinkingPeriodMillis/2)) /
            m_blinkingPeriodMillis;
    const double criticalIntensity = (m_remainingTimeTriggerSeconds - remainingTime) /
            m_remainingTimeTriggerSeconds;

    painter->save();
    painter->resetTransform();
    painter->setOpacity(0.5 * blickIntensity);
    painter->setPen(m_pen);
    painter->drawRect(1, 1,
            m_waveformRenderer->getWidth() - 2, m_waveformRenderer->getHeight() - 2);

    painter->setOpacity(0.5 * 0.25 * criticalIntensity * blickIntensity);
    painter->setPen(QPen(Qt::transparent));
    painter->setBrush(m_color);
    painter->drawRects(m_backRects);
    //painter->setOpacity(0.5 * criticalIntensity * blickIntensity);
    //painter->fillRect(m_waveformRenderer->getWidth()/2, 1,
    //        m_waveformRenderer->getWidth() - 2, m_waveformRenderer->getHeight() - 2,
    //        m_gradient);
    painter->restore();
}
