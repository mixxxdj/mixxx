#include "engine/enginevumeter.h"

#include "audio/types.h"
#include "moc_enginevumeter.cpp"
#include "util/sample.h"

namespace {

// Rate at which the vumeter is updated (using a sample rate of 44100 Hz):
constexpr unsigned int kVuUpdateRate = 30; // in Hz (1/s), fits to display frame rate
constexpr float kPeakHoldSeconds = 0.5f;   // in s

// Factor used to scale the normalized float RMS sum to a legacy 16-bit range
// for the logarithmic VU meter calculation. 32767 is SHRT_MAX.
constexpr float kLegacyLogScaleFactor = 32.767f;

// Smoothing Factors
// Must be from 0-1 the lower the factor, the more smoothing that is applied
constexpr CSAMPLE kAttackSmoothing = 1.0f; // .85
constexpr CSAMPLE kDecaySmoothing = 0.1f;  //.16//.4

} // namespace

EngineVuMeter::EngineVuMeter(const QString& group,
        const QString& legacyGroup,
        bool createLegacyAliases)
        : m_vuMeter(ConfigKey(group, QStringLiteral("vu_meter"))),
          m_vuMeterLeft(ConfigKey(group, QStringLiteral("vu_meter_left"))),
          m_vuMeterRight(ConfigKey(group, QStringLiteral("vu_meter_right"))),
          m_peakIndicator(ConfigKey(group, QStringLiteral("peak_indicator"))),
          m_peakIndicatorLeft(
                  ConfigKey(group, QStringLiteral("peak_indicator_left"))),
          m_peakIndicatorRight(
                  ConfigKey(group, QStringLiteral("peak_indicator_right"))),
          m_sampleRate(QStringLiteral("[App]"), QStringLiteral("samplerate")) {
    if (createLegacyAliases) {
        const QString& aliasGroup = legacyGroup.isEmpty() ? group : legacyGroup;
        m_vuMeter.addAlias(ConfigKey(aliasGroup, QStringLiteral("VuMeter")));
        m_vuMeterLeft.addAlias(ConfigKey(aliasGroup, QStringLiteral("VuMeterL")));
        m_vuMeterRight.addAlias(ConfigKey(aliasGroup, QStringLiteral("VuMeterR")));
        m_peakIndicator.addAlias(ConfigKey(aliasGroup, QStringLiteral("PeakIndicator")));
        m_peakIndicatorLeft.addAlias(ConfigKey(aliasGroup, QStringLiteral("PeakIndicatorL")));
        m_peakIndicatorRight.addAlias(ConfigKey(aliasGroup, QStringLiteral("PeakIndicatorR")));
    }
    // Initialize the calculation:
    reset();
}

void EngineVuMeter::process(CSAMPLE* pIn, const std::size_t bufferSize) {
    CSAMPLE fVolSumL, fVolSumR;
    const mixxx::audio::SampleRate sampleRate =
            mixxx::audio::SampleRate::fromDouble(m_sampleRate.get());
    const SampleUtil::CLIP_STATUS clipped =
            SampleUtil::sumAbsPerChannel(&fVolSumL, &fVolSumR, pIn, bufferSize);
    processFused(fVolSumL, fVolSumR, clipped, sampleRate, bufferSize);
}

void EngineVuMeter::processFused(
        CSAMPLE fVolSumL,
        CSAMPLE fVolSumR,
        SampleUtil::CLIP_STATUS clipped,
        mixxx::audio::SampleRate sampleRate,
        std::size_t bufferSize) {
    m_fRMSvolumeSumL += fVolSumL;
    m_fRMSvolumeSumR += fVolSumR;

    m_samplesCalculated += static_cast<unsigned int>(bufferSize / 2);

    // Are we ready to update the VU meter?:
    if (m_samplesCalculated > (sampleRate / kVuUpdateRate)) {
        const float samplesCalculatedFloat = static_cast<float>(m_samplesCalculated);

        doSmooth(m_fRMSvolumeL,
                std::log10(static_cast<float>(kLegacyLogScaleFactor) * (m_fRMSvolumeSumL / samplesCalculatedFloat) + 1.0f));
        doSmooth(m_fRMSvolumeR,
                std::log10(static_cast<float>(kLegacyLogScaleFactor) * (m_fRMSvolumeSumR / samplesCalculatedFloat) + 1.0f));

        const CSAMPLE epsilon = 0.0001f;

        // Since VU meters are a rolling sum of audio, the no-op checks in
        // ControlObject will not prevent us from causing tons of extra
        // work. Because of this, we use an epsilon here to be gentle on the GUI
        // and MIDI controllers.
        if (std::abs(m_fRMSvolumeL - static_cast<CSAMPLE>(m_vuMeterLeft.get())) > epsilon) {
            m_vuMeterLeft.set(m_fRMSvolumeL);
        }
        if (std::abs(m_fRMSvolumeR - static_cast<CSAMPLE>(m_vuMeterRight.get())) > epsilon) {
            m_vuMeterRight.set(m_fRMSvolumeR);
        }

        const CSAMPLE fRMSvolume = (m_fRMSvolumeL + m_fRMSvolumeR) * 0.5f;
        if (std::abs(fRMSvolume - static_cast<CSAMPLE>(m_vuMeter.get())) > epsilon) {
            m_vuMeter.set(fRMSvolume);
        }

        // Reset calculation:
        m_samplesCalculated = 0;
        m_fRMSvolumeSumL = 0;
        m_fRMSvolumeSumR = 0;
    }

    if (clipped & SampleUtil::CLIPPING_LEFT) {
        m_peakIndicatorLeft.set(1.0f);
        m_peakDurationL = static_cast<int>(
                std::round(kPeakHoldSeconds * static_cast<float>(sampleRate) /
                        (static_cast<float>(bufferSize) / 2.0f)));
    } else if (m_peakDurationL <= 0) {
        m_peakIndicatorLeft.set(0.0f);
    } else {
        --m_peakDurationL;
    }

    if (clipped & SampleUtil::CLIPPING_RIGHT) {
        m_peakIndicatorRight.set(1.0f);
        m_peakDurationR = static_cast<int>(
                std::round(kPeakHoldSeconds * static_cast<float>(sampleRate) /
                        (static_cast<float>(bufferSize) / 2.0f)));
    } else if (m_peakDurationR <= 0) {
        m_peakIndicatorRight.set(0.0f);
    } else {
        --m_peakDurationR;
    }

    m_peakIndicator.set(
            (m_peakIndicatorRight.toBool() || m_peakIndicatorLeft.toBool())
                    ? 1.0f
                    : 0.0f);
}

void EngineVuMeter::doSmooth(CSAMPLE& currentVolume, CSAMPLE newVolume) {
    if (currentVolume > newVolume) {
        currentVolume -= kDecaySmoothing * (currentVolume - newVolume);
    } else {
        currentVolume += kAttackSmoothing * (newVolume - currentVolume);
    }
    if (currentVolume < 0) {
        currentVolume = 0;
    }
    if (currentVolume > 1.0f) {
        currentVolume = 1.0f;
    }
}

void EngineVuMeter::reset() {
    m_vuMeter.set(0);
    m_vuMeterLeft.set(0);
    m_vuMeterRight.set(0);
    m_peakIndicator.set(0);
    m_peakIndicatorLeft.set(0);
    m_peakIndicatorRight.set(0);

    m_samplesCalculated = 0;
    m_fRMSvolumeL = 0;
    m_fRMSvolumeSumL = 0;
    m_fRMSvolumeR = 0;
    m_fRMSvolumeSumR = 0;
    m_peakDurationL = 0;
    m_peakDurationR = 0;
}
