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

namespace {

constexpr int kBlinkingPeriodMillis = 1000;

} // anonymous namespace

WaveformRendererEndOfTrack::WaveformRendererEndOfTrack(
        WaveformWidgetRenderer* waveformWidgetRenderer)
    : WaveformRendererAbstract(waveformWidgetRenderer),
      m_pEndOfTrackControl(nullptr),
      m_pTimeRemainingControl(nullptr) {
}

WaveformRendererEndOfTrack::~WaveformRendererEndOfTrack() {
    delete m_pEndOfTrackControl;
    delete m_pTimeRemainingControl;
}

bool WaveformRendererEndOfTrack::init() {
    m_timer.restart();

    m_pEndOfTrackControl = new ControlProxy(
            m_waveformRenderer->getGroup(), "end_of_track");
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
    if (!m_pEndOfTrackControl->toBool()) {
        return;
    }

    //ScopedTimer t("WaveformRendererEndOfTrack::draw");

    const int elapsed = m_timer.elapsed().toIntegerMillis() % kBlinkingPeriodMillis;

    const double blinkIntensity = (double)(2 * abs(elapsed - kBlinkingPeriodMillis / 2)) /
            kBlinkingPeriodMillis;

    const double remainingTime = m_pTimeRemainingControl->get();
    const double remainingTimeTriggerSeconds = WaveformWidgetFactory::instance()->getEndOfTrackWarningTime();
    const double criticalIntensity = (remainingTimeTriggerSeconds - remainingTime) /
            remainingTimeTriggerSeconds;

    painter->save();
    painter->resetTransform();
    painter->setOpacity(0.5 * blinkIntensity);
    painter->setPen(m_pen);
    painter->drawRect(1, 1,
            m_waveformRenderer->getWidth() - 2, m_waveformRenderer->getHeight() - 2);

    painter->setOpacity(0.5 * 0.25 * criticalIntensity * blinkIntensity);
    painter->setPen(QPen(Qt::transparent));
    painter->setBrush(m_color);
    painter->drawRects(m_backRects);
    // This is significant slower
    //painter->setOpacity(0.5 * criticalIntensity * blinkIntensity);
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
