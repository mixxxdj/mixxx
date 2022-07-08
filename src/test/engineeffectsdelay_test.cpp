// Tests for engineeffectsdelay.cpp

// Premise: internal Mixxx structure works with a stereo signal.
// If the mixxx::kEngineChannelCount wouldn't be a stereo in the future,
// tests have to be updated.

#include "engine/effects/engineeffectsdelay.h"

#include <benchmark/benchmark.h>
#include <gtest/gtest.h>

#include <QTest>
#include <QtDebug>

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
    void AssertIdenticalBufferEquals(const CSAMPLE* pBuffer,
            int iBufferLen,
            CSAMPLE* pReferenceBuffer,
            int iReferenceBufferLength) {
        EXPECT_EQ(iBufferLen, iReferenceBufferLength);

        for (int i = 0; i < iBufferLen; ++i) {
            EXPECT_FLOAT_EQ(pBuffer[i], pReferenceBuffer[i]);
        }
    }

    EngineEffectsDelay m_effectsDelay;
};

TEST_F(EngineEffectsDelayTest, NegativeDelayValue) {
    const SINT numSamples = 4;

    // Set negative delay value.
    m_effectsDelay.setDelayFrames(-1);

    CSAMPLE inputBuffer[] = {-100.0, 100.0, -99.0, 99.0};

    mixxx::SampleBuffer pInOut(numSamples);
    SampleUtil::copy(pInOut.data(), inputBuffer, numSamples);

#ifdef MIXXX_DEBUG_ASSERTIONS_ENABLED
    // Set thread safe for EXPECT_DEATH.
    GTEST_FLAG_SET(death_test_style, "threadsafe");

    EXPECT_DEATH({
        m_effectsDelay.process(pInOut.data(), numSamples);
    },
            "m_currentDelaySamples >= 0");
#else
    CSAMPLE* expectedResult = inputBuffer;

    m_effectsDelay.process(pInOut.data(), numSamples);
    AssertIdenticalBufferEquals(pInOut.data(), numSamples, expectedResult, numSamples);
#endif
}

TEST_F(EngineEffectsDelayTest, DelayGreaterThanDelayBufferSize) {
    const SINT numDelayFrames = mixxx::audio::SampleRate::kValueMax;
    const SINT numSamples = 4;

    // Set delay greater than the size of the delay buffer.
    m_effectsDelay.setDelayFrames(numDelayFrames);

    CSAMPLE inputBuffer[] = {-100.0, 100.0, -99.0, 99.0};

    mixxx::SampleBuffer pInOut(numSamples);
    SampleUtil::copy(pInOut.data(), inputBuffer, numSamples);

#ifdef MIXXX_DEBUG_ASSERTIONS_ENABLED
    // Set thread safe for EXPECT_DEATH.
    GTEST_FLAG_SET(death_test_style, "threadsafe");

    EXPECT_DEATH({
        m_effectsDelay.process(pInOut.data(), numSamples);
    },
            "delaySourcePos >= 0");
#else
    CSAMPLE* expectedResult = inputBuffer;

    m_effectsDelay.process(pInOut.data(), numSamples);
    AssertIdenticalBufferEquals(pInOut.data(), numSamples, expectedResult, numSamples);
#endif
}

TEST_F(EngineEffectsDelayTest, WholeBufferDelay) {
    const SINT numDelayFrames = 2;
    const SINT numSamples = 4;

    // Set delay same as the size of the input buffer.
    m_effectsDelay.setDelayFrames(numDelayFrames);

    CSAMPLE inputBuffer[] = {-100.0, 100.0, -99.0, 99.0};
    CSAMPLE zeroBuffer[] = {0.0, 0.0, 0.0, 0.0};
    CSAMPLE firstExpectedResult[] = {-100.0, 75.0, -49.5, 24.75};

    CSAMPLE* secondExpectedResult = inputBuffer;
    CSAMPLE* thirdExpectedResult = zeroBuffer;

    mixxx::SampleBuffer pInOut(numSamples);

    SampleUtil::copy(pInOut.data(), inputBuffer, numSamples);
    m_effectsDelay.process(pInOut.data(), numSamples);
    AssertIdenticalBufferEquals(pInOut.data(), numSamples, firstExpectedResult, numSamples);

    SampleUtil::copy(pInOut.data(), zeroBuffer, numSamples);
    m_effectsDelay.process(pInOut.data(), numSamples);
    AssertIdenticalBufferEquals(pInOut.data(), numSamples, secondExpectedResult, numSamples);

    SampleUtil::copy(pInOut.data(), zeroBuffer, numSamples);
    m_effectsDelay.process(pInOut.data(), numSamples);
    AssertIdenticalBufferEquals(pInOut.data(), numSamples, thirdExpectedResult, numSamples);
}

