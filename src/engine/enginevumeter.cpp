#include "engine/enginevumeter.h"

#include <algorithm>
#include <cmath>

#include "audio/types.h"
#include "moc_enginevumeter.cpp"

namespace {

// Rate at which the vumeter is updated. The audio envelope itself is sampled
// continuously and only the ControlObjects are throttled to this rate.
constexpr std::size_t kVuUpdateRate = 30; // in Hz (1/s), fits to display frame rate
constexpr std::size_t kPeakDuration = 500; // in ms

// Mixer-style peak meter ballistics. The level meter uses fast attack and a
// constant dB/s release so transient spacing does not get folded into the
// measured level before the display smoothing is applied.
constexpr double kDjAttackTimeSeconds = 0.005;
constexpr double kDjReleaseDbPerSecond = 30.0;

// VU/RMS mode uses true RMS values with a one-pole VU-style 300 ms rise time.
// The time constant below makes a step input reach 99% after 300 ms.
constexpr double kVuIntegrationTimeConstantSeconds = 0.06514476;

constexpr double kMinDb = -60.0;
constexpr double kMaxDb = 0.0;
constexpr double kDbRange = kMaxDb - kMinDb;

CSAMPLE levelFromAmplitude(double amplitude) {
    if (amplitude <= 0.0) {
        return CSAMPLE_ZERO;
    }
    const double clampedAmplitude = std::min<double>(amplitude, CSAMPLE_PEAK);
    const double db = 20.0 * std::log10(clampedAmplitude);
    return static_cast<CSAMPLE>(
            std::clamp((db - kMinDb) / kDbRange, 0.0, 1.0));
}

void smoothDjLevel(
        CSAMPLE& currentVolume,
        CSAMPLE newVolume,
        double secondsElapsed) {
    if (currentVolume <= newVolume) {
        const double attack = 1.0 - std::exp(-secondsElapsed / kDjAttackTimeSeconds);
        currentVolume += static_cast<CSAMPLE>(attack * (newVolume - currentVolume));
    } else {
        const double release = kDjReleaseDbPerSecond * secondsElapsed / kDbRange;
        currentVolume = static_cast<CSAMPLE>(
                std::max<double>(newVolume, currentVolume - release));
    }
    currentVolume = static_cast<CSAMPLE>(
            std::clamp<double>(currentVolume, 0.0, 1.0));
}

void smoothVuLevel(
        CSAMPLE& currentVolume,
        CSAMPLE newVolume,
        double secondsElapsed) {
    const double coefficient = 1.0 -
            std::exp(-secondsElapsed / kVuIntegrationTimeConstantSeconds);
    currentVolume += static_cast<CSAMPLE>(coefficient * (newVolume - currentVolume));
    currentVolume = static_cast<CSAMPLE>(
            std::clamp<double>(currentVolume, 0.0, 1.0));
}

} // namespace

const ConfigKey& EngineVuMeter::modeConfigKey() {
    static const ConfigKey kModeConfigKey(
            QStringLiteral("[Mixer Profile]"), QStringLiteral("VuMeterMode"));
    return kModeConfigKey;
}

EngineVuMeter::MeterMode EngineVuMeter::modeFromValue(double value) {
    switch (static_cast<int>(std::lround(value))) {
    case static_cast<int>(MeterMode::VuRms):
        return MeterMode::VuRms;
    case static_cast<int>(MeterMode::DjPeak):
    default:
        return MeterMode::DjPeak;
    }
}

