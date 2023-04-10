#include "waveform/widgets/qopengl/lrrgbwaveformwidget.h"

#include "waveform/renderers/qopengl/waveformrenderbackground.h"
#include "waveform/renderers/qopengl/waveformrenderbeat.h"
#include "waveform/renderers/qopengl/waveformrendererendoftrack.h"
#include "waveform/renderers/qopengl/waveformrendererlrrgb.h"
#include "waveform/renderers/qopengl/waveformrendererpreroll.h"
#include "waveform/renderers/qopengl/waveformrendermark.h"
#include "waveform/renderers/qopengl/waveformrendermarkrange.h"
#include "waveform/widgets/qopengl/moc_lrrgbwaveformwidget.cpp"

using namespace qopengl;

LRRGBWaveformWidget::LRRGBWaveformWidget(const QString& group, QWidget* parent)
        : WaveformWidget(group, parent) {
    addRenderer<WaveformRenderBackground>();
    addRenderer<WaveformRendererEndOfTrack>();
    addRenderer<WaveformRendererPreroll>();
    addRenderer<WaveformRenderMarkRange>();
    addRenderer<WaveformRendererLRRGB>();
    addRenderer<WaveformRenderBeat>();
    addRenderer<WaveformRenderMark>();

    m_initSuccess = init();
}

LRRGBWaveformWidget::~LRRGBWaveformWidget() {
}

void LRRGBWaveformWidget::castToQWidget() {
    m_widget = this;
}

void LRRGBWaveformWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
}
