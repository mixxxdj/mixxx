#include "engine/filters/enginefilterbessel8.h"

#include "moc_enginefilterbessel8.cpp"
#include "util/math.h"

namespace {
constexpr char kFidSpecLowPassBessel8[] = "LpBe8";
constexpr char kFidSpecBandPassBessel8[] = "BpBe8";
constexpr char kFidSpecHighPassBessel8[] = "HpBe8";
} // namespace

EngineFilterBessel8Low::EngineFilterBessel8Low(int sampleRate,
                                               double freqCorner1) {
    setFrequencyCorners(sampleRate, freqCorner1);
}

void EngineFilterBessel8Low::setFrequencyCorners(int sampleRate,
                                                 double freqCorner1) {
    // Copy the old coefficients into m_oldCoef
    setCoefs(kFidSpecLowPassBessel8, sizeof(kFidSpecLowPassBessel8), sampleRate, freqCorner1);
}


int EngineFilterBessel8Low::setFrequencyCornersForIntDelay(
        double desiredCorner1Ratio, int maxDelay) {
    // these values are calculated using the phase returned by
    // fid_response_pha() at corner / 20

    // group delay at 1 Hz freqCorner1 and 1 Hz Samplerate
    const double kDelayFactor1 = 0.506051799;
    // Factor, required to hit the end of the quadratic curve
    const double kDelayFactor2 = 1.661247;
    // Table for the non quadratic, high part near the sample rate
    const double delayRatioTable[] = {
            0.500000000,  // delay 0
            0.321399282,  // delay 1
            0.213843537,  // delay 2
            0.155141284,  // delay 3
            0.120432232,  // delay 4
            0.097999886,  // delay 5
            0.082451739,  // delay 6
            0.071098408,  // delay 7
            0.062444910,  // delay 8
            0.055665936,  // delay 9
            0.050197933,  // delay 10
            0.045689120,  // delay 11
            0.041927420,  // delay 12
            0.038735202,  // delay 13
            0.035992756,  // delay 14
            0.033611618,  // delay 15
            0.031525020,  // delay 16
            0.029681641,  // delay 17
            0.028041409,  // delay 18
            0.026572562,  // delay 19
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

    setCoefs(kFidSpecLowPassBessel8, sizeof(kFidSpecLowPassBessel8), 1, quantizedRatio);
    return iDelay;
}

EngineFilterBessel8Band::EngineFilterBessel8Band(int sampleRate,
                                                 double freqCorner1,
                                                 double freqCorner2) {
    setFrequencyCorners(sampleRate, freqCorner1, freqCorner2);
}

void EngineFilterBessel8Band::setFrequencyCorners(int sampleRate,
                                                  double freqCorner1,
                                                  double freqCorner2) {
    setCoefs(kFidSpecBandPassBessel8,
            sizeof(kFidSpecBandPassBessel8),
            sampleRate,
            freqCorner1,
            freqCorner2);
}


EngineFilterBessel8High::EngineFilterBessel8High(int sampleRate,
                                                 double freqCorner1) {
    setFrequencyCorners(sampleRate, freqCorner1);
}

void EngineFilterBessel8High::setFrequencyCorners(int sampleRate,
                                                  double freqCorner1) {
    setCoefs(kFidSpecHighPassBessel8, sizeof(kFidSpecHighPassBessel8), sampleRate, freqCorner1);
}
