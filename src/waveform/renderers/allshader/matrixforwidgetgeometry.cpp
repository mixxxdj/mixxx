#include "waveform/renderers/allshader/matrixforwidgetgeometry.h"

#include "waveform/renderers/waveformwidgetrenderer.h"

QMatrix4x4 matrixForWidgetGeometry(WaveformWidgetRenderer* const waveformRenderer,
        bool applyDevicePixelRatio) {
    const qreal ratio = applyDevicePixelRatio ? waveformRenderer->getDevicePixelRatio() : 1.0;
    QMatrix4x4 matrix;

    matrix.ortho(QRectF(0.0,
            0.0,
            waveformRenderer->getWidth() * ratio,
            waveformRenderer->getHeight() * ratio));
    if (waveformRenderer->getOrientation() == Qt::Vertical) {
        matrix.rotate(90.f, 0.0f, 0.0f, 1.0f);
        matrix.translate(0.f, -waveformRenderer->getWidth() * static_cast<float>(ratio), 0.f);
    }
    return matrix;
}
