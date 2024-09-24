#pragma once

#include "util/singleton.h"
#include "waveform/waveform.h"

class WaveformOverviewRenderer : public Singleton<WaveformOverviewRenderer> {
  public:
    QImage render(ConstWaveformPointer) const;
};
