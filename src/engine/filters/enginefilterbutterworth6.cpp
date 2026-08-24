#include "engine/filters/enginefilterbutterworth6.h"

#include "moc_enginefilterbutterworth6.cpp"

namespace {
constexpr char kFidSpecLowPassButterworth6[] = "LpBu6";
constexpr char kFidSpecBandPassButterworth6[] = "BpBu6";
constexpr char kFidSpecHighPassButterworth6[] = "HpBu6";
} // namespace

EngineFilterButterworth6Low::EngineFilterButterworth6Low(
        mixxx::audio::SampleRate sampleRate, double freqCorner1) {
    setFrequencyCorners(sampleRate, freqCorner1);
}

void EngineFilterButterworth6Low::setFrequencyCorners(mixxx::audio::SampleRate sampleRate,
        double freqCorner1) {
    // Copy the old coefficients into m_oldCoef
    setCoefs(kFidSpecLowPassButterworth6,
            sizeof(kFidSpecLowPassButterworth6),
            sampleRate,
            freqCorner1);
}

EngineFilterButterworth6Band::EngineFilterButterworth6Band(
        mixxx::audio::SampleRate sampleRate,
        double freqCorner1,
        double freqCorner2) {
    setFrequencyCorners(sampleRate, freqCorner1, freqCorner2);
}

void EngineFilterButterworth6Band::setFrequencyCorners(mixxx::audio::SampleRate sampleRate,
        double freqCorner1,
        double freqCorner2) {
    setCoefs(kFidSpecBandPassButterworth6,
            sizeof(kFidSpecBandPassButterworth6),
            sampleRate,
            freqCorner1,
            freqCorner2);
}

EngineFilterButterworth6High::EngineFilterButterworth6High(
        mixxx::audio::SampleRate sampleRate, double freqCorner1) {
    setFrequencyCorners(sampleRate, freqCorner1);
}

void EngineFilterButterworth6High::setFrequencyCorners(mixxx::audio::SampleRate sampleRate,
        double freqCorner1) {
    setCoefs(kFidSpecHighPassButterworth6,
            sizeof(kFidSpecHighPassButterworth6),
            sampleRate,
            freqCorner1);
}
