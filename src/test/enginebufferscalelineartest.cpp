#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QtDebug>
#include <QVector>

#include "engine/bufferscalers/enginebufferscalelinear.h"
#include "engine/readaheadmanager.h"
#include "test/mixxxtest.h"
#include "util/math.h"
#include "util/sample.h"
#include "util/types.h"

using ::testing::StrictMock;
using ::testing::Return;
using ::testing::Invoke;
using ::testing::_;

namespace {

class ReadAheadManagerMock : public ReadAheadManager {
  public:
    ReadAheadManagerMock()
            : ReadAheadManager(),
              m_pBuffer(NULL),
              m_iBufferSize(0),
              m_iReadPosition(0),
              m_iSamplesRead(0) {
    }

    SINT getNextSamplesFake(double dRate, CSAMPLE* buffer, SINT requested_samples) {
        Q_UNUSED(dRate);
        bool hasBuffer = m_pBuffer != NULL;
        // You forgot to set the mock read buffer.
        EXPECT_TRUE(hasBuffer);

        for (SINT i = 0; i < requested_samples; ++i) {
            buffer[i] = hasBuffer ? m_pBuffer[m_iReadPosition++ % m_iBufferSize] : 0;
        }
        m_iSamplesRead += requested_samples;
        return requested_samples;
    }

    void setReadBuffer(CSAMPLE* pBuffer, SINT iBufferSize) {
        m_pBuffer = pBuffer;
        m_iBufferSize = iBufferSize;
        m_iReadPosition = 0;
    }

    int getSamplesRead() {
        return m_iSamplesRead;
    }

    MOCK_METHOD3(getNextSamples, SINT(double dRate, CSAMPLE* buffer, SINT requested_samples));

    CSAMPLE* m_pBuffer;
    SINT m_iBufferSize;
    SINT m_iReadPosition;
    SINT m_iSamplesRead;
};

class EngineBufferScaleLinearTest : public MixxxTest {
  protected:
    void SetUp() override {
        m_pReadAheadMock = new StrictMock<ReadAheadManagerMock>();
        m_pScaler = new EngineBufferScaleLinear(m_pReadAheadMock);
    }

    void TearDown() override {
        delete m_pScaler;
        delete m_pReadAheadMock;
    }

    void SetRate(double rate) {
        double tempoRatio = rate;
        double pitchRatio = rate;
        m_pScaler->setSampleRate(44100);
        m_pScaler->setScaleParameters(
                1.0, &tempoRatio, &pitchRatio);
    }

    void SetRateNoLerp(double rate) {
        // Set it twice to prevent rate LERP'ing
        SetRate(rate);
        SetRate(rate);
    }

    void ClearBuffer(CSAMPLE* pBuffer, int length) {
        SampleUtil::clear(pBuffer, length);
    }

    void FillBuffer(CSAMPLE* pBuffer, CSAMPLE value, int length) {
        SampleUtil::fill(pBuffer, value, length);
    }

    void AssertWholeBufferEquals(const CSAMPLE* pBuffer, CSAMPLE value, int iBufferLen) {
        for (int i = 0; i < iBufferLen; ++i) {
            EXPECT_FLOAT_EQ(value, pBuffer[i]);
        }
    }

    void AssertBufferCycles(const CSAMPLE* pBuffer, int iBufferLen,
                            CSAMPLE* pCycleBuffer, int iCycleLength) {
        int cycleRead = 0;
        for (int i = 0; i < iBufferLen; ++i) {
            //qDebug() << "i" << i << pBuffer[i] << pCycleBuffer[cycleRead % iCycleLength];
            EXPECT_FLOAT_EQ(pCycleBuffer[cycleRead++ % iCycleLength], pBuffer[i]);
        }
    }

    void AssertBufferMonotonicallyProgresses(const CSAMPLE* pBuffer,
                                             CSAMPLE start, CSAMPLE finish,
                                             int iBufferLen) {
        CSAMPLE currentLimit = start;
        bool increasing = (finish - start) > 0;

        for (int i = 0; i < iBufferLen; ++i) {
            if (increasing) {
                //qDebug() << "i" << i << pBuffer[i] << currentLimit;
                EXPECT_GE(pBuffer[i], currentLimit);
                currentLimit = pBuffer[i];
            } else {
                EXPECT_LE(pBuffer[i], currentLimit);
                currentLimit = pBuffer[i];
            }
        }
    }

