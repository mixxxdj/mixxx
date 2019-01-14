#include "engine/filters/enginefilterbessel4.h"
#include "util/math.h"


EngineFilterBessel4Low::EngineFilterBessel4Low(int sampleRate,
                                               double freqCorner1) {
    setFrequencyCorners(sampleRate, freqCorner1);
}

void EngineFilterBessel4Low::setFrequencyCorners(int sampleRate,
                                                 double freqCorner1) {
    // Copy the old coefficients into m_oldCoef
    setCoefs("LpBe4", sampleRate, freqCorner1);
}

int EngineFilterBessel4Low::setFrequencyCornersForIntDelay(
        double desiredCorner1Ratio, int maxDelay) {
    // these values are calculated using the phase returned by
    // fid_response_pha() at corner / 20

    // group delay at 1 Hz freqCorner1 and 1 Hz Samplerate
    const double kDelayFactor1 = 0.336440447;
    // Factor, required to hit the end of the quadratic curve
    const double kDelayFactor2 = 1.1044845;
    // Table for the non quadratic, high part near the sample rate
    const double delayRatioTable[] = {
            0.500000000,  // delay 0
            0.258899546,  // delay 1
            0.154778862,  // delay 2
            0.107833769,  // delay 3
            0.082235025,  // delay 4
            0.066314175,  // delay 5
            0.055505336,  // delay 6
            0.047691446,  // delay 7
            0.041813481,  // delay 8
            0.037212241,  // delay 9
            0.033519902,  // delay 10
            0.030497945,  // delay 11
            0.027964718,  // delay 12
    };


    double dDelay = kDelayFactor1 / desiredCorner1Ratio - kDelayFactor2 * desiredCorner1Ratio;
    int iDelay =  math_clamp((int)(dDelay + 0.5), 0, maxDelay);

    double quantizedRatio;
    if (iDelay >= (int)(sizeof(delayRatioTable) / sizeof(double))) {
        // pq formula, only valid for low frequencies
        quantizedRatio = (-(iDelay / kDelayFactor2 / 2)) +
                sqrt((iDelay / kDelayFactor2 / 2)*(iDelay / kDelayFactor2 / 2)
                                       + kDelayFactor1 / kDelayFactor2);
    } else {
        quantizedRatio = delayRatioTable[iDelay];
    }

    setCoefs("LpBe4", 1, quantizedRatio);
    return iDelay;
}

EngineFilterBessel4Band::EngineFilterBessel4Band(int sampleRate,
                                                 double freqCorner1,
                                                 double freqCorner2) {
    setFrequencyCorners(sampleRate, freqCorner1, freqCorner2);
}

void EngineFilterBessel4Band::setFrequencyCorners(int sampleRate,
                                                  double freqCorner1,
                                                  double freqCorner2) {
    setCoefs("BpBe4", sampleRate, freqCorner1, freqCorner2);
}


EngineFilterBessel4High::EngineFilterBessel4High(int sampleRate,
                                                 double freqCorner1) {
    setFrequencyCorners(sampleRate, freqCorner1);
}

void EngineFilterBessel4High::setFrequencyCorners(int sampleRate,
                                                  double freqCorner1) {
    setCoefs("HpBe4", sampleRate, freqCorner1);
}
