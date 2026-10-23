#include "waveform/widgets/waveformwidgetabstract.h"

#include <QWidget>

#include "waveform/renderers/waveformwidgetrenderer.h"
#include "waveform/vsyncthread.h"

WaveformWidgetAbstract::WaveformWidgetAbstract(const QString& group)
        : WaveformWidgetRenderer(group),
          m_initSuccess(false) {
    m_pWidget = nullptr;
}

WaveformWidgetAbstract::~WaveformWidgetAbstract() {
}

void WaveformWidgetAbstract::hold() {
    if (m_pWidget) {
        m_pWidget->hide();
    }
}

void WaveformWidgetAbstract::release() {
    if (m_pWidget) {
        m_pWidget->show();
    }
}

mixxx::Duration WaveformWidgetAbstract::render() {
    if (m_pWidget) {
        m_pWidget->repaint(); // Repaints the widget directly by calling paintEvent()
    }
    // Time for Painter setup, unknown in this case
    return mixxx::Duration();
}

void WaveformWidgetAbstract::resize(int width, int height) {
    if (m_pWidget) {
        qreal devicePixelRatio = m_pWidget->devicePixelRatioF();
        resizeRenderer(width, height, devicePixelRatio);
        m_pWidget->resize(width, height);
    }
}
