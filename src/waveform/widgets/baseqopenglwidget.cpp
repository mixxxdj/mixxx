#include "waveform/widgets/baseqopenglwidget.h"

#include "util/time.h"

BaseQOpenGLWidget::BaseQOpenGLWidget(const char* group, QWidget* pParent)
        : QOpenGLWidget(pParent),
          WaveformWidgetAbstract(group),
          m_shouldRenderOnNextTick(false) {
    connect(this, &QOpenGLWidget::aboutToCompose,
            this, &BaseQOpenGLWidget::slotAboutToCompose);
    connect(this, &QOpenGLWidget::frameSwapped,
            this, &BaseQOpenGLWidget::slotFrameSwapped);
}

void BaseQOpenGLWidget::slotAboutToCompose() {
    if (m_shouldRenderOnNextTick) {
        m_lastRender = mixxx::Time::elapsed();
        preRender(m_lastSwapDurationMovingAverage);
        render();
        m_shouldRenderOnNextTick = false;
    }
}

void BaseQOpenGLWidget::slotFrameSwapped() {
    if (m_lastRender != m_lastSwapRender) {
        m_lastSwapRender = m_lastRender;
        m_lastSwapDuration = mixxx::Time::elapsed() - m_lastRender;
        const double decay = 0.5;
        m_lastSwapDurationMovingAverage = mixxx::Duration::fromNanos(
            decay * m_lastSwapDurationMovingAverage.toDoubleNanos() +
            (1.0 - decay) * m_lastSwapDuration.toDoubleNanos());
    }
}
