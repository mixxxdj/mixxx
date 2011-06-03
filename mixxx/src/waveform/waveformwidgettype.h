#ifndef WAVEFORMWIDGETTYPE_H
#define WAVEFORMWIDGETTYPE_H

class WaveformWidgetType
{
public:
    enum Type {
        EmptyWaveform = 0, //TODO
        SimplePureQtWaveform, //TODO
        FilteredPureQtWaveform,
        SimpleOpenGlWaveform, //TODO
        FilteredOpenGlWaveform,
        Count_WaveformwidgetType
    };
};

#endif // WAVEFORMWIDGETTYPE_H
