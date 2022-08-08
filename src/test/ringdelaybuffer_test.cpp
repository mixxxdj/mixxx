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

TEST_F(RingDelayBufferTest, ReadWriteNoSkipTest) {
    const SINT numSamplesHalf = 2;
    const SINT numSamples = 4;
    const CSAMPLE inputBuffer[] = {-100.0, 100.0, -99.0, 99.0};
    const CSAMPLE inputBufferHalf[] = {-98.0, 98.0};
    const CSAMPLE firstExpectedResult[] = {-50.0, 50.0};
    const CSAMPLE secondExpectedResult[] = {-99.0, 99.0, -98.0, 98.0};
    const CSAMPLE thirdExpectedResult[] = {-100.0, 100.0, -99.0, 99.0};

    mixxx::SampleBuffer halfSizeOutput(numSamplesHalf);
    mixxx::SampleBuffer output(numSamples);

    EXPECT_EQ(m_pRingDelayBuffer->write(inputBuffer, numSamples), numSamples);
    EXPECT_EQ(m_pRingDelayBuffer->read(halfSizeOutput.data(), numSamplesHalf), numSamplesHalf);

    AssertIdenticalBufferEquals(halfSizeOutput.data(),
            numSamplesHalf,
            firstExpectedResult,
            numSamplesHalf);

    EXPECT_EQ(m_pRingDelayBuffer->write(inputBufferHalf, numSamplesHalf), numSamplesHalf);
    EXPECT_EQ(m_pRingDelayBuffer->read(output.data(), numSamples), numSamples);

    AssertIdenticalBufferEquals(output.data(), numSamples, secondExpectedResult, numSamples);

    // Write and read over one ring.
    EXPECT_EQ(m_pRingDelayBuffer->write(inputBuffer, numSamples), numSamples);
    EXPECT_EQ(m_pRingDelayBuffer->read(output.data(), numSamples), numSamples);

    AssertIdenticalBufferEquals(output.data(), numSamples, thirdExpectedResult, numSamples);
}

