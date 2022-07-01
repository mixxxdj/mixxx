// Tests for engineeffectsdelay.cpp

// Premise: internal Mixxx structure works with a stereo signal.
// If the mixxx::kEngineChannelCount wouldn't be a stereo in the future,
// tests have to be updated.

#include "engine/effects/engineeffectsdelay.h"

#include <gtest/gtest.h>

#include <QTest>
#include <QtDebug>

#include "engine/engine.h"
#include "test/mixxxtest.h"
#include "util/sample.h"
#include "util/types.h"

namespace {

static_assert(mixxx::kEngineChannelCount == mixxx::audio::ChannelCount::stereo(),
        "EngineEffectsDelayTest requires stereo input signal.");

class EngineEffectsDelayTest : public MixxxTest {
  protected:
    void SetUp() override {
        m_pEffectsDelay = std::make_unique<EngineEffectsDelay>();
    }

    void TearDown() override {
    }

    void AssertIdenticalBufferEquals(const CSAMPLE* pBuffer,
            int iBufferLen,
            CSAMPLE* pReferenceBuffer,
            int iReferenceBufferLength) {
        EXPECT_EQ(iBufferLen, iReferenceBufferLength);

        for (int i = 0; i < iBufferLen; ++i) {
            EXPECT_FLOAT_EQ(pBuffer[i], pReferenceBuffer[i]);
        }
    }

