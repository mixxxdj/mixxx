#pragma once

#include "util/types.h"

// HACK until we have Control 2.0
#define MIXXX_XFADER_ADDITIVE   0.0
#define MIXXX_XFADER_CONSTPWR   1.0

class EngineXfader {
  public:
    static double getPowerCalibration(double transform);
    static void getXfadeGains(double xfadePosition,
            double transform,
            double powerCalibration,
            double curve,
            bool reverse,
            CSAMPLE_GAIN* gain1,
            CSAMPLE_GAIN* gain2);

    static const char* kXfaderConfigKey;
    static const double kTransformDefault;
    static const double kTransformMax;
    static const double kTransformMin;
};
