#include "engineobject.h"
class EngineXfader{
public:
	static FLOAT_TYPE getCalibration(FLOAT_TYPE transform);
	static void getXfadeGains(FLOAT_TYPE &gain1, FLOAT_TYPE &gain2, FLOAT_TYPE xfadePosition, FLOAT_TYPE transform, FLOAT_TYPE calibration);

};