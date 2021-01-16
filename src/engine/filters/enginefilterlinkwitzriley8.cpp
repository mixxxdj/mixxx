#include "engine/filters/enginefilterlinkwitzriley8.h"

#include "moc_enginefilterlinkwitzriley8.cpp"

namespace {
constexpr char kFidSpecLowPassButterworth4[] = "LpBu4";
constexpr char kFidSpecHighPassButterworth4[] = "HpBu4";
} // namespace

EngineFilterLinkwitzRiley8Low::EngineFilterLinkwitzRiley8Low(int sampleRate, double freqCorner1) {
    setFrequencyCorners(sampleRate, freqCorner1);
}

void EngineFilterLinkwitzRiley8Low::setFrequencyCorners(int sampleRate,
                                             double freqCorner1) {
    // Copy the old coefficients into m_oldCoef
    setCoefs2(sampleRate,
            4,
            kFidSpecLowPassButterworth4,
            sizeof(kFidSpecLowPassButterworth4),
            freqCorner1,
            0,
            0,
            kFidSpecLowPassButterworth4,
            sizeof(kFidSpecLowPassButterworth4),
            freqCorner1,
            0,
            0);
}

EngineFilterLinkwitzRiley8High::EngineFilterLinkwitzRiley8High(int sampleRate, double freqCorner1) {
    setFrequencyCorners(sampleRate, freqCorner1);
}

void EngineFilterLinkwitzRiley8High::setFrequencyCorners(int sampleRate,
                                             double freqCorner1) {
    setCoefs2(sampleRate,
            4,
            kFidSpecHighPassButterworth4,
            sizeof(kFidSpecHighPassButterworth4),
            freqCorner1,
            0,
            0,
            kFidSpecHighPassButterworth4,
            sizeof(kFidSpecHighPassButterworth4),
            freqCorner1,
            0,
            0);
}