TEST_F(RingDelayBufferTest, ReadWriteSkipTest) {
    const SINT numSamplesHalf = 2;
    const SINT numSamples = 4;
    const SINT firstJumpSizeLeft = -6;
    const SINT secondJumpSizeLeft = -1;
    const SINT firstJumpSizeRight = 5;
    const SINT secondJumpSizeRight = 1;
    const CSAMPLE firstInputBuffer[] = {-100.0, 100.0, -99.0, 99.0};
    const CSAMPLE secondInputBuffer[] = {-98.0, 98.0, -97.0, 97.0};
    const CSAMPLE firstExpectedResult[] = {-50.0, 50.0, -99.0, 99.0};
    const CSAMPLE secondExpectedResult[] = {0.0, 0.0, -50.0, 50.0};
    const CSAMPLE thirdExpectedResult[] = {50.0, -99.0, 99.0, -100.0};
    const CSAMPLE fourthExpectedResult[] = {-97.0, 97.0, -98.0, 98.0};
    const CSAMPLE fifthExpectedResult[] = {97.0, -98.0, 98.0, -97.0};

    mixxx::SampleBuffer halfSizeOutput(numSamplesHalf);
    mixxx::SampleBuffer output(numSamples);

    EXPECT_EQ(m_pRingDelayBuffer->write(firstInputBuffer, numSamples), numSamples);
    EXPECT_EQ(m_pRingDelayBuffer->read(output.data(), numSamples), numSamples);

    AssertIdenticalBufferEquals(output.data(), numSamples, firstExpectedResult, numSamples);

    // Move read position to the left (circle around).
    EXPECT_EQ(m_pRingDelayBuffer->moveReadPositionBy(firstJumpSizeLeft), firstJumpSizeLeft);
    EXPECT_EQ(m_pRingDelayBuffer->read(output.data(), numSamples), numSamples);

    AssertIdenticalBufferEquals(output.data(), numSamples, secondExpectedResult, numSamples);

    // Fill the second half of the delay buffer with the first input data.
    EXPECT_EQ(m_pRingDelayBuffer->write(firstInputBuffer, numSamples), numSamples);

    // Move read position to the left (not circle around).
    EXPECT_EQ(m_pRingDelayBuffer->moveReadPositionBy(secondJumpSizeLeft), secondJumpSizeLeft);

    EXPECT_EQ(m_pRingDelayBuffer->read(output.data(), numSamples), numSamples);

    AssertIdenticalBufferEquals(output.data(), numSamples, thirdExpectedResult, numSamples);

    // For the second cycle fill the first half of the delay buffer
    // with second input data. Data are changed based on the testing
    // use case.
    EXPECT_EQ(m_pRingDelayBuffer->write(secondInputBuffer, numSamples), numSamples);

    // Move read position to the right (circle around).
    EXPECT_EQ(m_pRingDelayBuffer->moveReadPositionBy(firstJumpSizeRight), firstJumpSizeRight);

    // Fill the second half of the delay buffer with the second input data.
    EXPECT_EQ(m_pRingDelayBuffer->write(secondInputBuffer, numSamples), numSamples);

    EXPECT_EQ(m_pRingDelayBuffer->read(output.data(), numSamples), numSamples);

    AssertIdenticalBufferEquals(output.data(), numSamples, fourthExpectedResult, numSamples);

    // Move read position to the right (not circle around).
    EXPECT_EQ(m_pRingDelayBuffer->moveReadPositionBy(secondJumpSizeRight), secondJumpSizeRight);

    // Fill the first half of the delay buffer with the second input data.
    EXPECT_EQ(m_pRingDelayBuffer->write(secondInputBuffer, numSamples), numSamples);

    EXPECT_EQ(m_pRingDelayBuffer->read(output.data(), numSamples), numSamples);

    AssertIdenticalBufferEquals(output.data(), numSamples, fifthExpectedResult, numSamples);
}

TEST_F(RingDelayBufferTest, IsEmptyTest) {
    const SINT numSamples = 4;
    const CSAMPLE inputBuffer[] = {-100.0, 100.0, -99.0, 99.0};

    EXPECT_TRUE(m_pRingDelayBuffer->isEmpty());
    m_pRingDelayBuffer->write(inputBuffer, numSamples);

    EXPECT_FALSE(m_pRingDelayBuffer->isEmpty());
}

TEST_F(RingDelayBufferTest, ClearTest) {
    const SINT numSamples = 4;
    const CSAMPLE inputBuffer[] = {-100.0, 100.0, -99.0, 99.0};

    EXPECT_TRUE(m_pRingDelayBuffer->isEmpty());
    m_pRingDelayBuffer->write(inputBuffer, numSamples);

    EXPECT_FALSE(m_pRingDelayBuffer->isEmpty());
    m_pRingDelayBuffer->clear();
    EXPECT_TRUE(m_pRingDelayBuffer->isEmpty());
}

TEST_F(RingDelayBufferTest, IsFullTest) {
    const SINT numSamples = 4;
    const CSAMPLE inputBuffer[] = {-100.0, 100.0, -99.0, 99.0};

    EXPECT_FALSE(m_pRingDelayBuffer->isFull());
    m_pRingDelayBuffer->write(inputBuffer, numSamples);

    EXPECT_FALSE(m_pRingDelayBuffer->isFull());
    m_pRingDelayBuffer->write(inputBuffer, numSamples);

    EXPECT_TRUE(m_pRingDelayBuffer->isFull());
    m_pRingDelayBuffer->clear();

    EXPECT_FALSE(m_pRingDelayBuffer->isFull());
}

