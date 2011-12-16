#ifndef WAVEFORMWIDGETTYPE_H
#define WAVEFORMWIDGETTYPE_H

class WaveformWidgetType
{
public:
    enum Type {
        EmptyWaveform = 0,
        SimpleSoftwareWaveform, //TODO
        SoftwareWaveform, //TODO
        SimpleGlWaveform, //TODO
        GlWaveform,
        GLSLWaveform,
        Count_WaveformwidgetType
    };
};

#endif // WAVEFORMWIDGETTYPE_H
