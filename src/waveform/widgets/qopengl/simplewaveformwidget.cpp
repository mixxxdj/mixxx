#include "waveform/widgets/qopengl/simplewaveformwidget.h"

#include "waveform/renderers/qopengl/waveformrenderbackground.h"
#include "waveform/renderers/qopengl/waveformrenderbeat.h"
#include "waveform/renderers/qopengl/waveformrendererendoftrack.h"
#include "waveform/renderers/qopengl/waveformrendererpreroll.h"
#include "waveform/renderers/qopengl/waveformrenderersimple.h"
#include "waveform/renderers/qopengl/waveformrendermark.h"
#include "waveform/renderers/qopengl/waveformrendermarkrange.h"
#include "waveform/widgets/qopengl/moc_simplewaveformwidget.cpp"

using namespace qopengl;

SimpleWaveformWidget::SimpleWaveformWidget(const QString& group, QWidget* parent)
        : WaveformWidget(group, parent) {
    addRenderer<WaveformRenderBackground>();
    addRenderer<WaveformRendererEndOfTrack>();
    addRenderer<WaveformRendererPreroll>();
    addRenderer<WaveformRenderMarkRange>();
    addRenderer<WaveformRendererSimple>();
    addRenderer<WaveformRenderBeat>();
    addRenderer<WaveformRenderMark>();

    m_initSuccess = init();
}

SimpleWaveformWidget::~SimpleWaveformWidget() {
}

void SimpleWaveformWidget::castToQWidget() {
    m_widget = this;
}

void SimpleWaveformWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
}
