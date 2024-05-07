#pragma once

class WaveformWidgetType {
  public:
    enum Type {
        // The order must not be changed because the waveforms are referenced
        // from the sorted preferences by a number.
        Empty = 0,
        Simple,                  // 5  Simple GL
        Filtered,                // 7  Filtered GLSL
        HSV,                     // 8  HSV
        VSyncTest,               // 9  VSync GL
        RGB,                     // 12 RGB GLSL
        Stacked,                 // 16 RGB Stacked
        Count_WaveformWidgetType //    Also used as invalid value
    };
};

class WaveformWidgetBackend {
  public:
    enum Backend {
        None = 0,
        GL,
        GLSL,
#ifdef MIXXX_USE_QOPENGL
        AllShader,
#endif
        Count_WaveformWidgetBackend
    };
};
