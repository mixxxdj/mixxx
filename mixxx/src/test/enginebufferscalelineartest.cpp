#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QtDebug>
#include <QVector>

#include "defs.h"
#include "configobject.h"
#include "engine/readaheadmanager.h"
#include "engine/enginebufferscalelinear.h"

using ::testing::StrictMock;
using ::testing::Return;
using ::testing::Invoke;
using ::testing::_;

namespace {

class ReadAheadManagerMock : public ReadAheadManager {
  public:
    ReadAheadManagerMock()
            : ReadAheadManager(NULL),
              m_pBuffer(NULL),
              m_iBufferSize(0),
              m_iReadPosition(0),
              m_iSamplesRead(0) {
    }

    int getNextSamplesFake(double dRate, CSAMPLE* buffer, int requested_samples) {
        bool hasBuffer = m_pBuffer != NULL;
        // You forgot to set the mock read buffer.
        EXPECT_TRUE(hasBuffer);

        for (int i = 0; i < requested_samples; ++i) {
            buffer[i] = hasBuffer ? m_pBuffer[m_iReadPosition++ % m_iBufferSize] : 0;
        }
        m_iSamplesRead += requested_samples;
        return requested_samples;
    }

    void setReadBuffer(CSAMPLE* pBuffer, int iBufferSize) {
        m_pBuffer = pBuffer;
        m_iBufferSize = iBufferSize;
        m_iReadPosition = 0;
    }

    int getSamplesRead() {
        return m_iSamplesRead;
    }

    MOCK_METHOD3(getNextSamples, int(double dRate, CSAMPLE* buffer, int requested_samples));

    CSAMPLE* m_pBuffer;
    int m_iBufferSize;
    int m_iReadPosition;
    int m_iSamplesRead;
};

class EngineBufferScaleLinearTest : public testing::Test {
  protected:
    virtual void SetUp() {
        m_pConfig = new ConfigObject<ConfigValue>("");
        m_pReadAheadMock = new StrictMock<ReadAheadManagerMock>();
        m_pScaler = new EngineBufferScaleLinear(m_pReadAheadMock);
    }

    virtual void TearDown() {
        delete m_pConfig;
        delete m_pScaler;
        delete m_pReadAheadMock;
    }

    void SetRate(double rate) {
        m_pScaler->setTempo(1.0);
        m_pScaler->setBaseRate(rate);
    }

    void SetRateNoLerp(double rate) {
        // Set it twice to prevent rate LERP'ing
        SetRate(rate);
        SetRate(rate);
    }

    void ClearBuffer(CSAMPLE* pBuffer, int length) {
        memset(pBuffer, 0, sizeof(pBuffer[0])*length);
    }

    void FillBuffer(CSAMPLE* pBuffer, CSAMPLE value, int length) {
        for (int i = 0; i < length; ++i) {
            pBuffer[i] = value;
        }
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

    ConfigObject<ConfigValue>* m_pConfig;
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

    CSAMPLE* pOutput = m_pScaler->getScaled(kiLinearScaleReadAheadLength);
    // TODO(rryan) the LERP w/ the previous buffer causes samples 0 and 1 to be
    // 0, for now skip the first two.
    AssertWholeBufferEquals(pOutput+2, 1.0f, kiLinearScaleReadAheadLength-2);

    // Check that the total samples read from the RAMAN is equal to the samples
    // we requested.
    ASSERT_EQ(kiLinearScaleReadAheadLength, m_pReadAheadMock->getSamplesRead());
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

    const int totalSamples = kiLinearScaleReadAheadLength;
    CSAMPLE* pOutput = m_pScaler->getScaled(totalSamples);

    AssertBufferCycles(pOutput, totalSamples,
                       readBuffer.data(), readBuffer.size());

    // Check that the total samples read from the RAMAN is equal to the samples
    // we requested.
    ASSERT_EQ(totalSamples, m_pReadAheadMock->getSamplesRead());
}

TEST_F(EngineBufferScaleLinearTest, TestRateLERPMonotonicallyProgresses) {
    // Starting from a rate of 0.0, we'll go to a rate of 1.0
    SetRate(0.0f);
    SetRate(1.0f);

    const int bufferSize = kiLinearScaleReadAheadLength;

    // Read all 1's
    CSAMPLE readBuffer[] = { 1.0f };
    m_pReadAheadMock->setReadBuffer(readBuffer, 1);

    // Tell the RAMAN mock to invoke getNextSamplesFake
    EXPECT_CALL(*m_pReadAheadMock, getNextSamples(_, _, _))
            .WillRepeatedly(Invoke(m_pReadAheadMock, &ReadAheadManagerMock::getNextSamplesFake));

    CSAMPLE* pOutput = m_pScaler->getScaled(bufferSize);

    AssertBufferMonotonicallyProgresses(pOutput, 0.0f, 1.0f, bufferSize);
}

TEST_F(EngineBufferScaleLinearTest, TestDoubleSpeedSmoothlyHalvesSamples) {
    SetRateNoLerp(2.0f);
    const int bufferSize = kiLinearScaleReadAheadLength;

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

    CSAMPLE* pOutput = m_pScaler->getScaled(bufferSize);

    CSAMPLE expectedResult[] = { 1.0, 1.0,
                                 -1.0, -1.0 };
    AssertBufferCycles(pOutput, bufferSize, expectedResult, 4);

    // Check that the total samples read from the RAMAN is double the samples
    // we requested.
    ASSERT_EQ(bufferSize*2, m_pReadAheadMock->getSamplesRead());
}

TEST_F(EngineBufferScaleLinearTest, TestHalfSpeedSmoothlyDoublesSamples) {
    SetRateNoLerp(0.5f);
    const int bufferSize = kiLinearScaleReadAheadLength;

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

    CSAMPLE* pOutput = m_pScaler->getScaled(bufferSize);

    CSAMPLE expectedResult[] = { -101.0, 101.0,
                                 -100.0, 100.0,
                                 -99.0, 99.0,
                                 -100.0, 100.0 };
    AssertBufferCycles(pOutput, bufferSize, expectedResult, 8);

    // Check that the total samples read from the RAMAN is half the samples we
    // requested. TODO(XXX) the extra +2 in this seems very suspicious. We need
    // to find out why this happens.
    ASSERT_EQ(bufferSize/2+2, m_pReadAheadMock->getSamplesRead());
}

TEST_F(EngineBufferScaleLinearTest, TestRepeatedScaleCalls) {
    SetRateNoLerp(0.5f);
    const int bufferSize = kiLinearScaleReadAheadLength;

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

    int samplesRemaining = bufferSize;
    while (samplesRemaining > 0) {
        int toRead = math_min(8, samplesRemaining);
        CSAMPLE* pOutput = m_pScaler->getScaled(8);
        samplesRemaining -= toRead;
        AssertBufferCycles(pOutput, toRead, expectedResult, toRead);
    }
}

}  // namespace
