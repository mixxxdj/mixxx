#include "engine/filters/enginefilterbutterworth8.h"

#include "moc_enginefilterbutterworth8.cpp"

namespace {
constexpr char kFidSpecLowPassButterworth8[] = "LpBu8";
constexpr char kFidSpecBandPassButterworth8[] = "BpBu8";
constexpr char kFidSpecHighPassButterworth8[] = "HpBu8";
} // namespace

EngineFilterButterworth8Low::EngineFilterButterworth8Low(int sampleRate, double freqCorner1) {
    setFrequencyCorners(sampleRate, freqCorner1);
}

void EngineFilterButterworth8Low::setFrequencyCorners(int sampleRate,
                                             double freqCorner1) {
    // Copy the old coefficients into m_oldCoef
    setCoefs(kFidSpecLowPassButterworth8,
            sizeof(kFidSpecLowPassButterworth8),
            sampleRate,
            freqCorner1);
}


EngineFilterButterworth8Band::EngineFilterButterworth8Band(int sampleRate, double freqCorner1,
                                         double freqCorner2) {
    setFrequencyCorners(sampleRate, freqCorner1, freqCorner2);
}

void EngineFilterButterworth8Band::setFrequencyCorners(int sampleRate,
                                             double freqCorner1,
                                             double freqCorner2) {
    setCoefs(kFidSpecBandPassButterworth8,
            sizeof(kFidSpecBandPassButterworth8),
            sampleRate,
            freqCorner1,
            freqCorner2);
}

EngineFilterButterworth8High::EngineFilterButterworth8High(int sampleRate, double freqCorner1) {
    setFrequencyCorners(sampleRate, freqCorner1);
}

void EngineFilterButterworth8High::setFrequencyCorners(int sampleRate,
                                             double freqCorner1) {
    setCoefs(kFidSpecHighPassButterworth8,
            sizeof(kFidSpecHighPassButterworth8),
            sampleRate,
            freqCorner1);
}
