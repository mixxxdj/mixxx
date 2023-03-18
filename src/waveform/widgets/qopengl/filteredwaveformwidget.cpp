#include "waveform/widgets/qopengl/filteredwaveformwidget.h"

#include "waveform/renderers/qopengl/waveformrenderbackground.h"
#include "waveform/renderers/qopengl/waveformrenderbeat.h"
#include "waveform/renderers/qopengl/waveformrendererendoftrack.h"
#include "waveform/renderers/qopengl/waveformrendererfiltered.h"
#include "waveform/renderers/qopengl/waveformrendererpreroll.h"
#include "waveform/renderers/qopengl/waveformrendermark.h"
#include "waveform/renderers/qopengl/waveformrendermarkrange.h"
#include "waveform/widgets/qopengl/moc_filteredwaveformwidget.cpp"

using namespace qopengl;

FilteredWaveformWidget::FilteredWaveformWidget(const QString& group, QWidget* parent)
        : WaveformWidget(group, parent) {
    addRenderer<WaveformRenderBackground>();
    addRenderer<WaveformRendererEndOfTrack>();
    addRenderer<WaveformRendererPreroll>();
    addRenderer<WaveformRenderMarkRange>();
    addRenderer<WaveformRendererFiltered>();
    addRenderer<WaveformRenderBeat>();
    addRenderer<WaveformRenderMark>();

    m_initSuccess = init();
}

FilteredWaveformWidget::~FilteredWaveformWidget() {
}

void FilteredWaveformWidget::castToQWidget() {
    m_widget = this;
}

void FilteredWaveformWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
}
