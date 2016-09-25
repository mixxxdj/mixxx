#pragma once

#include "util/singleton.h"
#include "waveform/waveform.h"

class WaveformOverviewRenderer : public Singleton<WaveformOverviewRenderer> {
  public:
    QImage render(ConstWaveformPointer) const;

    // QImage renderToImage() const;

    /*
    // We do not lock the mutex since m_dataSize and m_visualSampleRate are not
    // changed after the constructor runs.
    bool isValid() const {
        return getDataSize() > 0 && getVisualSampleRate() > 0;
    }
*/
};
