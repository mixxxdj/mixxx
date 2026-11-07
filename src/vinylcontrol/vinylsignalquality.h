#pragma once

#include "vinylcontrol/defs_vinylcontrol.h"

struct VinylSignalQualityReport {
    unsigned char processor;
    float timecode_quality;
    float angle;
    unsigned char scope[kVinylScopeSize * kVinylScopeSize];
};

class VinylSignalQualityListener {
  public:
    virtual void onVinylSignalQualityUpdate(const VinylSignalQualityReport& report) = 0;
};
