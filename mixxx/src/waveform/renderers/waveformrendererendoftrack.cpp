#include <QDomNode>
#include <QPaintEvent>
#include <QPainter>

#include "waveformrendererendoftrack.h"

#include "defs.h"
#include "waveformwidgetrenderer.h"

WaveformRendererEndOfTrack::WaveformRendererEndOfTrack(
    WaveformWidgetRenderer* waveformWidgetRenderer)
        : WaveformRendererAbstract(waveformWidgetRenderer),
          m_color(200, 25, 20),
          m_blinkingPeriodMillis(1000),
          m_remainingTimeTriggerSeconds(30.0) {
}

WaveformRendererEndOfTrack::~WaveformRendererEndOfTrack() {
}

void WaveformRendererEndOfTrack::init() {
    m_timer.restart();
}

void WaveformRendererEndOfTrack::setup(const QDomNode& /*node*/) {
    // TODO(vrince): add EnfOfTrack color in skins and why not blinking period
    // too
    setDirty(true);
}

void WaveformRendererEndOfTrack::draw(QPainter* painter,
                                      QPaintEvent* /*event*/) {
    if (isDirty()) {
        generatePixmap();
    }

    TrackPointer pTrack = m_waveformRenderer->getTrackInfo();
    if (!pTrack) {
        return;
    }

    double trackSamples = m_waveformRenderer->getTrackSamples();
    // TODO(rryan) WARNING NOT ACCURATE! Should use track_samplerate CO
    double sampleRate = pTrack->getSampleRate();

    if (sampleRate == 0.0 || trackSamples == 0.0) {
        return;
    }

    double dPlaypos = m_waveformRenderer->getPlayPos();
    double remainingFrames = (1.0 - dPlaypos) * 0.5 * trackSamples;
    double remainingTime = remainingFrames / sampleRate;

    if (remainingTime > m_remainingTimeTriggerSeconds) {
        return;
    }

    // TODO(vRince): add some logic about direction, play/stop, loop ?
    // NOTE(vRince): why don't we use a nice QAnimation ?
    int elapsed = m_timer.elapsed() % m_blinkingPeriodMillis;
    int index = s_maxAlpha *
            static_cast<double>(2*abs(elapsed - m_blinkingPeriodMillis/2)) /
            m_blinkingPeriodMillis;
    index = math_min(s_maxAlpha - 1, math_max(0, index));

    int xPos = m_waveformRenderer->getWidth()-m_pixmaps[index].width();
    painter->drawPixmap(QPoint(xPos, 0), m_pixmaps[index]);
}

void WaveformRendererEndOfTrack::generatePixmap() {
    for (int i = 0; i < s_maxAlpha; ++i) {
        m_pixmaps[i] = QPixmap(m_waveformRenderer->getWidth()/4,
                               m_waveformRenderer->getHeight());
        m_pixmaps[i].fill(QColor(0, 0, 0, 0));

        QColor startColor = m_color;
        startColor.setAlpha(0);
        QColor endcolor = m_color;

        QRadialGradient gradBackground(
            QPointF(1.5*m_pixmaps[i].width(),
                    m_pixmaps[i].height()/2),
            1.5*m_pixmaps[i].width());
        endcolor.setAlpha(i);
        gradBackground.setColorAt(1.0, startColor);
        gradBackground.setColorAt(0.0, endcolor);

        QLinearGradient linearGradBorder(
            QPointF(0, 0), QPointF(m_pixmaps[i].width(), 0));
        linearGradBorder.setColorAt(0.0, startColor);
        linearGradBorder.setColorAt(1.0, endcolor);

        QBrush brush(gradBackground);
        QPen pen(linearGradBorder, 2.0);

        QRectF rectangle(2.5, 2.5,
                         m_pixmaps[i].width() - 5,
                         m_pixmaps[i].height() - 5);
        QPainter painter;
        painter.begin(&m_pixmaps[i]);
        painter.setRenderHint(QPainter::HighQualityAntialiasing, true);
        painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
        painter.setBrush(brush);
        painter.setPen(pen);
        painter.drawRoundedRect(rectangle, 9.5, 9.5);
    }
    setDirty(false);
}
