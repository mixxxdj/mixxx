#include "engine/enginebandmeter.h"

#include <gtest/gtest.h>

#include <QString>
#include <cmath>
#include <numbers>

#include "control/controlobject.h"
#include "test/mixxxtest.h"
#include "util/samplebuffer.h"

namespace {

const QString kGroup = QStringLiteral("[Test]");
constexpr mixxx::audio::SampleRate kSampleRate = mixxx::audio::SampleRate(44100);
constexpr std::size_t kBufferFrames = 1024;
constexpr std::size_t kBufferSamples = kBufferFrames * 2;
constexpr CSAMPLE kAmplitude = 0.5f;
// The mean absolute value of a sine is 2/pi of its amplitude. The tolerance
// allows for the attenuation near the edges of a band.
constexpr double kPassedLevel = kAmplitude * 2 / std::numbers::pi;
constexpr double kPassedTolerance = 0.05;
constexpr double kRejectedLevel = 0.005;

class EngineBandMeterTest : public MixxxTest {
  protected:
    EngineBandMeterTest()
            : m_meter(kGroup),
              m_buffer(kBufferSamples),
              m_frame(0) {
    }

    // Feeds one second of a stereo sine into the meter.
    void processSine(double frequencyHz, mixxx::audio::SampleRate sampleRate = kSampleRate) {
        const std::size_t numBuffers = sampleRate.value() / kBufferFrames;
        for (std::size_t bufferIndex = 0; bufferIndex < numBuffers; ++bufferIndex) {
            for (std::size_t frame = 0; frame < kBufferFrames; ++frame) {
                const double phase = 2 * std::numbers::pi * frequencyHz *
                        static_cast<double>(m_frame) / sampleRate.value();
                const CSAMPLE sample = kAmplitude * static_cast<CSAMPLE>(std::sin(phase));
                m_buffer[static_cast<SINT>(frame * 2)] = sample;
                m_buffer[static_cast<SINT>(frame * 2 + 1)] = sample;
                ++m_frame;
            }
            m_meter.process(m_buffer.data(), kBufferSamples, sampleRate);
        }
    }

    double level(const QString& item) const {
        return ControlObject::get(ConfigKey(kGroup, item));
    }

