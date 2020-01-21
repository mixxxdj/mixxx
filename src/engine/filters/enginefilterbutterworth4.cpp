#include "engine/filters/enginefilterbutterworth4.h"

#include "moc_enginefilterbutterworth4.cpp"

namespace {
constexpr char kFidSpecLowPassButterworth4[] = "LpBu4";
constexpr char kFidSpecBandPassButterworth4[] = "BpBu4";
constexpr char kFidSpecHighPassButterworth4[] = "HpBu4";
} // namespace

EngineFilterButterworth4Low::EngineFilterButterworth4Low(int sampleRate, double freqCorner1) {
    setFrequencyCorners(sampleRate, freqCorner1);
}

void EngineFilterButterworth4Low::setFrequencyCorners(int sampleRate,
                                             double freqCorner1) {
    // Copy the old coefficients into m_oldCoef
    setCoefs(kFidSpecLowPassButterworth4,
            sizeof(kFidSpecLowPassButterworth4),
            sampleRate,
            freqCorner1);
}


EngineFilterButterworth4Band::EngineFilterButterworth4Band(int sampleRate, double freqCorner1,
                                         double freqCorner2) {
    setFrequencyCorners(sampleRate, freqCorner1, freqCorner2);
}

void EngineFilterButterworth4Band::setFrequencyCorners(int sampleRate,
                                             double freqCorner1,
                                             double freqCorner2) {
    setCoefs(kFidSpecBandPassButterworth4,
            sizeof(kFidSpecBandPassButterworth4),
            sampleRate,
            freqCorner1,
            freqCorner2);
}


EngineFilterButterworth4High::EngineFilterButterworth4High(int sampleRate, double freqCorner1) {
    setFrequencyCorners(sampleRate, freqCorner1);
}

void EngineFilterButterworth4High::setFrequencyCorners(int sampleRate,
                                             double freqCorner1) {
    setCoefs(kFidSpecHighPassButterworth4,
            sizeof(kFidSpecHighPassButterworth4),
            sampleRate,
            freqCorner1);
}
