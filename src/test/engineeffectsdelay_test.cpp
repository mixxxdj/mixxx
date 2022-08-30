// Tests for engineeffectsdelay.cpp

// Premise: internal Mixxx structure works with a stereo signal.
// If the mixxx::kEngineChannelCount wouldn't be a stereo in the future,
// tests have to be updated.

#include "engine/effects/engineeffectsdelay.h"

#include <benchmark/benchmark.h>
#include <gtest/gtest.h>

#include <QTest>
#include <QtDebug>
#include <span>

#include "engine/engine.h"
#include "test/mixxxtest.h"
#include "util/sample.h"
#include "util/samplebuffer.h"
#include "util/types.h"

namespace {

static_assert(mixxx::kEngineChannelCount == mixxx::audio::ChannelCount::stereo(),
        "EngineEffectsDelayTest requires stereo input signal.");

class EngineEffectsDelayTest : public MixxxTest {
  protected:
    void AssertIdenticalBufferEquals(const std::span<CSAMPLE> buffer,
            const std::span<const CSAMPLE> referenceBuffer) {
        ASSERT_EQ(buffer.size(), referenceBuffer.size());

        for (std::span<CSAMPLE>::size_type i = 0; i < buffer.size(); ++i) {
            EXPECT_FLOAT_EQ(buffer[i], referenceBuffer[i]);
        }
    }

