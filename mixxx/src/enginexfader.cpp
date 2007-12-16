#include "enginexfader.h"

FLOAT_TYPE EngineXfader::getCalibration(float transform)
{
	//get the transform_root of -3db (.5)
	return pow(.5, 1./transform);
}

void EngineXfader::getXfadeGains(float &gain1, float &gain2, float xfadePosition, float transform, float calibration)
{
	//Apply Calibration
	xfadePosition *= calibration;
	float xfadePositionLeft = xfadePosition - calibration;
	float xfadePositionRight = xfadePosition + calibration;

	if(xfadePositionLeft < 0) //on left side
	{
		xfadePositionLeft *= -1;
		gain2 = (1. - (1. * pow(xfadePositionLeft, transform)));
	}
	else
		gain2 = 1.;
	if(xfadePositionRight > 0) //right side
	{
		gain1 = (1. - (1. * pow(xfadePositionRight, transform)));
	}
	else
		gain1 = 1.;
	
	//prevent phase reversal
	if(gain1 < 0.)
		gain1 = 0.;
	if(gain2 < 0.)
		gain2 = 0.;

	return;
}
