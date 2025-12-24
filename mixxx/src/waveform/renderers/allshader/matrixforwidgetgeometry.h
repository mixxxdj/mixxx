#pragma once

#include <QMatrix4x4>

class WaveformWidgetRenderer;

// Calculate the orthogonal project matrix for the WaveformWidgetRenderer, taking into account
// width and height, but also orientation (to rotate the matrix).
// Note that this allows us to basically use the same renderer code for horizontal and vertical
// orientation of the waveform, as we simply multiple the vertices with a rotated matrix in
// the vertex shaders.
QMatrix4x4 matrixForWidgetGeometry(
        WaveformWidgetRenderer* const widget, bool applyDevicePixelRatio);
