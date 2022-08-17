// Tests for ringdelaybuffer.h

#include "util/ringdelaybuffer.h"

#include <benchmark/benchmark.h>
#include <gtest/gtest.h>

#include <QTest>

#include "test/mixxxtest.h"
#include "util/sample.h"
#include "util/samplebuffer.h"
#include "util/types.h"

namespace {

class RingDelayBufferTest : public MixxxTest {
  protected:
    void SetUp() override {
        m_pRingDelayBuffer = std::make_unique<RingDelayBuffer>(m_ringDelayBufferSize);
    }

    void TearDown() override {
    }

    void AssertIdenticalBufferEquals(const CSAMPLE* pBuffer,
            int iBufferLen,
            const CSAMPLE* pReferenceBuffer,
            int iReferenceBufferLength) {
        EXPECT_EQ(iBufferLen, iReferenceBufferLength);

        for (int i = 0; i < iBufferLen; ++i) {
            EXPECT_FLOAT_EQ(pBuffer[i], pReferenceBuffer[i]);
        }
    }

    std::unique_ptr<RingDelayBuffer> m_pRingDelayBuffer;
    const SINT m_ringDelayBufferSize = 8;
};

TEST_F(RingDelayBufferTest, ReadWriteNoDelayTest) {
    const SINT numSamplesHalf = 2;
    const SINT numSamples = 4;
    const CSAMPLE inputBuffer[] = {-100.0, 100.0, -99.0, 99.0};
    const CSAMPLE inputBufferHalf[] = {-98.0, 98.0};
    const CSAMPLE firstExpectedResult[] = {-50.0, 50.0, -99.0, 99.0};
    const CSAMPLE secondExpectedResult[] = {-99.0, 99.0, -98.0, 98.0};
    const CSAMPLE thirdExpectedResult[] = {-100.0, 100.0, -99.0, 99.0};

    mixxx::SampleBuffer output(numSamples);

    EXPECT_EQ(m_pRingDelayBuffer->write(inputBuffer, numSamples), numSamples);
    EXPECT_EQ(m_pRingDelayBuffer->read(output.data(), numSamples, 0), numSamples);

    AssertIdenticalBufferEquals(output.data(),
            numSamples,
            firstExpectedResult,
            numSamples);

    EXPECT_EQ(m_pRingDelayBuffer->write(inputBufferHalf, numSamplesHalf), numSamplesHalf);
    EXPECT_EQ(m_pRingDelayBuffer->read(output.data(), numSamples, 0), numSamples);

    AssertIdenticalBufferEquals(output.data(), numSamples, secondExpectedResult, numSamples);

    // Write and read over one ring.
    EXPECT_EQ(m_pRingDelayBuffer->write(inputBuffer, numSamples), numSamples);
    EXPECT_EQ(m_pRingDelayBuffer->read(output.data(), numSamples, 0), numSamples);

    AssertIdenticalBufferEquals(output.data(), numSamples, thirdExpectedResult, numSamples);
}

TEST_F(RingDelayBufferTest, ReadWriteDelayTest) {
    const SINT numSamples = 4;
    const SINT firstDelaySize = 2;
    const SINT secondDelaySize = 4;
    const CSAMPLE inputBuffer[] = {-100.0, 100.0, -99.0, 99.0};
    const CSAMPLE firstExpectedResult[] = {-50.0, 50.0, -99.0, 99.0};
    const CSAMPLE secondExpectedResult[] = {0.0, 0.0, -50.0, 50.0};
    const CSAMPLE thirdExpectedResult[] = {-50.0, 50.0, -99.0, 99.0};

    mixxx::SampleBuffer output(numSamples);

    // Read without delay.
    EXPECT_EQ(m_pRingDelayBuffer->write(inputBuffer, numSamples), numSamples);
    EXPECT_EQ(m_pRingDelayBuffer->read(output.data(), numSamples, 0), numSamples);

    AssertIdenticalBufferEquals(output.data(), numSamples, firstExpectedResult, numSamples);

    // Read with delay.
    EXPECT_EQ(m_pRingDelayBuffer->read(output.data(), numSamples, firstDelaySize), numSamples);

    AssertIdenticalBufferEquals(output.data(), numSamples, secondExpectedResult, numSamples);

    // Fill the second half of the delay buffer with the first input data.
    EXPECT_EQ(m_pRingDelayBuffer->write(inputBuffer, numSamples), numSamples);

    // Read with delay (not circle around).
    EXPECT_EQ(m_pRingDelayBuffer->read(output.data(), numSamples, secondDelaySize), numSamples);

    AssertIdenticalBufferEquals(output.data(), numSamples, thirdExpectedResult, numSamples);
}

static void BM_WriteReadWholeBufferNoDelay(benchmark::State& state) {
    const SINT ringDelayBufferSize = static_cast<SINT>(state.range(0));
    const SINT numSamples = ringDelayBufferSize / 2;

    RingDelayBuffer m_ringDelayBuffer(ringDelayBufferSize);

    mixxx::SampleBuffer input(numSamples);
    mixxx::SampleBuffer output(numSamples);

    SampleUtil::fill(input.data(), 0.0f, numSamples);

    for (auto _ : state) {
        state.PauseTiming();
        m_ringDelayBuffer.clear();
        state.ResumeTiming();

        m_ringDelayBuffer.write(input.data(), numSamples);
        m_ringDelayBuffer.read(output.data(), numSamples, 0);
        m_ringDelayBuffer.write(input.data(), numSamples);
        m_ringDelayBuffer.read(output.data(), numSamples, 0);
    }
}
BENCHMARK(BM_WriteReadWholeBufferNoDelay)->Range(64, 4 << 10);

static void BM_WriteReadWholeBufferDelay(benchmark::State& state) {
    const SINT ringDelayBufferSize = static_cast<SINT>(state.range(0));
    const SINT numSamples = ringDelayBufferSize / 2;
    const SINT delaySize = numSamples;

    RingDelayBuffer m_ringDelayBuffer(ringDelayBufferSize);

    mixxx::SampleBuffer input(numSamples);
    mixxx::SampleBuffer output(numSamples);

    SampleUtil::fill(input.data(), 0.0f, numSamples);

    for (auto _ : state) {
        state.PauseTiming();
        m_ringDelayBuffer.clear();
        state.ResumeTiming();

        m_ringDelayBuffer.write(input.data(), numSamples);
        m_ringDelayBuffer.read(output.data(), numSamples, delaySize);
        m_ringDelayBuffer.write(input.data(), numSamples);
        m_ringDelayBuffer.read(output.data(), numSamples, delaySize);
    }
}
BENCHMARK(BM_WriteReadWholeBufferDelay)->Range(64, 4 << 10);
} // namespace
