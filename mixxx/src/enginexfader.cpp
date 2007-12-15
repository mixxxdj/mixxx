#include "enginexfader.h"

FLOAT_TYPE EngineXfader::getCalibration(FLOAT_TYPE transform)
{
	//get the transform_root of -3db (.5)
	return pow(.5, 1./transform);
}

void EngineXfader::getXfadeGains(FLOAT_TYPE &gain1, FLOAT_TYPE &gain2, FLOAT_TYPE xfadePosition, FLOAT_TYPE transform, FLOAT_TYPE calibration)
{
	//Apply Calibration
	xfadePosition *= calibration;
	FLOAT_TYPE xfadePositionLeft = xfadePosition - calibration;
	FLOAT_TYPE xfadePositionRight = xfadePosition + calibration;

	if(xfadePositionLeft < 0) //on left side
	{
		xfadePositionLeft *= -1;
		gain2 = 1. * (1. - (1. * pow(xfadePositionLeft, transform)));
	}
	else
		gain2 = 1.;
	if(xfadePositionRight > 0) //right side
	{
		gain1 = 1. * (1. - (1. * pow(xfadePositionRight, transform)));
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