TEST_F(EngineEffectsDelayTest, HalfBufferDelay) {
    const SINT numDelayFrames = 1;
    const SINT numSamples = 4;

    // Set delay size of half of the input buffer.
    m_effectsDelay.setDelayFrames(numDelayFrames);

    CSAMPLE inputBuffer[] = {-100.0, 100.0, -99.0, 99.0};
    CSAMPLE zeroBuffer[] = {0.0, 0.0, 0.0, 0.0};
    CSAMPLE firstExpectedResult[] = {-100.0, 75.0, -99.5, 99.75};
    CSAMPLE secondExpectedResult[] = {-99.0, 99.0, -100.0, 100.0};
    CSAMPLE thirdExpectedResult[] = {-99.0, 99.0, 0.0, 0.0};

    mixxx::SampleBuffer pInOut(numSamples);

    SampleUtil::copy(pInOut.data(), inputBuffer, numSamples);
    m_effectsDelay.process(pInOut.data(), numSamples);
    AssertIdenticalBufferEquals(pInOut.data(), numSamples, firstExpectedResult, numSamples);

    SampleUtil::copy(pInOut.data(), inputBuffer, numSamples);
    m_effectsDelay.process(pInOut.data(), numSamples);
    AssertIdenticalBufferEquals(pInOut.data(), numSamples, secondExpectedResult, numSamples);

    SampleUtil::copy(pInOut.data(), zeroBuffer, numSamples);
    m_effectsDelay.process(pInOut.data(), numSamples);
    AssertIdenticalBufferEquals(pInOut.data(), numSamples, thirdExpectedResult, numSamples);
}

TEST_F(EngineEffectsDelayTest, MisalignedDelayAccordingToBuffer) {
    const SINT numDelayFrames = 3;
    const SINT numSamples = 8;

    // Set the number of delay frames different from the input buffer size
    // or half of the input buffer size.
    m_effectsDelay.setDelayFrames(numDelayFrames);

    CSAMPLE inputBuffer[] = {-100.0, 100.0, -99.0, 99.0, -98.0, 98.0, -97.0, 97.0};
    CSAMPLE zeroBuffer[] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    CSAMPLE firstExpectedResult[] = {-100.0, 87.5, -74.25, 61.875, -49.0, 36.75, -99.25, 99.625};
    CSAMPLE secondExpectedResult[] = {-99.0, 99.0, -98.0, 98.0, -97.0, 97.0, -100.0, 100.0};
    CSAMPLE thirdExpectedResult[] = {-99.0, 99.0, -98.0, 98.0, -97.0, 97.0, 0.0, 0.0};

    mixxx::SampleBuffer pInOut(numSamples);

    SampleUtil::copy(pInOut.data(), inputBuffer, numSamples);
    m_effectsDelay.process(pInOut.data(), numSamples);
    AssertIdenticalBufferEquals(pInOut.data(), numSamples, firstExpectedResult, numSamples);

    SampleUtil::copy(pInOut.data(), inputBuffer, numSamples);
    m_effectsDelay.process(pInOut.data(), numSamples);
    AssertIdenticalBufferEquals(pInOut.data(), numSamples, secondExpectedResult, numSamples);

    SampleUtil::copy(pInOut.data(), zeroBuffer, numSamples);
    m_effectsDelay.process(pInOut.data(), numSamples);
    AssertIdenticalBufferEquals(pInOut.data(), numSamples, thirdExpectedResult, numSamples);
}

