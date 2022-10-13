#include "waveform/widgets/qopengl/waveformwidget.h"

#include "waveform/renderers/qopengl/iwaveformrenderer.h"
#include "widget/wwaveformviewer.h"

using namespace qopengl;

WaveformWidget::WaveformWidget(const QString& group, QWidget* parent)
        : WGLWidget(parent), WaveformWidgetAbstract(group) {
}

WaveformWidget::~WaveformWidget() {
    makeCurrentIfNeeded();
    for (int i = 0; i < m_rendererStack.size(); ++i) {
        delete m_rendererStack[i];
    }
    m_rendererStack.clear();
    doneCurrent();
}

void WaveformWidget::renderGL() {
    makeCurrentIfNeeded();

    if (shouldOnlyDrawBackground()) {
        if (!m_rendererStack.empty()) {
            m_rendererStack[0]->qopenglWaveformRenderer()->renderGL();
        }
    } else {
        for (int i = 0; i < m_rendererStack.size(); ++i) {
            m_rendererStack[i]->qopenglWaveformRenderer()->renderGL();
        }
    }
    doneCurrent();
}

void WaveformWidget::initializeGL() {
    makeCurrentIfNeeded();
    for (int i = 0; i < m_rendererStack.size(); ++i) {
        m_rendererStack[i]->qopenglWaveformRenderer()->initializeOpenGLFunctions();
        m_rendererStack[i]->qopenglWaveformRenderer()->initializeGL();
    }
    doneCurrent();
}

void WaveformWidget::handleEventFromWindow(QEvent* ev) {
    auto viewer = dynamic_cast<WWaveformViewer*>(parent());
    if (viewer) {
        viewer->handleEventFromWindow(ev);
    }
}
