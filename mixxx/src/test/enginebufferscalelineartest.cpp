#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QtDebug>

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
    ReadAheadManagerMock(CSAMPLE fillValue)
            : ReadAheadManager(NULL),
              m_fillValue(fillValue) {
    }

    int getNextSamplesFake(double dRate, CSAMPLE* buffer, int requested_samples) {
        for (int i = 0; i < requested_samples; ++i) {
            buffer[i] = m_fillValue;
        }
        return requested_samples;
    }

    MOCK_METHOD3(getNextSamples, int(double dRate, CSAMPLE* buffer, int requested_samples));

    CSAMPLE m_fillValue;
};

class EngineBufferScaleLinearTest : public testing::Test {
  protected:
    virtual void SetUp() {
        m_pConfig = new ConfigObject<ConfigValue>("");
        m_pReadAheadMock = new StrictMock<ReadAheadManagerMock>(1.0f);
        m_pScaler = new EngineBufferScaleLinear(m_pReadAheadMock);
    }

    virtual void TearDown() {
        delete m_pConfig;
        delete m_pScaler;
        delete m_pReadAheadMock;
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

    ConfigObject<ConfigValue>* m_pConfig;
    StrictMock<ReadAheadManagerMock>* m_pReadAheadMock;
    EngineBufferScaleLinear* m_pScaler;
};

TEST_F(EngineBufferScaleLinearTest, ScaleConstant) {
    m_pScaler->setTempo(1.0f);
    m_pScaler->setBaseRate(1.0f);

    // Tell the RAMAN mock to invoke getNextSamplesFake
    EXPECT_CALL(*m_pReadAheadMock, getNextSamples(_, _, _))
            .WillRepeatedly(Invoke(m_pReadAheadMock, &ReadAheadManagerMock::getNextSamplesFake));

    // Scale twice to get rid of the LERP'ing
    m_pScaler->scale(0, kiLinearScaleReadAheadLength, 0, 0);

    CSAMPLE* pOutput = m_pScaler->scale(0, kiLinearScaleReadAheadLength, 0, 0);
    AssertWholeBufferEquals(pOutput, 1.0f, kiLinearScaleReadAheadLength);
}

}  // namespace
