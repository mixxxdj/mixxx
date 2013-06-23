#ifndef WAVEFORMWIDGETTYPE_H
#define WAVEFORMWIDGETTYPE_H

class WaveformWidgetType {
  public:
    enum Type {
        EmptyWaveform = 0,
        SoftwareSimpleWaveform, //TODO
        SoftwareWaveform, //TODO
        QtSimpleWaveform, //TODO
        QtWaveform,
        GLSimpleWaveform,
        GLWaveform,
        GLSLWaveform,
        HSVWaveform,
        Count_WaveformwidgetType // Also used as invalid value
    };
};

#endif // WAVEFORMWIDGETTYPE_H
