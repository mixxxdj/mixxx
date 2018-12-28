#include "waveformwidgetabstract.h"
#include "waveform/renderers/waveformwidgetrenderer.h"

#include <QtDebug>
#include <QWidget>


WaveformWidgetAbstract::WaveformWidgetAbstract(const char* group)
    : WaveformWidgetRenderer(group),
      m_widget(nullptr),
      m_initSuccess(false) {
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

void WaveformWidgetAbstract::preRender(VSyncThread* vsyncThread) {
    WaveformWidgetRenderer::onPreRender(vsyncThread);
}

mixxx::Duration WaveformWidgetAbstract::render() {
    if (m_widget) {
        m_widget->repaint(); // Repaints the widget directly by calling paintEvent()
    }
    // Time for Painter setup, unknown in this case
    return mixxx::Duration();
}

void WaveformWidgetAbstract::resize(int width, int height, float devicePixelRatio) {
    if (m_widget) {
        m_widget->resize(width, height);
    }
    WaveformWidgetRenderer::resize(width, height, devicePixelRatio);
}
