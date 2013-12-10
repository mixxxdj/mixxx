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
        GLWaveform,
        GLSLWaveform,
        HSVWaveform,
        GLVSyncTest,
        RGBWaveform,
        Count_WaveformwidgetType // Also used as invalid value
    };
};

#endif // WAVEFORMWIDGETTYPE_H
