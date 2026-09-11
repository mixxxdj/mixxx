#include "waveform/renderers/allshader/waveformrenderersignalbase.h"

namespace allshader {

WaveformRendererSignalBase::WaveformRendererSignalBase(
        WaveformWidgetRenderer* waveformWidget, ::WaveformRendererSignalBase::Options options)
        : ::WaveformRendererSignalBase(waveformWidget, options),
          m_ignoreStem(false) {
}

void WaveformRendererSignalBase::draw(QPainter*, QPaintEvent*) {
    DEBUG_ASSERT(false);
}

void WaveformRendererSignalBase::setAxesColor(const QColor& axesColor) {
    axesColor.getRgbF(&m_axesColor_r, &m_axesColor_g, &m_axesColor_b, &m_axesColor_a);
}

void WaveformRendererSignalBase::setColor(const QColor& color) {
    color.getRgbF(&m_signalColor_r, &m_signalColor_g, &m_signalColor_b);
    color.getHsvF(&m_signalColor_h, &m_signalColor_s, &m_signalColor_v);
}

void WaveformRendererSignalBase::setLowColor(const QColor& lowColor) {
    lowColor.getRgbF(&m_rgbLowColor_r, &m_rgbLowColor_g, &m_rgbLowColor_b);
    lowColor.getRgbF(&m_lowColor_r, &m_lowColor_g, &m_lowColor_b);
}

void WaveformRendererSignalBase::setMidColor(const QColor& midColor) {
    midColor.getRgbF(&m_rgbMidColor_r, &m_rgbMidColor_g, &m_rgbMidColor_b);
    midColor.getRgbF(&m_midColor_r, &m_midColor_g, &m_midColor_b);
}

void WaveformRendererSignalBase::setHighColor(const QColor& highColor) {
    highColor.getRgbF(&m_rgbHighColor_r, &m_rgbHighColor_g, &m_rgbHighColor_b);
    highColor.getRgbF(&m_highColor_r, &m_highColor_g, &m_highColor_b);
}

} // namespace allshader
