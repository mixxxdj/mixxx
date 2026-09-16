#include "engine/filters/enginefilterbutterworth2.h"

#include "moc_enginefilterbutterworth2.cpp"

namespace {
constexpr char kFidSpecLowPassButterworth2[] = "LpBu2";
constexpr char kFidSpecBandPassButterworth2[] = "BpBu2";
constexpr char kFidSpecHighPassButterworth2[] = "HpBu2";
} // namespace

EngineFilterButterworth2Low::EngineFilterButterworth2Low(
        mixxx::audio::SampleRate sampleRate, double freqCorner1) {
    setFrequencyCorners(sampleRate, freqCorner1);
}

void EngineFilterButterworth2Low::setFrequencyCorners(mixxx::audio::SampleRate sampleRate,
        double freqCorner1) {
    // Copy the old coefficients into m_oldCoef
    setCoefs(kFidSpecLowPassButterworth2,
            sizeof(kFidSpecLowPassButterworth2),
            sampleRate,
            freqCorner1);
}

EngineFilterButterworth2Band::EngineFilterButterworth2Band(
        mixxx::audio::SampleRate sampleRate,
        double freqCorner1,
        double freqCorner2) {
    setFrequencyCorners(sampleRate, freqCorner1, freqCorner2);
}

void EngineFilterButterworth2Band::setFrequencyCorners(mixxx::audio::SampleRate sampleRate,
        double freqCorner1,
        double freqCorner2) {
    setCoefs(kFidSpecBandPassButterworth2,
            sizeof(kFidSpecBandPassButterworth2),
            sampleRate,
            freqCorner1,
            freqCorner2);
}

EngineFilterButterworth2High::EngineFilterButterworth2High(
        mixxx::audio::SampleRate sampleRate, double freqCorner1) {
    setFrequencyCorners(sampleRate, freqCorner1);
}

void EngineFilterButterworth2High::setFrequencyCorners(mixxx::audio::SampleRate sampleRate,
        double freqCorner1) {
    setCoefs(kFidSpecHighPassButterworth2,
            sizeof(kFidSpecHighPassButterworth2),
            sampleRate,
            freqCorner1);
}