    std::unique_ptr<EngineEffectsDelay> m_pEffectsDelay;
};

TEST_F(EngineEffectsDelayTest, NegativeDelayValue) {
    const SINT numSamples = 4;

    // Set negative delay value.
    m_pEffectsDelay->setDelayFrames(-1);

    CSAMPLE inputBuffer[] = {-100.0, 100.0, -99.0, 99.0};

    CSAMPLE* pOutput = SampleUtil::alloc(numSamples);

#ifdef MIXXX_DEBUG_ASSERTIONS_ENABLED
    // Set thread safe for EXPECT_DEATH.
    GTEST_FLAG_SET(death_test_style, "threadsafe");

    EXPECT_DEATH({
        m_pEffectsDelay->process(inputBuffer, pOutput, numSamples);
    },
            "m_delaySamples >= 0");
#else
    CSAMPLE* expectedResult = inputBuffer;

    m_pEffectsDelay->process(inputBuffer, pOutput, numSamples);
    AssertIdenticalBufferEquals(pOutput, numSamples, expectedResult, numSamples);
#endif

    SampleUtil::free(pOutput);
}

TEST_F(EngineEffectsDelayTest, DelayGreaterThanDelayBufferSize) {
    const SINT numDelayFrames = mixxx::audio::SampleRate::kValueMax;
    const SINT numSamples = 4;

    // Set delay greater than the size of the delay buffer.
    m_pEffectsDelay->setDelayFrames(numDelayFrames);

    CSAMPLE inputBuffer[] = {-100.0, 100.0, -99.0, 99.0};

    CSAMPLE* pOutput = SampleUtil::alloc(numSamples);

#ifdef MIXXX_DEBUG_ASSERTIONS_ENABLED
    // Set thread safe for EXPECT_DEATH.
    GTEST_FLAG_SET(death_test_style, "threadsafe");

    EXPECT_DEATH({
        m_pEffectsDelay->process(inputBuffer, pOutput, numSamples);
    },
            "delaySourcePos >= 0");
#else
    CSAMPLE* expectedResult = inputBuffer;

    m_pEffectsDelay->process(inputBuffer, pOutput, numSamples);
    AssertIdenticalBufferEquals(pOutput, numSamples, expectedResult, numSamples);
#endif

    SampleUtil::free(pOutput);
}

TEST_F(EngineEffectsDelayTest, WholeBufferDelay) {
    const SINT numDelayFrames = 2;
    const SINT numSamples = 4;

    // Set delay same as the size of the input buffer.
    m_pEffectsDelay->setDelayFrames(numDelayFrames);

    CSAMPLE inputBuffer[] = {-100.0, 100.0, -99.0, 99.0};
    CSAMPLE zeroBuffer[] = {0.0, 0.0, 0.0, 0.0};
    CSAMPLE firstExpectedResult[] = {-100.0, 100.0, -99.0, 49.5};

    CSAMPLE* secondExpectedResult = inputBuffer;
    CSAMPLE* thirdExpectedResult = zeroBuffer;

    CSAMPLE* pOutput = SampleUtil::alloc(numSamples);

    m_pEffectsDelay->process(inputBuffer, pOutput, numSamples);
    AssertIdenticalBufferEquals(pOutput, numSamples, firstExpectedResult, numSamples);

    m_pEffectsDelay->process(zeroBuffer, pOutput, numSamples);
    AssertIdenticalBufferEquals(pOutput, numSamples, secondExpectedResult, numSamples);

    m_pEffectsDelay->process(zeroBuffer, pOutput, numSamples);
    AssertIdenticalBufferEquals(pOutput, numSamples, thirdExpectedResult, numSamples);

    SampleUtil::free(pOutput);
}

TEST_F(EngineEffectsDelayTest, HalfBufferDelay) {
    const SINT numDelayFrames = 1;
    const SINT numSamples = 4;

    // Set delay size of half of the input buffer.
    m_pEffectsDelay->setDelayFrames(numDelayFrames);

    CSAMPLE inputBuffer[] = {-100.0, 100.0, -99.0, 99.0};
    CSAMPLE zeroBuffer[] = {0.0, 0.0, 0.0, 0.0};
    CSAMPLE firstExpectedResult[] = {-100.0, 100.0, -99.0, 99.5};
    CSAMPLE secondExpectedResult[] = {-99.0, 99.0, -100.0, 100.0};
    CSAMPLE thirdExpectedResult[] = {-99.0, 99.0, 0.0, 0.0};

    CSAMPLE* pOutput = SampleUtil::alloc(numSamples);

    m_pEffectsDelay->process(inputBuffer, pOutput, numSamples);
    AssertIdenticalBufferEquals(pOutput, numSamples, firstExpectedResult, numSamples);

    m_pEffectsDelay->process(inputBuffer, pOutput, numSamples);
    AssertIdenticalBufferEquals(pOutput, numSamples, secondExpectedResult, numSamples);

    m_pEffectsDelay->process(zeroBuffer, pOutput, numSamples);
    AssertIdenticalBufferEquals(pOutput, numSamples, thirdExpectedResult, numSamples);

    SampleUtil::free(pOutput);
}

TEST_F(EngineEffectsDelayTest, MisalignedDelayAccordingToBuffer) {
    const SINT numDelayFrames = 3;
    const SINT numSamples = 8;

    // Set the number of delay frames different from the input buffer size
    // or half of the input buffer size.
    m_pEffectsDelay->setDelayFrames(numDelayFrames);

    CSAMPLE inputBuffer[] = {-100.0, 100.0, -99.0, 99.0, -98.0, 98.0, -97.0, 97.0};
    CSAMPLE zeroBuffer[] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    CSAMPLE firstExpectedResult[] = {-100.0, 100.0, -99.0, 99.0, -98.0, 73.5, -98.5, 99.25};
    CSAMPLE secondExpectedResult[] = {-99.0, 99.0, -98.0, 98.0, -97.0, 97.0, -100.0, 100.0};
    CSAMPLE thirdExpectedResult[] = {-99.0, 99.0, -98.0, 98.0, -97.0, 97.0, 0.0, 0.0};

    CSAMPLE* pOutput = SampleUtil::alloc(numSamples);

    m_pEffectsDelay->process(inputBuffer, pOutput, numSamples);
    AssertIdenticalBufferEquals(pOutput, numSamples, firstExpectedResult, numSamples);

    m_pEffectsDelay->process(inputBuffer, pOutput, numSamples);
    AssertIdenticalBufferEquals(pOutput, numSamples, secondExpectedResult, numSamples);

    m_pEffectsDelay->process(zeroBuffer, pOutput, numSamples);
    AssertIdenticalBufferEquals(pOutput, numSamples, thirdExpectedResult, numSamples);

    SampleUtil::free(pOutput);
}

TEST_F(EngineEffectsDelayTest, CrossfadeBetweenTwoNonZeroDelays) {
    SINT numDelayFrames = 2;
    const SINT numSamples = 8;

    // Set the number of delay frames as half of the input buffer size.
    m_pEffectsDelay->setDelayFrames(numDelayFrames);

    CSAMPLE inputBuffer[] = {-100.0, 100.0, -99.0, 99.0, -98.0, 98.0, -97.0, 97.0};

    CSAMPLE firstExpectedResult[] = {-100.0, 100.0, -99.0, 99.0, -98.0, 98.5, -98, 98.5};
    CSAMPLE secondExpectedResult[] = {-98.0, 98.0, -97.0, 97.0, -100.0, 100.0, -99.0, 99.0};
    CSAMPLE thirdExpectedResult[] = {-98.0, 98.0, -97.0, 97.0, -100.0, 99.5, -98.0, 97.5};
    CSAMPLE fourthExpectedResult[] = {-100.0, 100.0, -99.0, 99.0, -98.0, 98.0, -97.0, 97.0};

    CSAMPLE* pOutput = SampleUtil::alloc(numSamples);

    m_pEffectsDelay->process(inputBuffer, pOutput, numSamples);
    AssertIdenticalBufferEquals(pOutput, numSamples, firstExpectedResult, numSamples);

    m_pEffectsDelay->process(inputBuffer, pOutput, numSamples);
    AssertIdenticalBufferEquals(pOutput, numSamples, secondExpectedResult, numSamples);

    // Set the number of delay frames as the size of the input buffer.
    numDelayFrames = 4;
    m_pEffectsDelay->setDelayFrames(numDelayFrames);

    m_pEffectsDelay->process(inputBuffer, pOutput, numSamples);
    AssertIdenticalBufferEquals(pOutput, numSamples, thirdExpectedResult, numSamples);

    m_pEffectsDelay->process(inputBuffer, pOutput, numSamples);
    AssertIdenticalBufferEquals(pOutput, numSamples, fourthExpectedResult, numSamples);

    SampleUtil::free(pOutput);
}

TEST_F(EngineEffectsDelayTest, CrossfadeSecondDelayGreaterThanInputBufferSize) {
    SINT numDelayFrames = 2;
    const SINT numSamples = 8;

    m_pEffectsDelay->setDelayFrames(numDelayFrames);

    CSAMPLE inputBuffer[] = {-100.0, 100.0, -99.0, 99.0, -98.0, 98.0, -97.0, 97.0};

    CSAMPLE firstExpectedResult[] = {-100.0, 100.0, -99.0, 99.0, -98.0, 98.5, -98.0, 98.5};
    CSAMPLE secondExpectedResult[] = {-98.0, 98.0, -97.0, 97.0, -100.0, 100.0, -99.0, 99.0};
    CSAMPLE thirdExpectedResult[] = {-98.0, 98.0, -97.0, 97.0, -100.0, 99.25, -99.5, 99.75};
    CSAMPLE fourthExpectedResult[] = {-99.0, 99.0, -98.0, 98.0, -97.0, 97.0, -100.0, 100.0};

    CSAMPLE* pOutput = SampleUtil::alloc(numSamples);

    m_pEffectsDelay->process(inputBuffer, pOutput, numSamples);
    AssertIdenticalBufferEquals(pOutput, numSamples, firstExpectedResult, numSamples);

    m_pEffectsDelay->process(inputBuffer, pOutput, numSamples);
    AssertIdenticalBufferEquals(pOutput, numSamples, secondExpectedResult, numSamples);

    // Set the number of frames greater than the size of the input buffer.
    numDelayFrames = 7;
    m_pEffectsDelay->setDelayFrames(numDelayFrames);

    m_pEffectsDelay->process(inputBuffer, pOutput, numSamples);
    AssertIdenticalBufferEquals(pOutput, numSamples, thirdExpectedResult, numSamples);

    m_pEffectsDelay->process(inputBuffer, pOutput, numSamples);
    AssertIdenticalBufferEquals(pOutput, numSamples, fourthExpectedResult, numSamples);

    SampleUtil::free(pOutput);
}

TEST_F(EngineEffectsDelayTest, CrossfadeBetweenThreeNonZeroDelays) {
    SINT numDelayFrames = 3;
    const SINT numSamples = 8;

    m_pEffectsDelay->setDelayFrames(numDelayFrames);

    CSAMPLE inputBuffer[] = {-100.0, 100.0, -99.0, 99.0, -98.0, 98.0, -97.0, 97.0};

    CSAMPLE firstExpectedResult[] = {-100.0, 100.0, -99.0, 99.0, -98.0, 73.5, -98.5, 99.25};
    CSAMPLE secondExpectedResult[] = {-99.0, 99.0, -98.0, 98.0, -97.0, 97.0, -100.0, 100.0};
    CSAMPLE thirdExpectedResult[] = {-99.0, 99.0, -98.0, 98.0, -97.0, 97.5, -99.0, 98.5};
    CSAMPLE fourthExpectedResult[] = {-97.0, 97.0, -100.0, 100.0, -99.0, 99.0, -98.0, 98.0};
    CSAMPLE fifthExpectedResult[] = {-97.0, 97.0, -100.0, 100.0, -99.0, 98.5, -99.0, 99.5};
    CSAMPLE sixthExpectedResult[] = {-99.0, 99.0, -98.0, 98.0, -97.0, 97.0, -100.0, 100.0};

    CSAMPLE* pOutput = SampleUtil::alloc(numSamples);

    m_pEffectsDelay->process(inputBuffer, pOutput, numSamples);
    AssertIdenticalBufferEquals(pOutput, numSamples, firstExpectedResult, numSamples);

    m_pEffectsDelay->process(inputBuffer, pOutput, numSamples);
    AssertIdenticalBufferEquals(pOutput, numSamples, secondExpectedResult, numSamples);

    numDelayFrames = 1;
    m_pEffectsDelay->setDelayFrames(numDelayFrames);

    m_pEffectsDelay->process(inputBuffer, pOutput, numSamples);
    AssertIdenticalBufferEquals(pOutput, numSamples, thirdExpectedResult, numSamples);

    m_pEffectsDelay->process(inputBuffer, pOutput, numSamples);
    AssertIdenticalBufferEquals(pOutput, numSamples, fourthExpectedResult, numSamples);

    // Set the number of frames greater than the size of the input buffer.
    numDelayFrames = 7;
    m_pEffectsDelay->setDelayFrames(numDelayFrames);

    m_pEffectsDelay->process(inputBuffer, pOutput, numSamples);
    AssertIdenticalBufferEquals(pOutput, numSamples, fifthExpectedResult, numSamples);

    m_pEffectsDelay->process(inputBuffer, pOutput, numSamples);
    AssertIdenticalBufferEquals(pOutput, numSamples, sixthExpectedResult, numSamples);

    SampleUtil::free(pOutput);
}

TEST_F(EngineEffectsDelayTest, CopyWholeBufferForZeroDelay) {
    const SINT numSamples = 4;

    CSAMPLE inputBuffer[] = {-100.0, 100.0, -99.0, 99.0};
    CSAMPLE zeroBuffer[] = {0.0, 0.0, 0.0, 0.0};
    CSAMPLE* firstExpectedResult = inputBuffer;
    CSAMPLE* secondExpectedResult = zeroBuffer;

    CSAMPLE* pOutput = SampleUtil::alloc(numSamples);

    m_pEffectsDelay->process(inputBuffer, pOutput, numSamples);
    AssertIdenticalBufferEquals(pOutput, numSamples, firstExpectedResult, numSamples);

    m_pEffectsDelay->process(zeroBuffer, pOutput, numSamples);
    AssertIdenticalBufferEquals(pOutput, numSamples, secondExpectedResult, numSamples);

    SampleUtil::free(pOutput);
}

} // namespace
