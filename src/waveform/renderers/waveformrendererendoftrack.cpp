#include <QDomNode>
#include <QPaintEvent>
#include <QPainter>

#include "waveformrendererendoftrack.h"
#include "waveformwidgetrenderer.h"

#include "control/controlobject.h"
#include "control/controlproxy.h"

#include "widget/wskincolor.h"
#include "widget/wwidget.h"

#include "util/timer.h"

WaveformRendererEndOfTrack::WaveformRendererEndOfTrack(
        WaveformWidgetRenderer* waveformWidgetRenderer)
    : WaveformRendererAbstract(waveformWidgetRenderer),
      m_pEndOfTrackControl(NULL),
      m_endOfTrackEnabled(false),
      m_pTrackSampleRate(NULL),
      m_pPlayControl(NULL),
      m_pLoopControl(NULL),
      m_pTimeRemainingControl(NULL),
      m_color(200, 25, 20),
      m_remainingTimeTriggerSeconds(30),
      m_blinkingPeriodMillis(1000) {
}

WaveformRendererEndOfTrack::~WaveformRendererEndOfTrack() {
    delete m_pEndOfTrackControl;
    delete m_pTrackSampleRate;
    delete m_pPlayControl;
    delete m_pLoopControl;
    delete m_pTimeRemainingControl;
}

bool WaveformRendererEndOfTrack::init() {
    m_timer.restart();

    m_pEndOfTrackControl = new ControlProxy(
            m_waveformRenderer->getGroup(), "end_of_track");
    m_pEndOfTrackControl->slotSet(0.);
    m_endOfTrackEnabled = false;

    m_pTrackSampleRate = new ControlProxy(
            m_waveformRenderer->getGroup(), "track_samplerate");
    m_pPlayControl = new ControlProxy(
            m_waveformRenderer->getGroup(), "play");
    m_pLoopControl = new ControlProxy(
            m_waveformRenderer->getGroup(), "loop_enabled");
    m_pTimeRemainingControl = new ControlProxy(
            m_waveformRenderer->getGroup(), "time_remaining");
    return true;
}

void WaveformRendererEndOfTrack::setup(const QDomNode& node, const SkinContext& context) {
    m_color = QColor(200, 25, 20);
    const QString endOfTrackColorName = context.selectString(node, "EndOfTrackColor");
    if (!endOfTrackColorName.isNull()) {
        m_color.setNamedColor(endOfTrackColorName);
        m_color = WSkinColor::getCorrectColor(m_color);
    }
    m_pen = QPen(QBrush(m_color), 2.5 * scaleFactor());
    generateBackRects();
}

void WaveformRendererEndOfTrack::onResize() {
    generateBackRects();
}

void WaveformRendererEndOfTrack::draw(QPainter* painter,
                                      QPaintEvent* /*event*/) {

    const double trackSamples = m_waveformRenderer->getTrackSamples();
    const double sampleRate = m_pTrackSampleRate->get();
    /*qDebug() << "WaveformRendererEndOfTrack :: "
             << "trackSamples" << trackSamples
             << "sampleRate" << sampleRate
             << "m_playControl->get()" << m_playControl->get()
             << "m_loopControl->get()" << m_loopControl->get();*/

    m_endOfTrackEnabled = m_pEndOfTrackControl->toBool();
    m_remainingTimeTriggerSeconds = WaveformWidgetFactory::instance()->getEndOfTrackWarningTime();
    // special case of track not long enough
    const double trackLength = 0.5 * trackSamples / sampleRate;

    if (sampleRate < 0.1 //not ready
            || trackSamples < 0.1 //not ready
            || !m_pPlayControl->toBool() //not playing
            || m_pLoopControl->toBool() //in loop
            || trackLength <= m_remainingTimeTriggerSeconds //track too short
            ) {
        if (m_endOfTrackEnabled) {
            m_pEndOfTrackControl->set(0.0);
            m_endOfTrackEnabled = false;
        }
        return;
    }

    const double remainingTime = m_pTimeRemainingControl->get();

    if (remainingTime > m_remainingTimeTriggerSeconds) {
        if (m_endOfTrackEnabled) {
            m_pEndOfTrackControl->set(0.);
            m_endOfTrackEnabled = false;
        }
        return;
    }

    // end of track is on
    if (!m_endOfTrackEnabled) {
        m_pEndOfTrackControl->set(1.);
        m_endOfTrackEnabled = true;

        //qDebug() << "EndOfTrack ON";
    }

    //ScopedTimer t("WaveformRendererEndOfTrack::draw");

    const int elapsed = m_timer.elapsed().toIntegerMillis() % m_blinkingPeriodMillis;

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
    // This is significant slower
    //painter->setOpacity(0.5 * criticalIntensity * blickIntensity);
    //painter->fillRect(m_waveformRenderer->getWidth()/2, 1,
    //        m_waveformRenderer->getWidth() - 2, m_waveformRenderer->getHeight() - 2,
    //        m_gradient);
    painter->restore();
}

void WaveformRendererEndOfTrack::generateBackRects() {
    m_backRects.resize(4);
    for (int i = 0; i < 4; i++) {
        if (m_waveformRenderer->getOrientation() == Qt::Vertical) {
            m_backRects[i].setTop(m_waveformRenderer->getHeight() / 2 +
                    i * m_waveformRenderer->getHeight() / 8);
            m_backRects[i].setBottom(m_waveformRenderer->getHeight());
            m_backRects[i].setLeft(0);
            m_backRects[i].setRight(m_waveformRenderer->getWidth());
        } else {
            m_backRects[i].setTop(0);
            m_backRects[i].setBottom(m_waveformRenderer->getHeight());
            m_backRects[i].setLeft(m_waveformRenderer->getWidth() / 2 +
                    i * m_waveformRenderer->getWidth() / 8);
            m_backRects[i].setRight(m_waveformRenderer->getWidth());
        }
    }
    // This is significant slower
    //m_gradient.setStart(m_waveformRenderer->getWidth() / 2, 0);
    //m_gradient.setFinalStop(m_waveformRenderer->getWidth(), 0);
    //m_gradient.setColorAt(0, Qt::transparent);
    //m_gradient.setColorAt(1, m_color);
}
