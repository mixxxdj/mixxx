#include "engine/enginebandmeter.h"

#include <cmath>

#include "engine/engine.h"
#include "util/assert.h"
#include "util/defs.h"
#include "util/sample.h"

namespace mixxx {

namespace {

// [AI-generated] The bands are narrower than the bands of the mixing
// equalizers, whose default shelves are 250 Hz and 2.5 kHz. The gaps around
// the shelves keep the leakage of the neighbor bands out, so that a band
// killed with the EQ reads as silent. [/AI-generated]
constexpr double kLowBandMaxHz = 150.0;
constexpr double kMidBandMinHz = 400.0;
constexpr double kMidBandMaxHz = 1500.0;
constexpr double kHighBandMinHz = 4000.0;

constexpr audio::SampleRate kDefaultSampleRate = audio::SampleRate(44100);

// [AI-generated] The levels are published at this rate at most. Engine
// buffers shorter than one period are accumulated. [/AI-generated]
constexpr int kUpdateRateHz = 60;

constexpr double kLevelEpsilon = 0.0001;

} // namespace

EngineBandMeter::Band::Band(const ConfigKey& key)
        : m_level(key),
          m_sumAbs(CSAMPLE_ZERO) {
    m_level.setReadOnly();
}

EngineBandMeter::EngineBandMeter(const QString& group)
        : m_low(ConfigKey(group, QStringLiteral("band_meter_low"))),
          m_mid(ConfigKey(group, QStringLiteral("band_meter_mid"))),
          m_high(ConfigKey(group, QStringLiteral("band_meter_high"))),
          m_lowFilter(kDefaultSampleRate, kLowBandMaxHz),
          m_midHighPassFilter(kDefaultSampleRate, kMidBandMinHz),
          m_midLowPassFilter(kDefaultSampleRate, kMidBandMaxHz),
          m_highFilter(kDefaultSampleRate, kHighBandMinHz),
          m_filtered(kMaxEngineSamples),
          m_sampleRate(kDefaultSampleRate),
          m_samplesAccumulated(0),
          m_isReset(false) {
    reset();
}

void EngineBandMeter::process(
        const CSAMPLE* pIn, std::size_t bufferSize, audio::SampleRate sampleRate) {
    VERIFY_OR_DEBUG_ASSERT(sampleRate.isValid() &&
            bufferSize <= static_cast<std::size_t>(m_filtered.size())) {
        return;
    }
    if (sampleRate != m_sampleRate) {
        m_sampleRate = sampleRate;
        m_lowFilter.setFrequencyCorners(sampleRate, kLowBandMaxHz);
        m_midHighPassFilter.setFrequencyCorners(sampleRate, kMidBandMinHz);
        m_midLowPassFilter.setFrequencyCorners(sampleRate, kMidBandMaxHz);
        m_highFilter.setFrequencyCorners(sampleRate, kHighBandMinHz);
    }
    m_isReset = false;

    CSAMPLE* pFiltered = m_filtered.data();
    m_lowFilter.process(pIn, pFiltered, bufferSize);
    accumulate(&m_low, bufferSize);

    m_midHighPassFilter.process(pIn, pFiltered, bufferSize);
    m_midLowPassFilter.process(pFiltered, pFiltered, bufferSize);
    accumulate(&m_mid, bufferSize);

    m_highFilter.process(pIn, pFiltered, bufferSize);
    accumulate(&m_high, bufferSize);

    m_samplesAccumulated += bufferSize;
    if (m_samplesAccumulated * kUpdateRateHz >=
            static_cast<std::size_t>(sampleRate.value()) * kEngineChannelOutputCount) {
        publish(&m_low);
        publish(&m_mid);
        publish(&m_high);
        m_samplesAccumulated = 0;
    }
}

void EngineBandMeter::reset() {
    if (m_isReset) {
        return;
    }
    m_isReset = true;
    m_samplesAccumulated = 0;
    for (Band* pBand : {&m_low, &m_mid, &m_high}) {
        pBand->m_sumAbs = CSAMPLE_ZERO;
        pBand->m_level.forceSet(0.0);
    }
    m_lowFilter.pauseFilter();
    m_midHighPassFilter.pauseFilter();
    m_midLowPassFilter.pauseFilter();
    m_highFilter.pauseFilter();
}

void EngineBandMeter::accumulate(Band* pBand, std::size_t bufferSize) {
    CSAMPLE sumAbsLeft;
    CSAMPLE sumAbsRight;
    SampleUtil::sumAbsPerChannel(&sumAbsLeft,
            &sumAbsRight,
            m_filtered.data(),
            static_cast<SINT>(bufferSize));
    pBand->m_sumAbs += sumAbsLeft + sumAbsRight;
}

void EngineBandMeter::publish(Band* pBand) {
    const CSAMPLE level = std::fmin(pBand->m_sumAbs / m_samplesAccumulated, CSAMPLE_ONE);
    pBand->m_sumAbs = CSAMPLE_ZERO;
    // [AI-generated] The levels change with every update. The epsilon keeps
    // inaudible changes from waking up the listeners of the control.
    // [/AI-generated]
    if (std::fabs(level - pBand->m_level.get()) > kLevelEpsilon) {
        pBand->m_level.forceSet(level);
    }
}

} // namespace mixxx