TEST_F(EngineEffectsDelayTest, CrossfadeBetweenTwoNonZeroDelays) {
    SINT numDelayFrames = 2;
    const SINT numSamples = 8;

    // Set the number of delay frames as half of the input buffer size.
    m_effectsDelay.setDelayFrames(numDelayFrames);

    CSAMPLE inputBuffer[] = {-100.0, 100.0, -99.0, 99.0, -98.0, 98.0, -97.0, 97.0};

    CSAMPLE firstExpectedResult[] = {-100.0, 87.5, -74.25, 61.875, -99.0, 99.25, -98.5, 98.75};
    CSAMPLE secondExpectedResult[] = {-98.0, 98.0, -97.0, 97.0, -100.0, 100.0, -99.0, 99.0};
    CSAMPLE thirdExpectedResult[] = {-98.0, 98.25, -97.5, 97.75, -99.0, 98.75, -97.5, 97.25};
    CSAMPLE fourthExpectedResult[] = {-100.0, 100.0, -99.0, 99.0, -98.0, 98.0, -97.0, 97.0};

    mixxx::SampleBuffer pInOut(numSamples);

    SampleUtil::copy(pInOut.data(), inputBuffer, numSamples);
    m_effectsDelay.process(pInOut.data(), numSamples);
    AssertIdenticalBufferEquals(pInOut.data(), numSamples, firstExpectedResult, numSamples);

    SampleUtil::copy(pInOut.data(), inputBuffer, numSamples);
    m_effectsDelay.process(pInOut.data(), numSamples);
    AssertIdenticalBufferEquals(pInOut.data(), numSamples, secondExpectedResult, numSamples);

    // Set the number of delay frames as the size of the input buffer.
    numDelayFrames = 4;
    m_effectsDelay.setDelayFrames(numDelayFrames);

    SampleUtil::copy(pInOut.data(), inputBuffer, numSamples);
    m_effectsDelay.process(pInOut.data(), numSamples);
    AssertIdenticalBufferEquals(pInOut.data(), numSamples, thirdExpectedResult, numSamples);

    SampleUtil::copy(pInOut.data(), inputBuffer, numSamples);
    m_effectsDelay.process(pInOut.data(), numSamples);
    AssertIdenticalBufferEquals(pInOut.data(), numSamples, fourthExpectedResult, numSamples);
}

TEST_F(EngineEffectsDelayTest, CrossfadeSecondDelayGreaterThanInputBufferSize) {
    SINT numDelayFrames = 2;
    const SINT numSamples = 8;

    m_effectsDelay.setDelayFrames(numDelayFrames);

    CSAMPLE inputBuffer[] = {-100.0, 100.0, -99.0, 99.0, -98.0, 98.0, -97.0, 97.0};

    CSAMPLE firstExpectedResult[] = {-100.0, 87.5, -74.25, 61.875, -99.0, 99.25, -98.5, 98.75};
    CSAMPLE secondExpectedResult[] = {-98.0, 98.0, -97.0, 97.0, -100.0, 100.0, -99.0, 99.0};
    CSAMPLE thirdExpectedResult[] = {-98.0, 98.125, -97.25, 97.375, -98.5, 98.125, -99.75, 99.875};
    CSAMPLE fourthExpectedResult[] = {-99.0, 99.0, -98.0, 98.0, -97.0, 97.0, -100.0, 100.0};

    mixxx::SampleBuffer pInOut(numSamples);

    SampleUtil::copy(pInOut.data(), inputBuffer, numSamples);
    m_effectsDelay.process(pInOut.data(), numSamples);
    AssertIdenticalBufferEquals(pInOut.data(), numSamples, firstExpectedResult, numSamples);

    SampleUtil::copy(pInOut.data(), inputBuffer, numSamples);
    m_effectsDelay.process(pInOut.data(), numSamples);
    AssertIdenticalBufferEquals(pInOut.data(), numSamples, secondExpectedResult, numSamples);

    // Set the number of frames greater than the size of the input buffer.
    numDelayFrames = 7;
    m_effectsDelay.setDelayFrames(numDelayFrames);

    SampleUtil::copy(pInOut.data(), inputBuffer, numSamples);
    m_effectsDelay.process(pInOut.data(), numSamples);
    AssertIdenticalBufferEquals(pInOut.data(), numSamples, thirdExpectedResult, numSamples);

    SampleUtil::copy(pInOut.data(), inputBuffer, numSamples);
    m_effectsDelay.process(pInOut.data(), numSamples);
    AssertIdenticalBufferEquals(pInOut.data(), numSamples, fourthExpectedResult, numSamples);
}