double EngineVuMeter::modeToValue(MeterMode mode) {
    return static_cast<double>(static_cast<int>(mode));
}

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
          m_sampleRate(QStringLiteral("[App]"), QStringLiteral("samplerate")),
          m_meterMode(modeConfigKey(), ControlFlag::NoWarnIfMissing) {
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
    updateMeterMode();

    const auto sampleRate = mixxx::audio::SampleRate::fromDouble(m_sampleRate.get());
    const auto sampleRateValue = static_cast<std::size_t>(sampleRate.value());
    const std::size_t frameCount = bufferSize / 2;
    if (!sampleRate.isValid() || frameCount == 0) {
        return;
    }

    CSAMPLE peakL = CSAMPLE_ZERO;
    CSAMPLE peakR = CSAMPLE_ZERO;
    CSAMPLE squareSumL = CSAMPLE_ZERO;
    CSAMPLE squareSumR = CSAMPLE_ZERO;
    bool clippedL = false;
    bool clippedR = false;
    for (std::size_t i = 0; i < frameCount; ++i) {
        const CSAMPLE sampleL = pIn[i * 2];
        const CSAMPLE absL = std::abs(sampleL);
        peakL = std::max(peakL, absL);
        squareSumL += sampleL * sampleL;
        clippedL = clippedL || absL > CSAMPLE_PEAK;

        const CSAMPLE sampleR = pIn[i * 2 + 1];
        const CSAMPLE absR = std::abs(sampleR);
        peakR = std::max(peakR, absR);
        squareSumR += sampleR * sampleR;
        clippedR = clippedR || absR > CSAMPLE_PEAK;
    }

    m_peakVolumeL = std::max(m_peakVolumeL, peakL);
    m_peakVolumeR = std::max(m_peakVolumeR, peakR);
    m_squareSumL += squareSumL;
    m_squareSumR += squareSumR;
    m_framesCalculated += frameCount;

    const auto updateFrameCount = std::max<std::size_t>(1, sampleRateValue / kVuUpdateRate);
    if (m_framesCalculated >= updateFrameCount) {
        const double secondsElapsed = static_cast<double>(m_framesCalculated) /
                static_cast<double>(sampleRateValue);
        setMeterControls(m_meterCalculator.process({m_peakVolumeL,
                m_peakVolumeR,
                m_squareSumL,
                m_squareSumR,
                m_framesCalculated,
                secondsElapsed}));

        m_framesCalculated = 0;
        m_peakVolumeL = CSAMPLE_ZERO;
        m_peakVolumeR = CSAMPLE_ZERO;
        m_squareSumL = CSAMPLE_ZERO;
        m_squareSumR = CSAMPLE_ZERO;
    }

    if (clippedL) {
        m_peakIndicatorLeft.set(1.0);
        m_peakDurationFramesL = kPeakDuration * sampleRateValue / 1000;
    } else if (m_peakDurationFramesL <= frameCount) {
        m_peakIndicatorLeft.set(0.0);
        m_peakDurationFramesL = 0;
    } else {
        m_peakDurationFramesL -= frameCount;
    }

    if (clippedR) {
        m_peakIndicatorRight.set(1.0);
        m_peakDurationFramesR = kPeakDuration * sampleRateValue / 1000;
    } else if (m_peakDurationFramesR <= frameCount) {
        m_peakIndicatorRight.set(0.0);
        m_peakDurationFramesR = 0;
    } else {
        m_peakDurationFramesR -= frameCount;
    }

    setPeakIndicatorControls();
}

EngineVuMeter::StereoLevel EngineVuMeter::MeterCalculatorBase::process(
        const MeterInput& input) {
    // No samples in this window: keep the displayed level unchanged. This also
    // guards the RMS division against a zero frame count.
    if (input.frameCount == 0) {
        return {m_volumeLeft, m_volumeRight};
    }
    smooth(m_volumeLeft,
            targetLevel(input.peakLeft, input.squareSumLeft, input.frameCount),
            input.secondsElapsed);
    smooth(m_volumeRight,
            targetLevel(input.peakRight, input.squareSumRight, input.frameCount),
            input.secondsElapsed);
    return {m_volumeLeft, m_volumeRight};
}

void EngineVuMeter::MeterCalculatorBase::reset() {
    m_volumeLeft = 0;
    m_volumeRight = 0;
}

