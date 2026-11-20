#include "engine/enginebandmeter.h"

#include <array>
#include <cmath>

#include "engine/engine.h"
#include "engine/filters/enginefilterlinkwitzriley4.h"
#include "util/assert.h"
#include "util/defs.h"
#include "util/sample.h"

namespace mixxx {

namespace {

// The bands are narrower than the bands of the mixing equalizers, whose
// default shelves are 250 Hz and 2.5 kHz. The gaps around the shelves keep
// the leakage of the neighbor bands out, so that a band killed with the EQ
// reads as silent.
constexpr double kLowBandMaxHz = 125.0;
constexpr double kMidBandMinHz = 400.0;
constexpr double kMidBandMaxHz = 1500.0;
constexpr double kHighBandMinHz = 4000.0;

// The sample rates for which the filters are designed in advance. At any
// other rate the filters of the nearest rate are used, which shifts the
// bands by the ratio of the two rates.
constexpr std::array<audio::SampleRate, 6> kSampleRates = {
        audio::SampleRate(44100),
        audio::SampleRate(48000),
        audio::SampleRate(88200),
        audio::SampleRate(96000),
        audio::SampleRate(176400),
        audio::SampleRate(192000),
};

// The levels are published at this rate at most. Engine buffers shorter
// than one period are accumulated.
constexpr int kUpdateRateHz = 60;

constexpr double kLevelEpsilon = 0.0001;

} // namespace

struct EngineBandMeter::FilterSet {
    explicit FilterSet(audio::SampleRate sampleRate)
            : m_sampleRate(sampleRate),
              m_low(sampleRate, kLowBandMaxHz),
              m_midHighPass(sampleRate, kMidBandMinHz),
              m_midLowPass(sampleRate, kMidBandMaxHz),
              m_high(sampleRate, kHighBandMinHz) {
    }

    void pause() {
        m_low.pauseFilter();
        m_midHighPass.pauseFilter();
        m_midLowPass.pauseFilter();
        m_high.pauseFilter();
    }

    const audio::SampleRate m_sampleRate;
    EngineFilterLinkwitzRiley4Low m_low;
    EngineFilterLinkwitzRiley4High m_midHighPass;
    EngineFilterLinkwitzRiley4Low m_midLowPass;
    EngineFilterLinkwitzRiley4High m_high;
};

EngineBandMeter::Band::Band(const ConfigKey& key)
        : m_level(key),
          m_sumAbs(CSAMPLE_ZERO) {
    m_level.setReadOnly();
}

EngineBandMeter::EngineBandMeter(const QString& group)
        : m_low(ConfigKey(group, QStringLiteral("band_meter_low"))),
          m_mid(ConfigKey(group, QStringLiteral("band_meter_mid"))),
          m_high(ConfigKey(group, QStringLiteral("band_meter_high"))),
          m_pFilterSet(nullptr),
          m_filtered(kMaxEngineSamples),
          m_samplesAccumulated(0),
          m_isReset(false) {
    m_filterSets.reserve(kSampleRates.size());
    for (audio::SampleRate sampleRate : kSampleRates) {
        m_filterSets.push_back(std::make_unique<FilterSet>(sampleRate));
    }
    m_pFilterSet = m_filterSets.front().get();
    reset();
}

EngineBandMeter::~EngineBandMeter() = default;

EngineBandMeter::FilterSet* EngineBandMeter::filterSetFor(audio::SampleRate sampleRate) {
    if (m_pFilterSet->m_sampleRate == sampleRate) {
        return m_pFilterSet;
    }
    FilterSet* pNearest = m_filterSets.front().get();
    for (const auto& pFilterSet : m_filterSets) {
        if (std::fabs(pFilterSet->m_sampleRate.toDouble() - sampleRate.toDouble()) <
                std::fabs(pNearest->m_sampleRate.toDouble() - sampleRate.toDouble())) {
            pNearest = pFilterSet.get();
        }
    }
    return pNearest;
}

void EngineBandMeter::process(
        const CSAMPLE* pIn, std::size_t bufferSize, audio::SampleRate sampleRate) {
    VERIFY_OR_DEBUG_ASSERT(sampleRate.isValid() &&
            bufferSize <= static_cast<std::size_t>(m_filtered.size())) {
        return;
    }
    FilterSet* pFilterSet = filterSetFor(sampleRate);
    if (pFilterSet != m_pFilterSet) {
        // These filters still hold the signal they have processed before.
        pFilterSet->pause();
        m_pFilterSet = pFilterSet;
    }
    m_isReset = false;

    CSAMPLE* pFiltered = m_filtered.data();
    pFilterSet->m_low.process(pIn, pFiltered, bufferSize);
    accumulate(&m_low, bufferSize);

    pFilterSet->m_midHighPass.process(pIn, pFiltered, bufferSize);
    pFilterSet->m_midLowPass.process(pFiltered, pFiltered, bufferSize);
    accumulate(&m_mid, bufferSize);

    pFilterSet->m_high.process(pIn, pFiltered, bufferSize);
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
    m_pFilterSet->pause();
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
    // The levels change with every update. The epsilon keeps inaudible
    // changes from waking up the listeners of the control.
    if (std::fabs(level - pBand->m_level.get()) > kLevelEpsilon) {
        pBand->m_level.forceSet(level);
    }
}

QAtomicInt EngineBandMeterRequest::s_count(0);

EngineBandMeterRequest::EngineBandMeterRequest() {
    s_count.ref();
}

EngineBandMeterRequest::~EngineBandMeterRequest() {
    s_count.deref();
}

// static
bool EngineBandMeterRequest::anyActive() {
    return s_count.loadRelaxed() > 0;
}

} // namespace mixxx