TEST_F(EngineEffectsDelayTest, CrossfadeBetweenThreeNonZeroDelays) {
    SINT numDelayFrames = 3;
    const SINT numSamples = 8;

    m_effectsDelay.setDelayFrames(numDelayFrames);

    CSAMPLE inputBuffer[] = {-100.0, 100.0, -99.0, 99.0, -98.0, 98.0, -97.0, 97.0};

    CSAMPLE firstExpectedResult[] = {-100.0, 87.5, -74.25, 61.875, -49.0, 36.75, -99.25, 99.625};
    CSAMPLE secondExpectedResult[] = {-99.0, 99.0, -98.0, 98.0, -97.0, 97.0, -100.0, 100.0};
    CSAMPLE thirdExpectedResult[] = {-99.0, 98.75, -98.5, 98.75, -98.0, 98.25, -98.5, 98.25};
    CSAMPLE fourthExpectedResult[] = {-97.0, 97.0, -100.0, 100.0, -99.0, 99.0, -98.0, 98.0};
    CSAMPLE fifthExpectedResult[] = {-97.0, 97.25, -99.5, 99.25, -98.0, 97.75, -99.5, 99.75};
    CSAMPLE sixthExpectedResult[] = {-99.0, 99.0, -98.0, 98.0, -97.0, 97.0, -100.0, 100.0};

    mixxx::SampleBuffer pInOut(numSamples);

    SampleUtil::copy(pInOut.data(), inputBuffer, numSamples);
    m_effectsDelay.process(pInOut.data(), numSamples);
    AssertIdenticalBufferEquals(pInOut.data(), numSamples, firstExpectedResult, numSamples);

    SampleUtil::copy(pInOut.data(), inputBuffer, numSamples);
    m_effectsDelay.process(pInOut.data(), numSamples);
    AssertIdenticalBufferEquals(pInOut.data(), numSamples, secondExpectedResult, numSamples);

    numDelayFrames = 1;
    m_effectsDelay.setDelayFrames(numDelayFrames);

    SampleUtil::copy(pInOut.data(), inputBuffer, numSamples);
    m_effectsDelay.process(pInOut.data(), numSamples);
    AssertIdenticalBufferEquals(pInOut.data(), numSamples, thirdExpectedResult, numSamples);

    SampleUtil::copy(pInOut.data(), inputBuffer, numSamples);
    m_effectsDelay.process(pInOut.data(), numSamples);
    AssertIdenticalBufferEquals(pInOut.data(), numSamples, fourthExpectedResult, numSamples);

    // Set the number of frames greater than the size of the input buffer.
    numDelayFrames = 7;
    m_effectsDelay.setDelayFrames(numDelayFrames);

    SampleUtil::copy(pInOut.data(), inputBuffer, numSamples);
    m_effectsDelay.process(pInOut.data(), numSamples);
    AssertIdenticalBufferEquals(pInOut.data(), numSamples, fifthExpectedResult, numSamples);

    SampleUtil::copy(pInOut.data(), inputBuffer, numSamples);
    m_effectsDelay.process(pInOut.data(), numSamples);
    AssertIdenticalBufferEquals(pInOut.data(), numSamples, sixthExpectedResult, numSamples);
}

TEST_F(EngineEffectsDelayTest, CopyWholeBufferForZeroDelay) {
    const SINT numSamples = 4;

    CSAMPLE inputBuffer[] = {-100.0, 100.0, -99.0, 99.0};
    CSAMPLE zeroBuffer[] = {0.0, 0.0, 0.0, 0.0};
    CSAMPLE* firstExpectedResult = inputBuffer;
    CSAMPLE* secondExpectedResult = zeroBuffer;

    mixxx::SampleBuffer pInOut(numSamples);

    SampleUtil::copy(pInOut.data(), inputBuffer, numSamples);
    m_effectsDelay.process(pInOut.data(), numSamples);
    AssertIdenticalBufferEquals(pInOut.data(), numSamples, firstExpectedResult, numSamples);

    SampleUtil::copy(pInOut.data(), zeroBuffer, numSamples);
    m_effectsDelay.process(pInOut.data(), numSamples);
    AssertIdenticalBufferEquals(pInOut.data(), numSamples, secondExpectedResult, numSamples);
}

