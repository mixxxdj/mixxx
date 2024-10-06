#include "waveform/renderers/allshader/matrixforwidgetgeometry.h"

#include "waveform/renderers/waveformwidgetrenderer.h"

QMatrix4x4 matrixForWidgetGeometry(WaveformWidgetRenderer* const waveformRenderer,
        bool applyDevicePixelRatio) {
    return waveformRenderer->getMatrix(applyDevicePixelRatio);
}
