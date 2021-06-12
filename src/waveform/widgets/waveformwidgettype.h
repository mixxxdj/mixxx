#pragma once

class WaveformWidgetType {
  public:
    enum Type {
        // The order must not be changed because the waveforms are referenced
        // from the sorted preferences by a number.
        EmptyWaveform = 0,
        SoftwareSimpleWaveform,  //TODO
        SoftwareWaveform,        // Filtered
        QtSimpleWaveform,        // Simple Qt
        QtWaveform,              // Filtered Qt
        GLSimpleWaveform,        // Simple GL
        GLFilteredWaveform,      // Filtered GL
        GLSLFilteredWaveform,    // Filtered GLSL
        HSVWaveform,             // HSV
        GLVSyncTest,             // VSync GL
        RGBWaveform,             // RGB
        GLRGBWaveform,           // RGB GL
        GLSLRGBWaveform,         // RGB GLSL
        QtVSyncTest,             // VSync Qt
        QtHSVWaveform,           // HSV Qt
        QtRGBWaveform,           // RGB Qt
        Count_WaveformwidgetType // Also used as invalid value
    };
};
