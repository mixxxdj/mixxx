#pragma once

class WaveformWidgetType {
  public:
    enum Type {
        // The order must not be changed because the waveforms are referenced
        // from the sorted preferences by a number.
        EmptyWaveform = 0,
        SoftwareSimpleWaveform, //TODO
#ifndef __APPLE__
        // Don't offer the simple renderers on macOS, they do not work with skins
        // that load GL widgets (spinnies, waveforms) in singletons.
        // Also excluded in enum WaveformWidgetType
        // https://bugs.launchpad.net/bugs/1928772
        SoftwareWaveform, // Filtered
#endif
        QtSimpleWaveform,     // Simple Qt
        QtWaveform,           // Filtered Qt
        GLSimpleWaveform,     // Simple GL
        GLFilteredWaveform,   // Filtered GL
        GLSLFilteredWaveform, // Filtered GLSL
#ifndef __APPLE__
        HSVWaveform, // HSV
#endif
        GLVSyncTest, // VSync GL
#ifndef __APPLE__
        RGBWaveform, // RGB
#endif
        GLRGBWaveform,           // RGB GL
        GLSLRGBWaveform,         // RGB GLSL
        QtVSyncTest,             // VSync Qt
        QtHSVWaveform,           // HSV Qt
        QtRGBWaveform,           // RGB Qt
        Count_WaveformwidgetType // Also used as invalid value
    };
};
