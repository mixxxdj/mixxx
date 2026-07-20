#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <QString>
#include <vector>

#include "control/controlobject.h"
#include "control/controlproxy.h"
#include "engine/enginevumeter.h"
#include "test/mixxxtest.h"
#include "util/sample.h"

namespace {

constexpr int kSampleRate = 44100;
constexpr int kVuUpdateFrames = kSampleRate / 30;
constexpr int kBufferFrames = 512;
constexpr double kMinDb = -60.0;
constexpr double kDbRange = 60.0;
constexpr double kDjReleaseDbPerSecond = 30.0;

QString makeGroup() {
    static int groupIndex = 0;
    return QStringLiteral("[EngineVuMeterTest%1]").arg(++groupIndex);
}

void setSampleRate() {
    ControlProxy sampleRate(QStringLiteral("[App]"), QStringLiteral("samplerate"));
    sampleRate.set(kSampleRate);
}

std::vector<CSAMPLE> makeStereoBuffer(int frames, CSAMPLE left, CSAMPLE right) {
    std::vector<CSAMPLE> buffer(frames * 2);
    for (int i = 0; i < frames; ++i) {
        buffer[i * 2] = left;
        buffer[i * 2 + 1] = right;
    }
    return buffer;
}

void processSilence(EngineVuMeter* pMeter, int buffers) {
    auto silence = makeStereoBuffer(kBufferFrames, 0.0f, 0.0f);
    for (int i = 0; i < buffers; ++i) {
        pMeter->process(silence.data(), silence.size());
    }
}

double levelFromAmplitude(double amplitude) {
    if (amplitude <= 0.0) {
        return 0.0;
    }
    const double db = 20.0 * std::log10(
            std::min<double>(amplitude, CSAMPLE_PEAK));
    return std::clamp((db - kMinDb) / kDbRange, 0.0, 1.0);
}

class EngineVuMeterTest : public MixxxTest {
  protected:
    EngineVuMeterTest()
            : m_vuMeterMode(EngineVuMeter::modeConfigKey(),
                      true,
                      false,
                      false,
                      EngineVuMeter::modeToValue(EngineVuMeter::MeterMode::DjPeak)) {
    }

    void setMeterMode(EngineVuMeter::MeterMode mode) {
        m_vuMeterMode.set(EngineVuMeter::modeToValue(mode));
    }