CSAMPLE EngineVuMeter::DjPeakMeterCalculator::targetLevel(
        CSAMPLE peak, CSAMPLE /*squareSum*/, std::size_t /*frameCount*/) const {
    return levelFromAmplitude(peak);
}

void EngineVuMeter::DjPeakMeterCalculator::smooth(
        CSAMPLE& currentVolume, CSAMPLE targetLevel, double secondsElapsed) const {
    smoothDjLevel(currentVolume, targetLevel, secondsElapsed);
}

CSAMPLE EngineVuMeter::VuRmsMeterCalculator::targetLevel(
        CSAMPLE /*peak*/, CSAMPLE squareSum, std::size_t frameCount) const {
    // frameCount is guaranteed non-zero by MeterCalculatorBase::process().
    const double rms = std::sqrt(
            static_cast<double>(squareSum) / static_cast<double>(frameCount));
    return levelFromAmplitude(rms);
}

void EngineVuMeter::VuRmsMeterCalculator::smooth(
        CSAMPLE& currentVolume, CSAMPLE targetLevel, double secondsElapsed) const {
    smoothVuLevel(currentVolume, targetLevel, secondsElapsed);
}

void EngineVuMeter::MeterCalculator::setMode(MeterMode mode) {
    m_mode = mode;
    reset();
}

EngineVuMeter::MeterCalculatorBase& EngineVuMeter::MeterCalculator::activeCalculator() {
    switch (m_mode) {
    case MeterMode::VuRms:
        return m_vuRmsMeter;
    case MeterMode::DjPeak:
    default:
        return m_djPeakMeter;
    }
}

EngineVuMeter::StereoLevel EngineVuMeter::MeterCalculator::process(
        const MeterInput& input) {
    return activeCalculator().process(input);
}

void EngineVuMeter::MeterCalculator::reset() {
    m_djPeakMeter.reset();
    m_vuRmsMeter.reset();
}

void EngineVuMeter::updateMeterMode() {
    const MeterMode mode = modeFromValue(m_meterMode.get());
    if (mode == m_meterCalculator.mode()) {
        return;
    }
    m_meterCalculator.setMode(mode);
    m_framesCalculated = 0;
    m_peakVolumeL = CSAMPLE_ZERO;
    m_peakVolumeR = CSAMPLE_ZERO;
    m_squareSumL = CSAMPLE_ZERO;
    m_squareSumR = CSAMPLE_ZERO;
    setMeterControls({0, 0});
}

void EngineVuMeter::setMeterControls(StereoLevel level) {
    constexpr double kEpsilon = 0.0001;

    if (std::abs(level.left - m_vuMeterLeft.get()) > kEpsilon) {
        m_vuMeterLeft.set(level.left);
    }
    if (std::abs(level.right - m_vuMeterRight.get()) > kEpsilon) {
        m_vuMeterRight.set(level.right);
    }

    const double volume = (level.left + level.right) / 2.0;
    if (std::abs(volume - m_vuMeter.get()) > kEpsilon) {
        m_vuMeter.set(volume);
    }
}

void EngineVuMeter::setPeakIndicatorControls() {
    m_peakIndicator.set(
            (m_peakIndicatorRight.toBool() || m_peakIndicatorLeft.toBool())
                    ? 1.0
                    : 0.0);
}

void EngineVuMeter::reset() {
    m_vuMeter.set(0);
    m_vuMeterLeft.set(0);
    m_vuMeterRight.set(0);
    m_peakIndicator.set(0);
    m_peakIndicatorLeft.set(0);
    m_peakIndicatorRight.set(0);

    m_meterCalculator.setMode(modeFromValue(m_meterMode.get()));
    m_framesCalculated = 0;
    m_peakVolumeL = 0;
    m_peakVolumeR = 0;
    m_squareSumL = 0;
    m_squareSumR = 0;
    m_peakDurationFramesL = 0;
    m_peakDurationFramesR = 0;
}
