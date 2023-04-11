#pragma once

#include <QMatrix4x4>

class WaveformWidgetRenderer;

QMatrix4x4 calculateMatrix(WaveformWidgetRenderer* const widget, bool applyDevicePixelRatio);
