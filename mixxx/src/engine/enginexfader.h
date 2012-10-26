// HACK until we have Control 2.0
#define MIXXX_XFADER_ADDITIVE   0.0f
#define MIXXX_XFADER_CONSTPWR   1.0f

class EngineXfader {
    
public:
    static float getCalibration(float transform);
    static void getXfadeGains(float &gain1, float &gain2, float xfadePosition, float transform, float calibration, bool constPower, bool reverse);
    
};
