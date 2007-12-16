#include "engineobject.h"

#define XF_STEEPNESS_FACTOR 4.

class EngineXfader{
public:
	static float getCalibration(float transform);
	static void getXfadeGains(float &gain1, float &gain2, float xfadePosition, float transform, float calibration);

};
