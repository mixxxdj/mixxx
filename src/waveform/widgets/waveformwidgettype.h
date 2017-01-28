#ifndef WAVEFORMWIDGETTYPE_H
#define WAVEFORMWIDGETTYPE_H

class WaveformWidgetType {
  public:
    enum Type {
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
        Count_WaveformwidgetType // Also used as invalid value
    };
};

#endif // WAVEFORMWIDGETTYPE_H
