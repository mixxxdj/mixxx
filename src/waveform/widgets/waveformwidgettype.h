#pragma once

class WaveformWidgetType {
  public:
    enum Type {
        // The order must not be changed because the waveforms are referenced
        // from the sorted preferences by a number.
        // The skipped numbers were legacy types that have been removed.
        EmptyWaveform = 0,
        SoftwareWaveform = 2,
        QtWaveform = 4,
        GLFilteredWaveform = 6,
        GLSLFilteredWaveform = 7,
        GLVSyncTest = 9,
        RGBWaveform = 10,
        GLRGBWaveform = 11,
        GLSLRGBWaveform = 12,
        QtVSyncTest = 13,
        QtRGBWaveform = 15,
        GLSLRGBStackedWaveform = 16,
        Count_WaveformwidgetType // Also used as invalid value
    };
};