    StrictMock<ReadAheadManagerMock>* m_pReadAheadMock;
    EngineBufferScaleLinear* m_pScaler;
};

TEST_F(EngineBufferScaleLinearTest, ScaleConstant) {
    SetRateNoLerp(1.0);

    CSAMPLE readBuffer[1] = { 1.0f };
    m_pReadAheadMock->setReadBuffer(readBuffer, 1);

    // Tell the RAMAN mock to invoke getNextSamplesFake
    EXPECT_CALL(*m_pReadAheadMock, getNextSamples(_, _, _))
            .WillRepeatedly(Invoke(m_pReadAheadMock, &ReadAheadManagerMock::getNextSamplesFake));

    CSAMPLE* pOutput = SampleUtil::alloc(kiLinearScaleReadAheadLength);
    m_pScaler->scaleBuffer(pOutput, kiLinearScaleReadAheadLength);
    // TODO(rryan) the LERP w/ the previous buffer causes samples 0 and 1 to be
    // 0, for now skip the first two.
    AssertWholeBufferEquals(pOutput+2, 1.0f, kiLinearScaleReadAheadLength - 2);

    // Check that the total samples read from the RAMAN is equal to the samples
    // we requested.
    ASSERT_EQ(kiLinearScaleReadAheadLength, m_pReadAheadMock->getSamplesRead());

    SampleUtil::free(pOutput);
}

TEST_F(EngineBufferScaleLinearTest, UnityRateIsSamplePerfect) {
    SetRateNoLerp(1.0);

    // Tell the RAMAN mock to invoke getNextSamplesFake
    EXPECT_CALL(*m_pReadAheadMock, getNextSamples(_, _, _))
            .WillRepeatedly(Invoke(m_pReadAheadMock, &ReadAheadManagerMock::getNextSamplesFake));

    QVector<CSAMPLE> readBuffer;
    for (int i = 0; i < 1000; ++i) {
        readBuffer.push_back(i);
    }
    m_pReadAheadMock->setReadBuffer(readBuffer.data(), readBuffer.size());

    CSAMPLE* pOutput = SampleUtil::alloc(kiLinearScaleReadAheadLength);
    m_pScaler->scaleBuffer(pOutput, kiLinearScaleReadAheadLength);

    AssertBufferCycles(pOutput, kiLinearScaleReadAheadLength,
                       readBuffer.data(), readBuffer.size());

    // Check that the total samples read from the RAMAN is equal to the samples
    // we requested.
    ASSERT_EQ(kiLinearScaleReadAheadLength, m_pReadAheadMock->getSamplesRead());

    SampleUtil::free(pOutput);
}

TEST_F(EngineBufferScaleLinearTest, TestRateLERPMonotonicallyProgresses) {
    // Starting from a rate of 0.0, we'll go to a rate of 1.0
    SetRate(0.0);
    SetRate(1.0);

    // Read all 1's
    CSAMPLE readBuffer[] = { 1.0f };
    m_pReadAheadMock->setReadBuffer(readBuffer, 1);

    // Tell the RAMAN mock to invoke getNextSamplesFake
    EXPECT_CALL(*m_pReadAheadMock, getNextSamples(_, _, _))
            .WillRepeatedly(Invoke(m_pReadAheadMock, &ReadAheadManagerMock::getNextSamplesFake));

    CSAMPLE* pOutput = SampleUtil::alloc(kiLinearScaleReadAheadLength);
    m_pScaler->scaleBuffer(pOutput, kiLinearScaleReadAheadLength);

    AssertBufferMonotonicallyProgresses(pOutput, 0.0f, 1.0f, kiLinearScaleReadAheadLength);

    SampleUtil::free(pOutput);
}

TEST_F(EngineBufferScaleLinearTest, TestDoubleSpeedSmoothlyHalvesSamples) {
    SetRateNoLerp(2.0);

    // To prove that the channels don't touch each other, we're using negative
    // values on the first channel and positive values on the second channel. If
    // a fraction of either channel were mixed into either, then we would see a
    // big shift in our desired values.
    CSAMPLE readBuffer[] = { 1.0, 1.0,
                             0.0, 0.0,
                             -1.0, -1.0,
                             0.0, 0.0 };
    m_pReadAheadMock->setReadBuffer(readBuffer, 8);

    // Tell the RAMAN mock to invoke getNextSamplesFake
    EXPECT_CALL(*m_pReadAheadMock, getNextSamples(_, _, _))
            .WillRepeatedly(Invoke(m_pReadAheadMock, &ReadAheadManagerMock::getNextSamplesFake));

    CSAMPLE* pOutput = SampleUtil::alloc(kiLinearScaleReadAheadLength);
    m_pScaler->scaleBuffer(pOutput, kiLinearScaleReadAheadLength);

    CSAMPLE expectedResult[] = { 1.0, 1.0,
                                 -1.0, -1.0 };
    AssertBufferCycles(pOutput, kiLinearScaleReadAheadLength, expectedResult, 4);

    // Check that the total samples read from the RAMAN is double the samples
    // we requested.
    ASSERT_EQ(kiLinearScaleReadAheadLength * 2, m_pReadAheadMock->getSamplesRead());

    SampleUtil::free(pOutput);
}

TEST_F(EngineBufferScaleLinearTest, TestHalfSpeedSmoothlyDoublesSamples) {
    SetRateNoLerp(0.5);

    // To prove that the channels don't touch each other, we're using negative
    // values on the first channel and positive values on the second channel. If
    // a fraction of either channel were mixed into either, then we would see a
    // big shift in our desired values.
    CSAMPLE readBuffer[] = { -101.0, 101.0,
                             -99.0, 99.0 };
    m_pReadAheadMock->setReadBuffer(readBuffer, 4);

    // Tell the RAMAN mock to invoke getNextSamplesFake
    EXPECT_CALL(*m_pReadAheadMock, getNextSamples(_, _, _))
            .WillRepeatedly(Invoke(m_pReadAheadMock, &ReadAheadManagerMock::getNextSamplesFake));

    CSAMPLE* pOutput = SampleUtil::alloc(kiLinearScaleReadAheadLength);
    m_pScaler->scaleBuffer(pOutput, kiLinearScaleReadAheadLength);

    CSAMPLE expectedResult[] = { -101.0, 101.0,
                                 -100.0, 100.0,
                                 -99.0, 99.0,
                                 -100.0, 100.0 };
    AssertBufferCycles(pOutput, kiLinearScaleReadAheadLength, expectedResult, 8);

    // Check that the total samples read from the RAMAN is half the samples we
    // requested. TODO(XXX) the extra +2 in this seems very suspicious. We need
    // to find out why this happens.
    ASSERT_EQ(kiLinearScaleReadAheadLength / 2 + 2, m_pReadAheadMock->getSamplesRead());

    SampleUtil::free(pOutput);
}

TEST_F(EngineBufferScaleLinearTest, TestRepeatedScaleCalls) {
    SetRateNoLerp(0.5);

    // To prove that the channels don't touch each other, we're using negative
    // values on the first channel and positive values on the second channel. If
    // a fraction of either channel were mixed into either, then we would see a
    // big shift in our desired values.
    CSAMPLE readBuffer[] = { -101.0, 101.0,
                             -99.0, 99.0 };
    m_pReadAheadMock->setReadBuffer(readBuffer, 4);

    // Tell the RAMAN mock to invoke getNextSamplesFake
    EXPECT_CALL(*m_pReadAheadMock, getNextSamples(_, _, _))
            .WillRepeatedly(Invoke(m_pReadAheadMock, &ReadAheadManagerMock::getNextSamplesFake));

    CSAMPLE expectedResult[] = { -101.0, 101.0,
                                 -100.0, 100.0,
                                 -99.0, 99.0,
                                 -100.0, 100.0 };

    CSAMPLE* pOutput = SampleUtil::alloc(kiLinearScaleReadAheadLength);

    int samplesRemaining = kiLinearScaleReadAheadLength;
    while (samplesRemaining > 0) {
        int toRead = math_min(8, samplesRemaining);
        m_pScaler->scaleBuffer(pOutput, 8);
        samplesRemaining -= toRead;
        AssertBufferCycles(pOutput, toRead, expectedResult, toRead);
    }

    SampleUtil::free(pOutput);
}

}  // namespace
