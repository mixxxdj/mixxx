#ifndef ENGINEXFADER_H
#define ENGINEXFADER_H

// HACK until we have Control 2.0
#define MIXXX_XFADER_ADDITIVE   0.0
#define MIXXX_XFADER_CONSTPWR   1.0

class EngineXfader {
  public:
    static double getPowerCalibration(double transform);
    static void getXfadeGains(
        double xfadePosition, double transform, double powerCalibration,
        bool constPower, bool reverse, double* gain1, double* gain2);
};

#endif /* ENGINEXFADER_H */
