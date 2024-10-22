#pragma once

#include "waveform/waveform.h"

class IVisualGainProvider {
  public:
    virtual double getVisualGain(FilterIndex index) const = 0;
};
