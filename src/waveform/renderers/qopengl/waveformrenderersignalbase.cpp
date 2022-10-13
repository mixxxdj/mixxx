#include "waveform/renderers/qopengl/waveformrenderersignalbase.h"

#include "waveform/widgets/qopengl/waveformwidget.h"

using namespace qopengl;

qopengl::WaveformRendererSignalBase::WaveformRendererSignalBase(
        WaveformWidgetRenderer* waveformWidget)
        : ::WaveformRendererSignalBase(waveformWidget) {
}
