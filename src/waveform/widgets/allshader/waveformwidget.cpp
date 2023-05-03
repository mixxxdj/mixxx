#include "waveform/widgets/allshader/waveformwidget.h"

#include "waveform/renderers/allshader/waveformrendererabstract.h"

using namespace allshader;

WaveformWidget::WaveformWidget(const QString& group, QWidget* parent)
        : WGLWidget(parent), WaveformWidgetAbstract(group) {
}

WaveformWidget::~WaveformWidget() {
    makeCurrentIfNeeded();
    for (auto renderer : std::as_const(m_rendererStack)) {
        delete renderer;
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
        for (auto renderer : std::as_const(m_rendererStack)) {
            renderer->allshaderWaveformRenderer()->paintGL();
        }
    }
}

void WaveformWidget::initializeGL() {
    for (auto renderer : std::as_const(m_rendererStack)) {
        renderer->allshaderWaveformRenderer()->initializeGL();
    }
}

void WaveformWidget::resizeGL(int w, int h) {
    makeCurrentIfNeeded();
    for (auto renderer : std::as_const(m_rendererStack)) {
        renderer->allshaderWaveformRenderer()->resizeGL(w, h);
    }
    doneCurrent();
}
