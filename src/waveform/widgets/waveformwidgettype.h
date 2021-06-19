#pragma once

class WaveformWidgetType {
  public:
    enum Type {
        // The order must not be changed because the waveforms are referenced
        // from the sorted preferences by a number.
        EmptyWaveform = 0,
        SoftwareSimpleWaveform,  // 1  TODO
        SoftwareWaveform,        // 2  Filtered
        QtSimpleWaveform,        // 3  Simple Qt
        QtWaveform,              // 4  Filtered Qt
        GLSimpleWaveform,        // 5  Simple GL
        GLFilteredWaveform,      // 6  Filtered GL
        GLSLFilteredWaveform,    // 7  Filtered GLSL
        HSVWaveform,             // 8  HSV
        GLVSyncTest,             // 9  VSync GL
        RGBWaveform,             // 10 RGB
        GLRGBWaveform,           // 11 RGB GL
        GLSLRGBWaveform,         // 12 RGB GLSL
        QtVSyncTest,             // 13 VSync Qt
        QtHSVWaveform,           // 14 HSV Qt
        QtRGBWaveform,           // 15 RGB Qt
        GLSLRGBStackedWaveform,  // 16 RGB Stacked
        Count_WaveformwidgetType // 17 Also used as invalid value
    };
};
