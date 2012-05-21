#include "waveformwidgetabstract.h"
#include "waveform/renderers/waveformwidgetrenderer.h"

#include <QtDebug>
#include <QWidget>

// Default constructor is only use by the factory to evaluate dynamically
// WaveformWidget
WaveformWidgetAbstract::WaveformWidgetAbstract() :
    WaveformWidgetRenderer() {
    m_widget = NULL;
}

WaveformWidgetAbstract::WaveformWidgetAbstract( const char* group) :
    WaveformWidgetRenderer(group) {
    m_widget = NULL;
}

WaveformWidgetAbstract::~WaveformWidgetAbstract() {
}

void WaveformWidgetAbstract::hold() {
    if (m_widget) {
        m_widget->hide();
    }
}

void WaveformWidgetAbstract::release() {
    if (m_widget) {
        m_widget->show();
    }
}

void WaveformWidgetAbstract::preRender() {
    WaveformWidgetRenderer::onPreRender();
}

void WaveformWidgetAbstract::render() {
    if (m_widget) {
        if (!m_widget->isVisible()) {
            m_widget->show();
        }
        m_widget->update();
    }
}

void WaveformWidgetAbstract::resize( int width, int height) {
    if (m_widget) {
        m_widget->resize(width, height);
    }
    WaveformWidgetRenderer::resize(width, height);
}
