#pragma once

#include <QFlags>
#include <array>
// required for Qt-Macros
#include <qobjectdefs.h>

#include "waveform/waveformanalysisprofile.h"

class WaveformWidgetType {
  public:
    enum Type {
        // The order must not be changed because the waveforms are referenced
        // from the sorted preferences by a number.
        Empty = 0,
        Simple = 5,    // 5  Simple GL
        Filtered = 7,  // 7  Filtered GLSL
        HSV = 8,       // 8  HSV
        VSyncTest = 9, // 9  VSync GL
        RGB = 12,      // 12 RGB GLSL
        Stacked = 16,  // 16 RGB Stacked
        Perceptual3Band = 17,
        Invalid,       // Don't use! Used to indicate invalid/unknown type, as
                       // Count_WaveformWidgetType used to.
    };
    static constexpr std::array kValues = {
            WaveformWidgetType::Empty,
            WaveformWidgetType::Simple,
            WaveformWidgetType::Filtered,
            WaveformWidgetType::HSV,
            WaveformWidgetType::VSyncTest,
            WaveformWidgetType::RGB,
            WaveformWidgetType::Stacked,
            WaveformWidgetType::Perceptual3Band,
    };

    static constexpr mixxx::WaveformAnalysisProfile analysisProfile(Type type) {
        return type == Perceptual3Band
                ? mixxx::WaveformAnalysisProfile::Perceptual3Band
                : mixxx::WaveformAnalysisProfile::Legacy;
    }
};

enum class WaveformWidgetBackend {
    None = 0,
    GL,
    GLSL,
#ifdef MIXXX_USE_QOPENGL
    AllShader,
#endif
};
