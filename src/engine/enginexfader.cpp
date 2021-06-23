#include "engine/enginexfader.h"

#include "util/math.h"

//static
const char* EngineXfader::kXfaderConfigKey = "[Mixer Profile]";
const double EngineXfader::kTransformDefault = 1.0;
const double EngineXfader::kTransformMax = 1000.0;
const double EngineXfader::kTransformMin = 0.6;

double EngineXfader::getPowerCalibration(double transform) {
    // get the transform_root of -3db (.5)
    return pow(0.5, 1.0 / transform);
}

void EngineXfader::getXfadeGains(double xfadePosition,
        double transform,
        double powerCalibration,
        double curve,
        bool reverse,
        CSAMPLE_GAIN* gain1,
        CSAMPLE_GAIN* gain2) {
    if (gain1 == nullptr || gain2 == nullptr) {
        return;
    }

    // Slow-fade/fast-cut
    double xfadePositionLeft = xfadePosition;
    double xfadePositionRight = xfadePosition;

    if (curve == MIXXX_XFADER_CONSTPWR) {
        // Apply Calibration
        xfadePosition *= powerCalibration;
        xfadePositionLeft = xfadePosition - powerCalibration;
        xfadePositionRight = xfadePosition + powerCalibration;
    }

    if (xfadePositionLeft < 0) { // on left side
        xfadePositionLeft *= -1;
        *gain2 = static_cast<CSAMPLE_GAIN>(1.0 - (1.0 * pow(xfadePositionLeft, transform)));
    } else {
        *gain2 = 1.0f;
    }

    if(xfadePositionRight > 0) { // right side
        *gain1 = static_cast<CSAMPLE_GAIN>(1.0 - (1.0 * pow(xfadePositionRight, transform)));
    } else {
        *gain1 = 1.0f;
    }

    //prevent phase reversal
    if (*gain1 < 0.0) {
        *gain1 = 0.0f;
    }
    if (*gain2 < 0.0) {
        *gain2 = 0.0f;
    }

    if (curve == MIXXX_XFADER_CONSTPWR) {
        if (*gain1 > *gain2) {
            *gain2 = 1 - *gain1;
        } else {
            *gain1 = 1 - *gain2;
        }

        // The resulting power at the crossover point depends on the correlation of the input signals
        // In theory the gain ratio varies from 0.5 for two equal signals to sqrt(0.5) = 0.707 for totally
        // uncorrelated signals.
        // Since the underlying requirement for this curve is constant loudness, we did a test with 30 s
        // snippets of various genres and ReplayGain 2.0 analysis. Almost all results where near 0.707
        // with one exception of mixing two parts of the same track, which resulted in 0.66.
        // Based on the testing, we normalize the gain as if the signals were uncorrelated. The
        // correction on the following lines ensures that  gain1^2 + gain2^2 == 1.
        CSAMPLE_GAIN gain = static_cast<CSAMPLE_GAIN>(sqrt(*gain1 * *gain1 + *gain2 * *gain2));
        *gain1 = *gain1 / gain;
        *gain2 = *gain2 / gain;
    }

    if (reverse) {
        CSAMPLE_GAIN gain_temp = *gain1;
        *gain1 = *gain2;
        *gain2 = gain_temp;
    }
}
