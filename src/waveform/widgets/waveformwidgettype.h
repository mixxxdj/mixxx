#ifndef WAVEFORMWIDGETTYPE_H
#define WAVEFORMWIDGETTYPE_H

class WaveformWidgetType {
  public:
    enum Type {
        // The order must not be changed because the waveforms are referenced
        // from the sorted preferences by a number.
        EmptyWaveform = 0,
        SoftwareSimpleWaveform, //TODO
        SoftwareWaveform,
        QtSimpleWaveform,
        QtWaveform,
        GLSimpleWaveform,
        GLFilteredWaveform,
        GLSLFilteredWaveform,
        HSVWaveform,
        GLVSyncTest,
        RGBWaveform,
        GLRGBWaveform,
        GLSLRGBWaveform,
        QtVSyncTest,
        QtHSVWaveform,
        QtRGBWaveform,
        Count_WaveformwidgetType // Also used as invalid value
    };
};

#endif // WAVEFORMWIDGETTYPE_H
