#include "waveform/widgets/allshader/waveformwidget.h"

#include <QApplication>
#include <QWheelEvent>

#include "moc_waveformwidget.cpp"
#include "waveform/renderers/allshader/waveformrendererabstract.h"
#include "waveform/renderers/waveformrendererabstract.h"

namespace allshader {

WaveformWidget::WaveformWidget(const QString& group, QWidget* parent)
        : WGLWidget(parent), WaveformWidgetAbstract(group) {
}

WaveformWidget::~WaveformWidget() {
    makeCurrentIfNeeded();
    for (auto* pRenderer : std::as_const(m_rendererStack)) {
        delete pRenderer;
    }
    m_rendererStack.clear();
    doneCurrent();
}

mixxx::Duration WaveformWidget::render() {
    makeCurrentIfNeeded();
    paintGL();
    doneCurrent();
    // In the legacy widgets, this is used to "return timer for painter setup"
    // which is not relevant here. Also note that the return value is not used
    // at all, so it might be better to remove it everywhere. In the meantime.
    // we need to return something for API compatibility.
    return mixxx::Duration();
}

void WaveformWidget::paintGL() {
    if (shouldOnlyDrawBackground()) {
        if (!m_rendererStack.empty()) {
            m_rendererStack[0]->allshaderWaveformRenderer()->paintGL();
        }
    } else {
        for (auto* pRenderer : std::as_const(m_rendererStack)) {
            pRenderer->allshaderWaveformRenderer()->paintGL();
        }
    }
}

void WaveformWidget::initializeGL() {
    for (auto* pRenderer : std::as_const(m_rendererStack)) {
        pRenderer->allshaderWaveformRenderer()->initializeGL();
    }
}

void WaveformWidget::resizeGL(int w, int h) {
    for (auto* pRenderer : std::as_const(m_rendererStack)) {
        pRenderer->allshaderWaveformRenderer()->resizeGL(w, h);
    }
}

void WaveformWidget::wheelEvent(QWheelEvent* pEvent) {
    QApplication::sendEvent(parentWidget(), pEvent);
    pEvent->accept();
}

void WaveformWidget::leaveEvent(QEvent* pEvent) {
    QApplication::sendEvent(parentWidget(), pEvent);
    pEvent->accept();
}

} // namespace allshader
