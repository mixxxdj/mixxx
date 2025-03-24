#include "waveform/renderers/allshader/waveformrenderersignalbase.h"

#include "util/colorcomponents.h"

namespace allshader {

WaveformRendererSignalBase::WaveformRendererSignalBase(
        WaveformWidgetRenderer* waveformWidget)
        : ::WaveformRendererSignalBase(waveformWidget),
          m_ignoreStem(false) {
}

void WaveformRendererSignalBase::draw(QPainter*, QPaintEvent*) {
    DEBUG_ASSERT(false);
}

void WaveformRendererSignalBase::setAxesColor(const QColor& axesColor) {
    getRgbF(axesColor, &m_axesColor_r, &m_axesColor_g, &m_axesColor_b, &m_axesColor_a);
}

void WaveformRendererSignalBase::setColor(const QColor& color) {
    getRgbF(color, &m_signalColor_r, &m_signalColor_g, &m_signalColor_b);
    getHsvF(color, &m_signalColor_h, &m_signalColor_s, &m_signalColor_v);
}

void WaveformRendererSignalBase::setLowColor(const QColor& lowColor) {
    getRgbF(lowColor, &m_rgbLowColor_r, &m_rgbLowColor_g, &m_rgbLowColor_b);
    getRgbF(lowColor, &m_lowColor_r, &m_lowColor_g, &m_lowColor_b);
}

void WaveformRendererSignalBase::setMidColor(const QColor& midColor) {
    getRgbF(midColor, &m_rgbMidColor_r, &m_rgbMidColor_g, &m_rgbMidColor_b);
    getRgbF(midColor, &m_midColor_r, &m_midColor_g, &m_midColor_b);
}

void WaveformRendererSignalBase::setHighColor(const QColor& highColor) {
    getRgbF(highColor, &m_rgbHighColor_r, &m_rgbHighColor_g, &m_rgbHighColor_b);
    getRgbF(highColor, &m_highColor_r, &m_highColor_g, &m_highColor_b);
}

} // namespace allshader