  private:
    ControlObject m_vuMeterMode;
};

TEST_F(EngineVuMeterTest, ShortTransientUsesPeakLevel) {
    setSampleRate();

    const QString group = makeGroup();
    EngineVuMeter meter(group, QString(), false);
    ControlProxy vuMeter(group, QStringLiteral("vu_meter"));
    ControlProxy vuMeterLeft(group, QStringLiteral("vu_meter_left"));
    ControlProxy vuMeterRight(group, QStringLiteral("vu_meter_right"));

    auto buffer = makeStereoBuffer(kVuUpdateFrames, 0.0f, 0.0f);
    buffer[0] = CSAMPLE_PEAK;
    buffer[1] = CSAMPLE_PEAK;

    meter.process(buffer.data(), buffer.size());

    EXPECT_GT(vuMeter.get(), 0.99);
    EXPECT_GT(vuMeterLeft.get(), 0.99);
    EXPECT_GT(vuMeterRight.get(), 0.99);
}

TEST_F(EngineVuMeterTest, DjPeakModeReleasesAtConstantDbPerSecond) {
    setSampleRate();
    setMeterMode(EngineVuMeter::MeterMode::DjPeak);

    const QString group = makeGroup();
    EngineVuMeter meter(group, QString(), false);
    ControlProxy vuMeter(group, QStringLiteral("vu_meter"));

    // Saturate the meter to full scale (0 dBFS).
    auto fullScale = makeStereoBuffer(kVuUpdateFrames, CSAMPLE_PEAK, CSAMPLE_PEAK);
    for (int i = 0; i < 5; ++i) {
        meter.process(fullScale.data(), fullScale.size());
    }
    const double levelBefore = vuMeter.get();
    ASSERT_GT(levelBefore, 0.99);

    // Release for a known duration of silence and check the level dropped by
    // the expected dB amount (constant dB/s release mapped onto the meter
    // range). Each buffer holds exactly one update window so the elapsed time
    // per fire is well defined.
    constexpr int kReleaseWindows = 10;
    auto silence = makeStereoBuffer(kVuUpdateFrames, 0.0f, 0.0f);
    for (int i = 0; i < kReleaseWindows; ++i) {
        meter.process(silence.data(), silence.size());
    }

    const double secondsElapsed = static_cast<double>(
            kReleaseWindows * kVuUpdateFrames) / kSampleRate;
    const double expectedDrop = kDjReleaseDbPerSecond * secondsElapsed / kDbRange;
    EXPECT_NEAR(vuMeter.get(), levelBefore - expectedDrop, 0.02);
}

TEST_F(EngineVuMeterTest, VuRmsModeDoesNotTreatSingleSampleTransientAsFullScale) {
    setSampleRate();
    setMeterMode(EngineVuMeter::MeterMode::VuRms);

    const QString group = makeGroup();
    EngineVuMeter meter(group, QString(), false);
    ControlProxy vuMeter(group, QStringLiteral("vu_meter"));

    auto buffer = makeStereoBuffer(kVuUpdateFrames, 0.0f, 0.0f);
    buffer[0] = CSAMPLE_PEAK;
    buffer[1] = CSAMPLE_PEAK;

    meter.process(buffer.data(), buffer.size());

    EXPECT_LT(vuMeter.get(), 0.25);
}

TEST_F(EngineVuMeterTest, VuRmsModeUsesSteadyRmsLevel) {
    setSampleRate();
    setMeterMode(EngineVuMeter::MeterMode::VuRms);

    const QString group = makeGroup();
    EngineVuMeter meter(group, QString(), false);
    ControlProxy vuMeter(group, QStringLiteral("vu_meter"));

    auto buffer = makeStereoBuffer(kVuUpdateFrames, 0.5f, 0.5f);
    for (int i = 0; i < 20; ++i) {
        meter.process(buffer.data(), buffer.size());
    }

    EXPECT_NEAR(vuMeter.get(), levelFromAmplitude(0.5), 0.02);
}

TEST_F(EngineVuMeterTest, SwitchingModeResetsMeterState) {
    setSampleRate();

    const QString group = makeGroup();
    EngineVuMeter meter(group, QString(), false);
    ControlProxy vuMeter(group, QStringLiteral("vu_meter"));

    auto buffer = makeStereoBuffer(kVuUpdateFrames, 0.0f, 0.0f);
    buffer[0] = CSAMPLE_PEAK;
    buffer[1] = CSAMPLE_PEAK;

    meter.process(buffer.data(), buffer.size());
    ASSERT_GT(vuMeter.get(), 0.99);

    setMeterMode(EngineVuMeter::MeterMode::VuRms);
    meter.process(buffer.data(), buffer.size());

    EXPECT_LT(vuMeter.get(), 0.25);
}

TEST_F(EngineVuMeterTest, ClipIndicatorHoldUsesElapsedFrames) {
    setSampleRate();

    const QString group = makeGroup();
    EngineVuMeter meter(group, QString(), false);
    ControlProxy peakIndicator(group, QStringLiteral("peak_indicator"));
    ControlProxy peakIndicatorLeft(group, QStringLiteral("peak_indicator_left"));

    auto clipped = makeStereoBuffer(kBufferFrames, 0.0f, 0.0f);
    clipped[0] = CSAMPLE_PEAK + 0.1f;
    meter.process(clipped.data(), clipped.size());

    EXPECT_DOUBLE_EQ(peakIndicatorLeft.get(), 1.0);
    EXPECT_DOUBLE_EQ(peakIndicator.get(), 1.0);

    processSilence(&meter, 20);
    EXPECT_DOUBLE_EQ(peakIndicatorLeft.get(), 1.0);
    EXPECT_DOUBLE_EQ(peakIndicator.get(), 1.0);

    processSilence(&meter, 24);
    EXPECT_DOUBLE_EQ(peakIndicatorLeft.get(), 0.0);
    EXPECT_DOUBLE_EQ(peakIndicator.get(), 0.0);
}

TEST(EngineVuMeterModeTest, ModeValueRoundTripAndClamping) {
    // modeToValue/modeFromValue round-trip for every defined mode.
    EXPECT_EQ(EngineVuMeter::modeFromValue(
                      EngineVuMeter::modeToValue(EngineVuMeter::MeterMode::DjPeak)),
            EngineVuMeter::MeterMode::DjPeak);
    EXPECT_EQ(EngineVuMeter::modeFromValue(
                      EngineVuMeter::modeToValue(EngineVuMeter::MeterMode::VuRms)),
            EngineVuMeter::MeterMode::VuRms);

    // Out-of-range or unknown values fall back to the default DjPeak mode.
    EXPECT_EQ(EngineVuMeter::modeFromValue(-1.0), EngineVuMeter::MeterMode::DjPeak);
    EXPECT_EQ(EngineVuMeter::modeFromValue(99.0), EngineVuMeter::MeterMode::DjPeak);

    // Non-integer values round to the nearest mode.
    EXPECT_EQ(EngineVuMeter::modeFromValue(0.4), EngineVuMeter::MeterMode::DjPeak);
    EXPECT_EQ(EngineVuMeter::modeFromValue(1.4), EngineVuMeter::MeterMode::VuRms);
}

} // namespace
