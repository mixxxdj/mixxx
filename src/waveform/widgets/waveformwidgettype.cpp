#include "waveform/widgets/waveformwidgettype.h"

const QSet<WaveformWidgetType::Type> WaveformWidgetType::openGlTypes = {
        QtSimpleWaveform,
        QtWaveform,
        GLSimpleWaveform,
        GLFilteredWaveform,
        GLSLFilteredWaveform,
        GLVSyncTest,
        GLRGBWaveform,
        GLSLRGBWaveform,
        QtVSyncTest,
        QtHSVWaveform,
        QtRGBWaveform,
};
