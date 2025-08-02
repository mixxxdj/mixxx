#include "engine/enginevumeter.h"

#include "audio/types.h"
#include "moc_enginevumeter.cpp"
#include "util/sample.h"

namespace {

// Rate at which the vumeter is updated (using a sample rate of 44100 Hz):
constexpr unsigned int kVuUpdateRate = 30; // in Hz (1/s), fits to display frame rate
constexpr int kPeakDuration = 500; // in ms

// Smoothing Factors
// Must be from 0-1 the lower the factor, the more smoothing that is applied
constexpr CSAMPLE kAttackSmoothing = 1.0f; // .85
constexpr CSAMPLE kDecaySmoothing = 0.1f;  //.16//.4

} // namespace

EngineVuMeter::EngineVuMeter(const QString& group, const QString& legacyGroup)
        : m_vuMeter(ConfigKey(group, QStringLiteral("vu_meter"))),
          m_vuMeterLeft(ConfigKey(group, QStringLiteral("vu_meter_left"))),
          m_vuMeterRight(ConfigKey(group, QStringLiteral("vu_meter_right"))),
          m_peakIndicator(ConfigKey(group, QStringLiteral("peak_indicator"))),
          m_peakIndicatorLeft(ConfigKey(group, QStringLiteral("peak_indicator_left"))),
          m_peakIndicatorRight(ConfigKey(group, QStringLiteral("peak_indicator_right"))),
          m_sampleRate(QStringLiteral("[App]"), QStringLiteral("samplerate")) {
    const QString& aliasGroup = legacyGroup.isEmpty() ? group : legacyGroup;
    m_vuMeter.addAlias(ConfigKey(aliasGroup, QStringLiteral("VuMeter")));
    m_vuMeterLeft.addAlias(ConfigKey(aliasGroup, QStringLiteral("VuMeterL")));
    m_vuMeterRight.addAlias(ConfigKey(aliasGroup, QStringLiteral("VuMeterR")));
    m_peakIndicator.addAlias(ConfigKey(aliasGroup, QStringLiteral("PeakIndicator")));
    m_peakIndicatorLeft.addAlias(ConfigKey(aliasGroup, QStringLiteral("PeakIndicatorL")));
    m_peakIndicatorRight.addAlias(ConfigKey(aliasGroup, QStringLiteral("PeakIndicatorR")));
    // Initialize the calculation:
    reset();
}

void EngineVuMeter::process(CSAMPLE* pIn, const std::size_t bufferSize) {
    CSAMPLE fVolSumL, fVolSumR;

    const auto sampleRate = mixxx::audio::SampleRate::fromDouble(m_sampleRate.get());

    SampleUtil::CLIP_STATUS clipped = SampleUtil::sumAbsPerChannel(&fVolSumL,
            &fVolSumR,
            pIn,
            bufferSize);
    m_fRMSvolumeSumL += fVolSumL;
    m_fRMSvolumeSumR += fVolSumR;

    m_samplesCalculated += static_cast<unsigned int>(bufferSize / 2);

    // Are we ready to update the VU meter?:
    if (m_samplesCalculated > (sampleRate / kVuUpdateRate)) {
        doSmooth(m_fRMSvolumeL,
                std::log10(SHRT_MAX * m_fRMSvolumeSumL / (m_samplesCalculated * 1000) + 1));
        doSmooth(m_fRMSvolumeR,
                std::log10(SHRT_MAX * m_fRMSvolumeSumR / (m_samplesCalculated * 1000) + 1));

        const double epsilon = .0001;

        // Since VU meters are a rolling sum of audio, the no-op checks in
        // ControlObject will not prevent us from causing tons of extra
        // work. Because of this, we use an epsilon here to be gentle on the GUI
        // and MIDI controllers.
        if (fabs(m_fRMSvolumeL - m_vuMeterLeft.get()) > epsilon) {
            m_vuMeterLeft.set(m_fRMSvolumeL);
        }
        if (fabs(m_fRMSvolumeR - m_vuMeterRight.get()) > epsilon) {
            m_vuMeterRight.set(m_fRMSvolumeR);
        }

        double fRMSvolume = (m_fRMSvolumeL + m_fRMSvolumeR) / 2.0;
        if (fabs(fRMSvolume - m_vuMeter.get()) > epsilon) {
            m_vuMeter.set(fRMSvolume);
        }

        // Reset calculation:
        m_samplesCalculated = 0;
        m_fRMSvolumeSumL = 0;
        m_fRMSvolumeSumR = 0;
    }

    if (clipped & SampleUtil::CLIPPING_LEFT) {
        m_peakIndicatorLeft.set(1.0);
        m_peakDurationL = static_cast<int>(kPeakDuration * sampleRate / bufferSize / 2000);
    } else if (m_peakDurationL <= 0) {
        m_peakIndicatorLeft.set(0.0);
    } else {
        --m_peakDurationL;
    }

    if (clipped & SampleUtil::CLIPPING_RIGHT) {
        m_peakIndicatorRight.set(1.0);
        m_peakDurationR = static_cast<int>(kPeakDuration * sampleRate / bufferSize / 2000);
    } else if (m_peakDurationR <= 0) {
        m_peakIndicatorRight.set(0.0);
    } else {
        --m_peakDurationR;
    }

    m_peakIndicator.set(
            (m_peakIndicatorRight.toBool() || m_peakIndicatorLeft.toBool())
                    ? 1.0
                    : 0.0);
}

void EngineVuMeter::doSmooth(CSAMPLE &currentVolume, CSAMPLE newVolume)
{
    if (currentVolume > newVolume) {
        currentVolume -= kDecaySmoothing * (currentVolume - newVolume);
    } else {
        currentVolume += kAttackSmoothing * (newVolume - currentVolume);
    }
    if (currentVolume < 0) {
        currentVolume=0;
    }
    if (currentVolume > 1.0) {
        currentVolume=1.0;
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
