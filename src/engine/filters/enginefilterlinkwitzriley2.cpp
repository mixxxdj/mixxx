#include "engine/filters/enginefilterlinkwitzriley2.h"

#include "moc_enginefilterlinkwitzriley2.cpp"

namespace {
constexpr char kFidSpecLowPassButterworth1[] = "LpBu1";
constexpr char kFidSpecHighPassButterworth1[] = "HpBu1";
} // namespace

EngineFilterLinkwitzRiley2Low::EngineFilterLinkwitzRiley2Low(
        mixxx::audio::SampleRate sampleRate, double freqCorner1) {
    setFrequencyCorners(sampleRate, freqCorner1);
}

void EngineFilterLinkwitzRiley2Low::setFrequencyCorners(mixxx::audio::SampleRate sampleRate,
        double freqCorner1) {
    // Copy the old coefficients into m_oldCoef
    setCoefs2(sampleRate,
            1,
            kFidSpecLowPassButterworth1,
            sizeof(kFidSpecLowPassButterworth1),
            freqCorner1,
            0,
            0,
            kFidSpecLowPassButterworth1,
            sizeof(kFidSpecLowPassButterworth1),
            freqCorner1,
            0,
            0);
}

EngineFilterLinkwitzRiley2High::EngineFilterLinkwitzRiley2High(
        mixxx::audio::SampleRate sampleRate, double freqCorner1) {
    setFrequencyCorners(sampleRate, freqCorner1);
}

void EngineFilterLinkwitzRiley2High::setFrequencyCorners(mixxx::audio::SampleRate sampleRate,
        double freqCorner1) {
    setCoefs2(sampleRate,
            1,
            kFidSpecHighPassButterworth1,
            sizeof(kFidSpecHighPassButterworth1),
            freqCorner1,
            0,
            0,
            kFidSpecHighPassButterworth1,
            sizeof(kFidSpecHighPassButterworth1),
            freqCorner1,
            0,
            0);
}