static void BM_ZeroDelay(benchmark::State& state) {
    SINT bufferSizeInSamples = static_cast<SINT>(state.range(0));

    EngineEffectsDelay effectsDelay;

    mixxx::SampleBuffer pInOut(bufferSizeInSamples);
    SampleUtil::fill(pInOut.data(), 0.0f, bufferSizeInSamples);

    for (auto _ : state) {
        effectsDelay.process(pInOut.data(), bufferSizeInSamples);
    }
}
BENCHMARK(BM_ZeroDelay)->Range(64, 4 << 10);

static void BM_DelaySmallerThanBufferSize(benchmark::State& state) {
    SINT bufferSizeInSamples = static_cast<SINT>(state.range(0));
    SINT bufferSizeInFrames = bufferSizeInSamples / mixxx::kEngineChannelCount;

    // The delay is half of the buffer size.
    SINT delayFrames = bufferSizeInFrames / 2;

    EngineEffectsDelay effectsDelay;

    mixxx::SampleBuffer pInOut(bufferSizeInSamples);
    SampleUtil::fill(pInOut.data(), 0.0f, bufferSizeInSamples);

    effectsDelay.setDelayFrames(delayFrames);

    for (auto _ : state) {
        effectsDelay.process(pInOut.data(), bufferSizeInSamples);
    }
}
BENCHMARK(BM_DelaySmallerThanBufferSize)->Range(64, 4 << 10);

static void BM_DelayGreaterThanBufferSize(benchmark::State& state) {
    SINT bufferSizeInSamples = static_cast<SINT>(state.range(0));
    SINT bufferSizeInFrames = bufferSizeInSamples / mixxx::kEngineChannelCount;

    // The delay is the same as twice of buffer size.
    SINT delayFrames = bufferSizeInFrames * 2;

    EngineEffectsDelay effectsDelay;

    mixxx::SampleBuffer pInOut(bufferSizeInSamples);
    SampleUtil::fill(pInOut.data(), 0.0f, bufferSizeInSamples);

    effectsDelay.setDelayFrames(delayFrames);

    for (auto _ : state) {
        effectsDelay.process(pInOut.data(), bufferSizeInSamples);
    }
}
BENCHMARK(BM_DelayGreaterThanBufferSize)->Range(64, 4 << 10);

static void BM_DelayCrossfading(benchmark::State& state) {
    SINT bufferSizeInSamples = static_cast<SINT>(state.range(0));
    SINT bufferSizeInFrames = bufferSizeInSamples / mixxx::kEngineChannelCount;

    // The first delay is half of the buffer size.
    SINT firstDelayFrames = bufferSizeInFrames / 2;

    // The second delay is the same as twice of buffer size.
    SINT secondDelayFrames = bufferSizeInFrames * 2;

    EngineEffectsDelay effectsDelay;

    mixxx::SampleBuffer pInOut(bufferSizeInSamples);
    SampleUtil::fill(pInOut.data(), 0.0f, bufferSizeInSamples);

    for (auto _ : state) {
        effectsDelay.setDelayFrames(firstDelayFrames);
        effectsDelay.process(pInOut.data(), bufferSizeInSamples);
        effectsDelay.setDelayFrames(secondDelayFrames);
        effectsDelay.process(pInOut.data(), bufferSizeInSamples);
    }
}
BENCHMARK(BM_DelayCrossfading)->Range(64, 4 << 10);

static void BM_DelayNoCrossfading(benchmark::State& state) {
    SINT bufferSizeInSamples = static_cast<SINT>(state.range(0));
    SINT bufferSizeInFrames = bufferSizeInSamples / mixxx::kEngineChannelCount;

    // The delay is half of the buffer size.
    SINT delayFrames = bufferSizeInFrames / 2;

    EngineEffectsDelay effectsDelay;

    mixxx::SampleBuffer pInOut(bufferSizeInSamples);
    SampleUtil::fill(pInOut.data(), 0.0f, bufferSizeInSamples);

    for (auto _ : state) {
        effectsDelay.setDelayFrames(delayFrames);
        effectsDelay.process(pInOut.data(), bufferSizeInSamples);
        effectsDelay.setDelayFrames(delayFrames);
        effectsDelay.process(pInOut.data(), bufferSizeInSamples);
    }
}
BENCHMARK(BM_DelayNoCrossfading)->Range(64, 4 << 10);

} // namespace
