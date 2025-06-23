#include "engine/filters/enginefilterlinkwitzriley4.h"

#include "moc_enginefilterlinkwitzriley4.cpp"

namespace {
constexpr char kFidSpecLowPassButterworth2[] = "LpBu2";
constexpr char kFidSpecHighPassButterworth2[] = "HpBu2";
} // namespace

EngineFilterLinkwitzRiley4Low::EngineFilterLinkwitzRiley4Low(
        mixxx::audio::SampleRate sampleRate, double freqCorner1) {
    setFrequencyCorners(sampleRate, freqCorner1);
}

void EngineFilterLinkwitzRiley4Low::setFrequencyCorners(mixxx::audio::SampleRate sampleRate,
        double freqCorner1) {
    // Copy the old coefficients into m_oldCoef
    setCoefs2(sampleRate,
            2,
            kFidSpecLowPassButterworth2,
            sizeof(kFidSpecLowPassButterworth2),
            freqCorner1,
            0,
            0,
            kFidSpecLowPassButterworth2,
            sizeof(kFidSpecLowPassButterworth2),
            freqCorner1,
            0,
            0);
}

EngineFilterLinkwitzRiley4High::EngineFilterLinkwitzRiley4High(
        mixxx::audio::SampleRate sampleRate, double freqCorner1) {
    setFrequencyCorners(sampleRate, freqCorner1);
}

void EngineFilterLinkwitzRiley4High::setFrequencyCorners(mixxx::audio::SampleRate sampleRate,
        double freqCorner1) {
    setCoefs2(sampleRate,
            2,
            kFidSpecHighPassButterworth2,
            sizeof(kFidSpecHighPassButterworth2),
            freqCorner1,
            0,
            0,
            kFidSpecHighPassButterworth2,
            sizeof(kFidSpecHighPassButterworth2),
            freqCorner1,
            0,
            0);
}
