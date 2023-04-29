#include "waveform/widgets/qopengl/waveformwidget.h"

#include "waveform/renderers/qopengl/waveformrendererabstract.h"
#include "widget/wwaveformviewer.h"

using namespace qopengl;

WaveformWidget::WaveformWidget(const QString& group, QWidget* parent)
        : WGLWidget(parent), WaveformWidgetAbstract(group) {
}

WaveformWidget::~WaveformWidget() {
    makeCurrentIfNeeded();
    for (auto renderer : m_rendererStack) {
        delete renderer;
    }
    m_rendererStack.clear();
    doneCurrent();
}

mixxx::Duration WaveformWidget::render() {
    makeCurrentIfNeeded();
    renderGL();
    doneCurrent();
    // In the legacy widgets, this is used to "return timer for painter setup"
    // which is not relevant here. Also note that the return value is not used
    // at all, so it might be better to remove it everywhere. In the meantime.
    // we need to return something for API compatibility.
    return mixxx::Duration();
}

void WaveformWidget::renderGL() {
    if (shouldOnlyDrawBackground()) {
        if (!m_rendererStack.empty()) {
            m_rendererStack[0]->qopenglWaveformRenderer()->renderGL();
        }
    } else {
        for (auto renderer : m_rendererStack) {
            renderer->qopenglWaveformRenderer()->renderGL();
        }
    }
}

void WaveformWidget::initializeGL() {
    for (auto renderer : m_rendererStack) {
        renderer->qopenglWaveformRenderer()->initializeOpenGLFunctions();
        renderer->qopenglWaveformRenderer()->initializeGL();
    }
}

void WaveformWidget::resizeGL(int w, int h) {
    makeCurrentIfNeeded();
    for (auto renderer : m_rendererStack) {
        renderer->qopenglWaveformRenderer()->resizeGL(w, h);
    }
    doneCurrent();
}

void WaveformWidget::handleEventFromWindow(QEvent* ev) {
    auto viewer = dynamic_cast<WWaveformViewer*>(parent());
    if (viewer) {
        viewer->handleEventFromWindow(ev);
    }
}