    EngineEffectsDelay m_effectsDelay;
};

//Test's purpose is to test clamping of the delay value in setter (lower bound).
TEST_F(EngineEffectsDelayTest, NegativeDelayValue) {
#ifdef MIXXX_DEBUG_ASSERTIONS_ENABLED
    // Set thread safe for EXPECT_DEATH.
    GTEST_FLAG_SET(death_test_style, "threadsafe");

    EXPECT_DEATH({
        // Set negative delay value.
        m_effectsDelay.setDelayFrames(-1);
    },
            "delayFrames >= 0");
#else
    const SINT numSamples = 4;

    // Set negative delay value.
    m_effectsDelay.setDelayFrames(-1);

    const CSAMPLE inputBuffer[] = {-100.0, 100.0, -99.0, 99.0};
    const CSAMPLE expectedResult[] = {-100.0, 100.0, -99.0, 99.0};

    mixxx::SampleBuffer inOutBuffer(numSamples);
    SampleUtil::copy(inOutBuffer.data(), inputBuffer, numSamples);

    m_effectsDelay.process(inOutBuffer.data(), numSamples);
    AssertIdenticalBufferEquals(inOutBuffer.span(), expectedResult);
#endif
}

//Test's purpose is to test clamping of the delay value in setter (upper bound).
TEST_F(EngineEffectsDelayTest, DelayGreaterThanDelayBufferSize) {
    const SINT numDelayFrames = mixxx::audio::SampleRate::kValueMax + 2;

#ifdef MIXXX_DEBUG_ASSERTIONS_ENABLED
    // Set thread safe for EXPECT_DEATH.
    GTEST_FLAG_SET(death_test_style, "threadsafe");

    EXPECT_DEATH({
        // Set delay greater than the size of the delay buffer.
        m_effectsDelay.setDelayFrames(numDelayFrames);
    },
            "delayFrames <= kMaxDelayFrames");
#else
    const SINT numSamples = 4;

    // Set delay greater than the size of the delay buffer.
    m_effectsDelay.setDelayFrames(numDelayFrames);

    const CSAMPLE inputBuffer[] = {-100.0, 100.0, -99.0, 99.0};
    const CSAMPLE expectedResult[] = {-50.0, 50.0, -49.5, 49.5};

    mixxx::SampleBuffer inOutBuffer(numSamples);
    SampleUtil::copy(inOutBuffer.data(), inputBuffer, numSamples);

    m_effectsDelay.process(inOutBuffer.data(), numSamples);
    AssertIdenticalBufferEquals(inOutBuffer.span(), expectedResult);
#endif
}

TEST_F(EngineEffectsDelayTest, WholeBufferDelay) {
    const SINT numDelayFrames = 2;
    const SINT numSamples = 4;

    // Set delay same as the size of the input buffer.
    m_effectsDelay.setDelayFrames(numDelayFrames);

    const CSAMPLE inputBuffer[] = {-100.0, 100.0, -99.0, 99.0};
    const CSAMPLE zeroBuffer[] = {0.0, 0.0, 0.0, 0.0};
    const CSAMPLE firstExpectedResult[] = {-50.0, 50.0, -49.5, 49.5};

    const CSAMPLE secondExpectedResult[] = {-50.0, 50.0, -99.0, 99.0};
    const CSAMPLE thirdExpectedResult[] = {0.0, 0.0, 0.0, 0.0};

    mixxx::SampleBuffer inOutBuffer(numSamples);

    SampleUtil::copy(inOutBuffer.data(), inputBuffer, numSamples);
    m_effectsDelay.process(inOutBuffer.data(), numSamples);
    AssertIdenticalBufferEquals(inOutBuffer.span(), firstExpectedResult);

    SampleUtil::copy(inOutBuffer.data(), zeroBuffer, numSamples);
    m_effectsDelay.process(inOutBuffer.data(), numSamples);
    AssertIdenticalBufferEquals(inOutBuffer.span(), secondExpectedResult);

    SampleUtil::copy(inOutBuffer.data(), zeroBuffer, numSamples);
    m_effectsDelay.process(inOutBuffer.data(), numSamples);
    AssertIdenticalBufferEquals(inOutBuffer.span(), thirdExpectedResult);
}

TEST_F(EngineEffectsDelayTest, HalfBufferDelay) {
    const SINT numDelayFrames = 1;
    const SINT numSamples = 4;

    // Set delay size of half of the input buffer.
    m_effectsDelay.setDelayFrames(numDelayFrames);

    const CSAMPLE inputBuffer[] = {-100.0, 100.0, -99.0, 99.0};
    const CSAMPLE zeroBuffer[] = {0.0, 0.0, 0.0, 0.0};
    const CSAMPLE firstExpectedResult[] = {-50.0, 50.0, -74.5, 74.5};
    const CSAMPLE secondExpectedResult[] = {-99.0, 99.0, -100.0, 100.0};
    const CSAMPLE thirdExpectedResult[] = {-99.0, 99.0, 0.0, 0.0};

    mixxx::SampleBuffer inOutBuffer(numSamples);

    SampleUtil::copy(inOutBuffer.data(), inputBuffer, numSamples);
    m_effectsDelay.process(inOutBuffer.data(), numSamples);
    AssertIdenticalBufferEquals(inOutBuffer.span(), firstExpectedResult);

    SampleUtil::copy(inOutBuffer.data(), inputBuffer, numSamples);
    m_effectsDelay.process(inOutBuffer.data(), numSamples);
    AssertIdenticalBufferEquals(inOutBuffer.span(), secondExpectedResult);

    SampleUtil::copy(inOutBuffer.data(), zeroBuffer, numSamples);
    m_effectsDelay.process(inOutBuffer.data(), numSamples);
    AssertIdenticalBufferEquals(inOutBuffer.span(), thirdExpectedResult);
}

TEST_F(EngineEffectsDelayTest, MisalignedDelayAccordingToBuffer) {
    const SINT numDelayFrames = 3;
    const SINT numSamples = 8;

    // Set the number of delay frames different from the input buffer size
    // or half of the input buffer size.
    m_effectsDelay.setDelayFrames(numDelayFrames);

    const CSAMPLE inputBuffer[] = {
            -100.0, 100.0, -99.0, 99.0, -98.0, 98.0, -97.0, 97.0};
    const CSAMPLE zeroBuffer[] = {
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    const CSAMPLE firstExpectedResult[] = {
            -25.0, 25.0, -37.125, 37.125, -36.75, 36.75, -43.0, 43.0};
    const CSAMPLE secondExpectedResult[] = {
            -49.5, 49.5, -73.5, 73.5, -97.0, 97.0, -100.0, 100.0};
    const CSAMPLE thirdExpectedResult[] = {
            -99.0, 99.0, -98.0, 98.0, -97.0, 97.0, 0.0, 0.0};

    mixxx::SampleBuffer inOutBuffer(numSamples);

    SampleUtil::copy(inOutBuffer.data(), inputBuffer, numSamples);
    m_effectsDelay.process(inOutBuffer.data(), numSamples);
    AssertIdenticalBufferEquals(inOutBuffer.span(), firstExpectedResult);

    SampleUtil::copy(inOutBuffer.data(), inputBuffer, numSamples);
    m_effectsDelay.process(inOutBuffer.data(), numSamples);
    AssertIdenticalBufferEquals(inOutBuffer.span(), secondExpectedResult);

    SampleUtil::copy(inOutBuffer.data(), zeroBuffer, numSamples);
    m_effectsDelay.process(inOutBuffer.data(), numSamples);
    AssertIdenticalBufferEquals(inOutBuffer.span(), thirdExpectedResult);
}

TEST_F(EngineEffectsDelayTest, CrossfadeBetweenTwoNonZeroDelays) {
    SINT numDelayFrames = 2;
    const SINT numSamples = 8;

    // Set the number of delay frames as half of the input buffer size.
    m_effectsDelay.setDelayFrames(numDelayFrames);

    const CSAMPLE inputBuffer[] = {
            -100.0, 100.0, -99.0, 99.0, -98.0, 98.0, -97.0, 97.0};

    const CSAMPLE firstExpectedResult[] = {
            -25.0, 25.0, -37.125, 37.125, -49.25, 49.25, -61.375, 61.375};
    const CSAMPLE secondExpectedResult[] = {
            -73.5, 73.5, -97.0, 97.0, -100.0, 100.0, -99.0, 99.0};
    const CSAMPLE thirdExpectedResult[] = {
            -98.0, 98.0, -97.5, 97.5, -99.0, 99.0, -97.5, 97.5};
    const CSAMPLE fourthExpectedResult[] = {
            -100.0, 100.0, -99.0, 99.0, -98.0, 98.0, -97.0, 97.0};

    mixxx::SampleBuffer inOutBuffer(numSamples);

    SampleUtil::copy(inOutBuffer.data(), inputBuffer, numSamples);
    m_effectsDelay.process(inOutBuffer.data(), numSamples);
    AssertIdenticalBufferEquals(inOutBuffer.span(), firstExpectedResult);

    SampleUtil::copy(inOutBuffer.data(), inputBuffer, numSamples);
    m_effectsDelay.process(inOutBuffer.data(), numSamples);
    AssertIdenticalBufferEquals(inOutBuffer.span(), secondExpectedResult);

    // Set the number of delay frames as the size of the input buffer.
    numDelayFrames = 4;
    m_effectsDelay.setDelayFrames(numDelayFrames);

    SampleUtil::copy(inOutBuffer.data(), inputBuffer, numSamples);
    m_effectsDelay.process(inOutBuffer.data(), numSamples);
    AssertIdenticalBufferEquals(inOutBuffer.span(), thirdExpectedResult);

    SampleUtil::copy(inOutBuffer.data(), inputBuffer, numSamples);
    m_effectsDelay.process(inOutBuffer.data(), numSamples);
    AssertIdenticalBufferEquals(inOutBuffer.span(), fourthExpectedResult);
}

TEST_F(EngineEffectsDelayTest, CrossfadeSecondDelayGreaterThanInputBufferSize) {
    SINT numDelayFrames = 2;
    const SINT numSamples = 8;

    m_effectsDelay.setDelayFrames(numDelayFrames);

    const CSAMPLE inputBuffer[] = {
            -100.0, 100.0, -99.0, 99.0, -98.0, 98.0, -97.0, 97.0};

    const CSAMPLE firstExpectedResult[] = {
            -25.0, 25.0, -37.125, 37.125, -49.25, 49.25, -61.375, 61.375};
    const CSAMPLE secondExpectedResult[] = {
            -73.5, 73.5, -97.0, 97.0, -100.0, 100.0, -99.0, 99.0};
    const CSAMPLE thirdExpectedResult[] = {
            -98.0, 98.0, -91.125, 91.125, -98.5, 98.5, -99.75, 99.75};
    const CSAMPLE fourthExpectedResult[] = {
            -99.0, 99.0, -98.0, 98.0, -97.0, 97.0, -100.0, 100.0};

    mixxx::SampleBuffer inOutBuffer(numSamples);

    SampleUtil::copy(inOutBuffer.data(), inputBuffer, numSamples);
    m_effectsDelay.process(inOutBuffer.data(), numSamples);
    AssertIdenticalBufferEquals(inOutBuffer.span(), firstExpectedResult);

    SampleUtil::copy(inOutBuffer.data(), inputBuffer, numSamples);
    m_effectsDelay.process(inOutBuffer.data(), numSamples);
    AssertIdenticalBufferEquals(inOutBuffer.span(), secondExpectedResult);

    // Set the number of frames greater than the size of the input buffer.
    numDelayFrames = 7;
    m_effectsDelay.setDelayFrames(numDelayFrames);

    SampleUtil::copy(inOutBuffer.data(), inputBuffer, numSamples);
    m_effectsDelay.process(inOutBuffer.data(), numSamples);
    AssertIdenticalBufferEquals(inOutBuffer.span(), thirdExpectedResult);

    SampleUtil::copy(inOutBuffer.data(), inputBuffer, numSamples);
    m_effectsDelay.process(inOutBuffer.data(), numSamples);
    AssertIdenticalBufferEquals(inOutBuffer.span(), fourthExpectedResult);
}

TEST_F(EngineEffectsDelayTest, CrossfadeBetweenThreeNonZeroDelays) {
    SINT numDelayFrames = 3;
    const SINT numSamples = 8;

    m_effectsDelay.setDelayFrames(numDelayFrames);

    const CSAMPLE inputBuffer[] = {
            -100.0, 100.0, -99.0, 99.0, -98.0, 98.0, -97.0, 97.0};

    const CSAMPLE firstExpectedResult[] = {
            -25.0, 25.0, -37.125, 37.125, -36.75, 36.75, -43.0, 43.0};
    const CSAMPLE secondExpectedResult[] = {
            -49.5, 49.5, -73.5, 73.5, -97.0, 97.0, -100.0, 100.0};
    const CSAMPLE thirdExpectedResult[] = {
            -99.0, 99.0, -98.5, 98.5, -98.0, 98.0, -98.5, 98.5};
    const CSAMPLE fourthExpectedResult[] = {
            -97.0, 97.0, -100.0, 100.0, -99.0, 99.0, -98.0, 98.0};
    const CSAMPLE fifthExpectedResult[] = {
            -97.0, 97.0, -99.5, 99.5, -98.0, 98.0, -99.5, 99.5};
    const CSAMPLE sixthExpectedResult[] = {
            -99.0, 99.0, -98.0, 98.0, -97.0, 97.0, -100.0, 100.0};

    mixxx::SampleBuffer inOutBuffer(numSamples);

    SampleUtil::copy(inOutBuffer.data(), inputBuffer, numSamples);
    m_effectsDelay.process(inOutBuffer.data(), numSamples);
    AssertIdenticalBufferEquals(inOutBuffer.span(), firstExpectedResult);

    SampleUtil::copy(inOutBuffer.data(), inputBuffer, numSamples);
    m_effectsDelay.process(inOutBuffer.data(), numSamples);
    AssertIdenticalBufferEquals(inOutBuffer.span(), secondExpectedResult);

    numDelayFrames = 1;
    m_effectsDelay.setDelayFrames(numDelayFrames);

    SampleUtil::copy(inOutBuffer.data(), inputBuffer, numSamples);
    m_effectsDelay.process(inOutBuffer.data(), numSamples);
    AssertIdenticalBufferEquals(inOutBuffer.span(), thirdExpectedResult);

    SampleUtil::copy(inOutBuffer.data(), inputBuffer, numSamples);
    m_effectsDelay.process(inOutBuffer.data(), numSamples);
    AssertIdenticalBufferEquals(inOutBuffer.span(), fourthExpectedResult);

    // Set the number of frames greater than the size of the input buffer.
    numDelayFrames = 7;
    m_effectsDelay.setDelayFrames(numDelayFrames);

    SampleUtil::copy(inOutBuffer.data(), inputBuffer, numSamples);
    m_effectsDelay.process(inOutBuffer.data(), numSamples);
    AssertIdenticalBufferEquals(inOutBuffer.span(), fifthExpectedResult);

    SampleUtil::copy(inOutBuffer.data(), inputBuffer, numSamples);
    m_effectsDelay.process(inOutBuffer.data(), numSamples);
    AssertIdenticalBufferEquals(inOutBuffer.span(), sixthExpectedResult);
}

TEST_F(EngineEffectsDelayTest, CopyWholeBufferForZeroDelay) {
    const SINT numSamples = 4;

    const CSAMPLE inputBuffer[] = {-100.0, 100.0, -99.0, 99.0};
    const CSAMPLE zeroBuffer[] = {0.0, 0.0, 0.0, 0.0};
    const CSAMPLE firstExpectedResult[] = {-100.0, 100.0, -99.0, 99.0};
    const CSAMPLE secondExpectedResult[] = {0.0, 0.0, 0.0, 0.0};

    mixxx::SampleBuffer inOutBuffer(numSamples);

    SampleUtil::copy(inOutBuffer.data(), inputBuffer, numSamples);
    m_effectsDelay.process(inOutBuffer.data(), numSamples);
    AssertIdenticalBufferEquals(inOutBuffer.span(), firstExpectedResult);

    SampleUtil::copy(inOutBuffer.data(), zeroBuffer, numSamples);
    m_effectsDelay.process(inOutBuffer.data(), numSamples);
    AssertIdenticalBufferEquals(inOutBuffer.span(), secondExpectedResult);
}

static void BM_ZeroDelay(benchmark::State& state) {
    const SINT bufferSizeInSamples = static_cast<SINT>(state.range(0));

    EngineEffectsDelay effectsDelay;

    mixxx::SampleBuffer inOutBuffer(bufferSizeInSamples);
    SampleUtil::fill(inOutBuffer.data(), 0.0f, bufferSizeInSamples);

    for (auto _ : state) {
        effectsDelay.process(inOutBuffer.data(), bufferSizeInSamples);
    }
}
BENCHMARK(BM_ZeroDelay)->Range(64, 4 << 10);

static void BM_DelaySmallerThanBufferSize(benchmark::State& state) {
    const SINT bufferSizeInSamples = static_cast<SINT>(state.range(0));
    const SINT bufferSizeInFrames = bufferSizeInSamples / mixxx::kEngineChannelCount;

    // The delay is half of the buffer size.
    const SINT delayFrames = bufferSizeInFrames / 2;

    EngineEffectsDelay effectsDelay;

    mixxx::SampleBuffer inOutBuffer(bufferSizeInSamples);
    SampleUtil::fill(inOutBuffer.data(), 0.0f, bufferSizeInSamples);

    effectsDelay.setDelayFrames(delayFrames);

    for (auto _ : state) {
        effectsDelay.process(inOutBuffer.data(), bufferSizeInSamples);
    }
}
BENCHMARK(BM_DelaySmallerThanBufferSize)->Range(64, 4 << 10);

static void BM_DelayGreaterThanBufferSize(benchmark::State& state) {
    const SINT bufferSizeInSamples = static_cast<SINT>(state.range(0));
    const SINT bufferSizeInFrames = bufferSizeInSamples / mixxx::kEngineChannelCount;

    // The delay is the same as twice of buffer size.
    const SINT delayFrames = bufferSizeInFrames * 2;

    EngineEffectsDelay effectsDelay;

    mixxx::SampleBuffer inOutBuffer(bufferSizeInSamples);
    SampleUtil::fill(inOutBuffer.data(), 0.0f, bufferSizeInSamples);

    effectsDelay.setDelayFrames(delayFrames);

    for (auto _ : state) {
        effectsDelay.process(inOutBuffer.data(), bufferSizeInSamples);
    }
}
BENCHMARK(BM_DelayGreaterThanBufferSize)->Range(64, 4 << 10);

static void BM_DelayCrossfading(benchmark::State& state) {
    const SINT bufferSizeInSamples = static_cast<SINT>(state.range(0));
    const SINT bufferSizeInFrames = bufferSizeInSamples / mixxx::kEngineChannelCount;

    // The first delay is half of the buffer size.
    const SINT firstDelayFrames = bufferSizeInFrames / 2;

    // The second delay is the same as twice of buffer size.
    const SINT secondDelayFrames = bufferSizeInFrames * 2;

    EngineEffectsDelay effectsDelay;

    mixxx::SampleBuffer inOutBuffer(bufferSizeInSamples);
    SampleUtil::fill(inOutBuffer.data(), 0.0f, bufferSizeInSamples);

    for (auto _ : state) {
        effectsDelay.setDelayFrames(firstDelayFrames);
        effectsDelay.process(inOutBuffer.data(), bufferSizeInSamples);
        effectsDelay.setDelayFrames(secondDelayFrames);
        effectsDelay.process(inOutBuffer.data(), bufferSizeInSamples);
    }
}
BENCHMARK(BM_DelayCrossfading)->Range(64, 4 << 10);

static void BM_DelayNoCrossfading(benchmark::State& state) {
    const SINT bufferSizeInSamples = static_cast<SINT>(state.range(0));
    const SINT bufferSizeInFrames = bufferSizeInSamples / mixxx::kEngineChannelCount;

    // The delay is half of the buffer size.
    const SINT delayFrames = bufferSizeInFrames / 2;

    EngineEffectsDelay effectsDelay;

    mixxx::SampleBuffer inOutBuffer(bufferSizeInSamples);
    SampleUtil::fill(inOutBuffer.data(), 0.0f, bufferSizeInSamples);

    for (auto _ : state) {
        effectsDelay.setDelayFrames(delayFrames);
        effectsDelay.process(inOutBuffer.data(), bufferSizeInSamples);
        effectsDelay.setDelayFrames(delayFrames);
        effectsDelay.process(inOutBuffer.data(), bufferSizeInSamples);
    }
}
BENCHMARK(BM_DelayNoCrossfading)->Range(64, 4 << 10);

} // namespace
