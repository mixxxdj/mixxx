#ifndef VINYLSIGNALQUALITY_H
#define VINYLSIGNALQUALITY_H

#include "vinylcontrol/defs_vinylcontrol.h"

struct VinylSignalQualityReport {
    unsigned char processor;
    float timecode_quality;
    float angle;
    unsigned char scope[MIXXX_VINYL_SCOPE_SIZE*MIXXX_VINYL_SCOPE_SIZE];
};

class VinylSignalQualityListener {
  public:
    virtual void onVinylSignalQualityUpdate(const VinylSignalQualityReport& report) = 0;
};

#endif /* VINYLSIGNALQUALITY_H */
