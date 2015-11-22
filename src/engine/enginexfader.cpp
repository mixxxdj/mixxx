#include "engine/enginexfader.h"

#include "util/math.h"

//static
const char* EngineXfader::kXfaderConfigKey = "[Mixer Profile]";
const double EngineXfader::kTransformMax = 1000.0;
const double EngineXfader::kTransformMin = 1.0;

double EngineXfader::getPowerCalibration(double transform) {
    // get the transform_root of -3db (.5)
    return pow(0.5, 1.0 / transform);
}

void EngineXfader::getXfadeGains(
        double xfadePosition, double transform, double powerCalibration,
        bool constPower, bool reverse, double* gain1, double* gain2) {
    if (gain1 == NULL || gain2 == NULL) {
        return;
    }

    // Slow-fade/fast-cut
    double xfadePositionLeft = xfadePosition;
    double xfadePositionRight = xfadePosition;

    if (constPower) {
        // Apply Calibration
        xfadePosition *= powerCalibration;
        xfadePositionLeft = xfadePosition - powerCalibration;
        xfadePositionRight = xfadePosition + powerCalibration;
    }

    if (xfadePositionLeft < 0) { // on left side
        xfadePositionLeft *= -1;
        *gain2 = (1.0 - (1.0 * pow(xfadePositionLeft, transform)));
    } else {
        *gain2 = 1.0;
    }

    if(xfadePositionRight > 0) { // right side
        *gain1 = (1.0 - (1.0 * pow(xfadePositionRight, transform)));
    } else {
        *gain1 = 1.0;
    }

    //prevent phase reversal
    if (*gain1 < 0.0) {
        *gain1 = 0.0;
    }
    if (*gain2 < 0.0) {
        *gain2 = 0.0;
    }

    if (constPower) {
        if (*gain1 > *gain2) {
            *gain2 = 1 - *gain1;
        } else {
            *gain1 = 1 - *gain2;
        }
    }

    if (reverse) {
        double gain_temp = *gain1;
        *gain1 = *gain2;
        *gain2 = gain_temp;
    }
}