TEST_F(RingDelayBufferTest, ReadAvailableItems) {
    const SINT numSamples = 4;
    const CSAMPLE inputBuffer[] = {-100.0, 100.0, -99.0, 99.0};

    EXPECT_EQ(m_pRingDelayBuffer->getReadAvailable(), 0);
    m_pRingDelayBuffer->write(inputBuffer, numSamples);

    EXPECT_EQ(m_pRingDelayBuffer->getReadAvailable(), numSamples);
    m_pRingDelayBuffer->write(inputBuffer, numSamples);

    EXPECT_EQ(m_pRingDelayBuffer->getReadAvailable(), 2 * numSamples);
}

TEST_F(RingDelayBufferTest, WriteAvailableItems) {
    const SINT numSamples = 4;
    const CSAMPLE inputBuffer[] = {-100.0, 100.0, -99.0, 99.0};

    EXPECT_EQ(m_pRingDelayBuffer->getWriteAvailable(), m_ringDelayBufferSize);
    m_pRingDelayBuffer->write(inputBuffer, numSamples);

    EXPECT_EQ(m_pRingDelayBuffer->getWriteAvailable(), m_ringDelayBufferSize - numSamples);
    m_pRingDelayBuffer->write(inputBuffer, numSamples);

    EXPECT_EQ(m_pRingDelayBuffer->getWriteAvailable(), 0);
}

static void BM_WriteReadWholeBufferNoSkip(benchmark::State& state) {
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
        m_ringDelayBuffer.read(output.data(), numSamples);
        m_ringDelayBuffer.write(input.data(), numSamples);
        m_ringDelayBuffer.read(output.data(), numSamples);
    }
}
BENCHMARK(BM_WriteReadWholeBufferNoSkip)->Range(64, 4 << 10);

static void BM_WriteReadWholeBufferSkipLeftNoCircle(benchmark::State& state) {
    const SINT ringDelayBufferSize = static_cast<SINT>(state.range(0));
    const SINT numSamples = ringDelayBufferSize / 2;
    const SINT jumpSizeLeft = -numSamples;

    RingDelayBuffer m_ringDelayBuffer(ringDelayBufferSize);

    mixxx::SampleBuffer input(numSamples);
    mixxx::SampleBuffer output(numSamples);

    SampleUtil::fill(input.data(), 0.0f, numSamples);

    for (auto _ : state) {
        state.PauseTiming();
        m_ringDelayBuffer.clear();
        state.ResumeTiming();

        m_ringDelayBuffer.write(input.data(), numSamples);
        m_ringDelayBuffer.read(output.data(), numSamples);
        m_ringDelayBuffer.moveReadPositionBy(jumpSizeLeft);
        m_ringDelayBuffer.write(input.data(), numSamples);
        m_ringDelayBuffer.read(output.data(), numSamples);
    }
}
BENCHMARK(BM_WriteReadWholeBufferSkipLeftNoCircle)->Range(64, 4 << 10);

static void BM_WriteReadWholeBufferSkipLeftCircle(benchmark::State& state) {
    const SINT ringDelayBufferSize = static_cast<SINT>(state.range(0));
    const SINT numSamples = ringDelayBufferSize / 2;
    const SINT jumpSizeLeft = -ringDelayBufferSize + 1;

    RingDelayBuffer m_ringDelayBuffer(ringDelayBufferSize);

    mixxx::SampleBuffer input(numSamples);
    mixxx::SampleBuffer output(numSamples);

    SampleUtil::fill(input.data(), 0.0f, numSamples);

    for (auto _ : state) {
        state.PauseTiming();
        m_ringDelayBuffer.clear();
        state.ResumeTiming();

        m_ringDelayBuffer.write(input.data(), numSamples);
        m_ringDelayBuffer.read(output.data(), numSamples);
        m_ringDelayBuffer.moveReadPositionBy(jumpSizeLeft);
        m_ringDelayBuffer.read(output.data(), numSamples);
        m_ringDelayBuffer.write(input.data(), numSamples);
    }
}
BENCHMARK(BM_WriteReadWholeBufferSkipLeftCircle)->Range(64, 4 << 10);

} // namespace