    mixxx::EngineBandMeter m_meter;
    mixxx::SampleBuffer m_buffer;
    std::size_t m_frame;
};

TEST_F(EngineBandMeterTest, bassOnlyRaisesLowBand) {
    processSine(60);
    EXPECT_NEAR(kPassedLevel, level(QStringLiteral("band_meter_low")), kPassedTolerance);
    EXPECT_LT(level(QStringLiteral("band_meter_mid")), kRejectedLevel);
    EXPECT_LT(level(QStringLiteral("band_meter_high")), kRejectedLevel);
}

TEST_F(EngineBandMeterTest, midsOnlyRaiseMidBand) {
    processSine(800);
    EXPECT_LT(level(QStringLiteral("band_meter_low")), kRejectedLevel);
    EXPECT_NEAR(kPassedLevel, level(QStringLiteral("band_meter_mid")), kPassedTolerance);
    EXPECT_LT(level(QStringLiteral("band_meter_high")), kRejectedLevel);
}

TEST_F(EngineBandMeterTest, highsOnlyRaiseHighBand) {
    processSine(10000);
    EXPECT_LT(level(QStringLiteral("band_meter_low")), kRejectedLevel);
    EXPECT_LT(level(QStringLiteral("band_meter_mid")), kRejectedLevel);
    EXPECT_NEAR(kPassedLevel, level(QStringLiteral("band_meter_high")), kPassedTolerance);
}

TEST_F(EngineBandMeterTest, silenceReadsZero) {
    processSine(60);
    m_buffer.clear();
    for (int bufferIndex = 0; bufferIndex < 50; ++bufferIndex) {
        m_meter.process(m_buffer.data(), kBufferSamples, kSampleRate);
    }
    EXPECT_LT(level(QStringLiteral("band_meter_low")), kRejectedLevel);
}

TEST_F(EngineBandMeterTest, resetReadsZero) {
    processSine(60);
    processSine(800);
    processSine(10000);
    m_meter.reset();
    EXPECT_DOUBLE_EQ(0.0, level(QStringLiteral("band_meter_low")));
    EXPECT_DOUBLE_EQ(0.0, level(QStringLiteral("band_meter_mid")));
    EXPECT_DOUBLE_EQ(0.0, level(QStringLiteral("band_meter_high")));
}

TEST_F(EngineBandMeterTest, shortBuffersAreAccumulated) {
    // 64 frames are shorter than one update period, so the level must still be
    // published after several buffers.
    constexpr std::size_t kShortBufferSamples = 64 * 2;
    for (SINT sampleIndex = 0; sampleIndex < static_cast<SINT>(kShortBufferSamples);
            ++sampleIndex) {
        // A 11025 Hz square wave passes the high band only.
        m_buffer[sampleIndex] = (sampleIndex / 4) % 2 == 0 ? kAmplitude : -kAmplitude;
    }
    for (int bufferIndex = 0; bufferIndex < 200; ++bufferIndex) {
        m_meter.process(m_buffer.data(), kShortBufferSamples, kSampleRate);
    }
    EXPECT_GT(level(QStringLiteral("band_meter_high")), 0.1);
}

TEST_F(EngineBandMeterTest, bandsAreTheSameAtOtherSampleRates) {
    for (int rate : {48000, 96000, 192000}) {
        const mixxx::audio::SampleRate sampleRate(rate);
        processSine(60, sampleRate);
        EXPECT_NEAR(kPassedLevel, level(QStringLiteral("band_meter_low")), kPassedTolerance);
        EXPECT_LT(level(QStringLiteral("band_meter_mid")), kRejectedLevel);
        EXPECT_LT(level(QStringLiteral("band_meter_high")), kRejectedLevel);

        processSine(800, sampleRate);
        EXPECT_LT(level(QStringLiteral("band_meter_low")), kRejectedLevel);
        EXPECT_NEAR(kPassedLevel, level(QStringLiteral("band_meter_mid")), kPassedTolerance);
        EXPECT_LT(level(QStringLiteral("band_meter_high")), kRejectedLevel);

        processSine(10000, sampleRate);
        EXPECT_LT(level(QStringLiteral("band_meter_low")), kRejectedLevel);
        EXPECT_LT(level(QStringLiteral("band_meter_mid")), kRejectedLevel);
        EXPECT_NEAR(kPassedLevel, level(QStringLiteral("band_meter_high")), kPassedTolerance);
    }
}

TEST_F(EngineBandMeterTest, unusualSampleRateUsesTheNearestFilters) {
    // 50 kHz is not a rate the filters are designed for. The bands of the
    // filters for 48 kHz are only shifted by 4 % there.
    const mixxx::audio::SampleRate sampleRate(50000);
    processSine(60, sampleRate);
    EXPECT_NEAR(kPassedLevel, level(QStringLiteral("band_meter_low")), kPassedTolerance);
    processSine(10000, sampleRate);
    EXPECT_LT(level(QStringLiteral("band_meter_low")), kRejectedLevel);
    EXPECT_NEAR(kPassedLevel, level(QStringLiteral("band_meter_high")), kPassedTolerance);
}

TEST(EngineBandMeterRequestTest, countsRequests) {
    EXPECT_FALSE(mixxx::EngineBandMeterRequest::anyActive());
    {
        const mixxx::EngineBandMeterRequest first;
        EXPECT_TRUE(mixxx::EngineBandMeterRequest::anyActive());
        {
            const mixxx::EngineBandMeterRequest second;
            EXPECT_TRUE(mixxx::EngineBandMeterRequest::anyActive());
        }
        EXPECT_TRUE(mixxx::EngineBandMeterRequest::anyActive());
    }
    EXPECT_FALSE(mixxx::EngineBandMeterRequest::anyActive());
}

} // namespace
