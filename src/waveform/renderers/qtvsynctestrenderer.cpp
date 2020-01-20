#include "waveform/renderers/qtvsynctestrenderer.h"

#include "track/track.h"
#include "util/painterscope.h"
#include "util/performancetimer.h"
#include "waveform/renderers/waveformwidgetrenderer.h"
#include "waveform/waveform.h"
#include "waveform/waveformwidgetfactory.h"

QtVSyncTestRenderer::QtVSyncTestRenderer(
        WaveformWidgetRenderer* waveformWidgetRenderer)
    : WaveformRendererSignalBase(waveformWidgetRenderer),
      m_drawcount(0) {
}

QtVSyncTestRenderer::~QtVSyncTestRenderer() {
}

void QtVSyncTestRenderer::onSetup(const QDomNode& node) {
    Q_UNUSED(node);
}

inline void setPoint(QPointF& point, qreal x, qreal y) {
    point.setX(x);
    point.setY(y);
}

void QtVSyncTestRenderer::draw(QPainter* pPainter, QPaintEvent* /*event*/) {

    PerformanceTimer timer;
    //mixxx::Duration t5, t6, t7, t8, t9, t10, t11, t12, t13;


    timer.start();

    TrackPointer pTrack = m_waveformRenderer->getTrackInfo();
    if (!pTrack) {
        return;
    }

    ConstWaveformPointer waveform = pTrack->getWaveform();
    if (waveform.isNull()) {
        return;
    }

    const int dataSize = waveform->getDataSize();
    if (dataSize <= 1) {
        return;
    }

    const WaveformData* data = waveform->data();
    if (data == nullptr) {
        return;
    }

    PainterScope PainterScope(pPainter);

    auto brush = QBrush(Qt::SolidPattern);
    if (++m_drawcount & 1) {
        brush.setColor(QColor(255, 255, 255));
    } else {
        brush.setColor(QColor(255, 0, 0));
    }

    pPainter->setBrush(brush);

    pPainter->drawRect(0, 0, m_waveformRenderer->getWidth(),
            m_waveformRenderer->getHeight());
}
