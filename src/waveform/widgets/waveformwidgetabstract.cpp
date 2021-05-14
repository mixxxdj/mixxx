#include "waveformwidgetabstract.h"

#include <QWidget>
#include <QtDebug>

#include "util/compatibility.h"
#include "waveform/renderers/waveformwidgetrenderer.h"

WaveformWidgetAbstract::WaveformWidgetAbstract(const QString& group)
        : WaveformWidgetRenderer(group),
          m_initSuccess(false) {
    m_widget = nullptr;
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

void WaveformWidgetAbstract::resize(int width, int height) {
    qreal devicePixelRatio = 1.0;
    if (m_widget) {
        m_widget->resize(width, height);
        devicePixelRatio = getDevicePixelRatioF(m_widget);
    }
    WaveformWidgetRenderer::resize(width, height, static_cast<float>(devicePixelRatio));
}
