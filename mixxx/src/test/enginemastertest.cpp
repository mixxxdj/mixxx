#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <QtDebug>

#include "defs.h"
#include "configobject.h"
#include "engine/enginemaster.h"
#include "engine/enginechannel.h"

using ::testing::Return;
using ::testing::_;

namespace {

class EngineChannelMock : public EngineChannel {
  public:
    EngineChannelMock(const char* group, ChannelOrientation defaultOrientation)
            : EngineChannel(group, defaultOrientation) {
    }

    void applyVolume(CSAMPLE* pBuff, const int iBufferSize) {
    }

    MOCK_METHOD0(isActive, bool());
    MOCK_METHOD0(isPFL, bool());
    MOCK_METHOD3(process, void(const CSAMPLE* pIn, const CSAMPLE* pOut, const int iBufferSize));
};

class EngineMasterTest : public testing::Test {
  protected:
    virtual void SetUp() {
        m_pConfig = new ConfigObject<ConfigValue>("");
        m_pMaster = new EngineMaster(m_pConfig, "[Master]", false);
    }

    virtual void TearDown() {
        delete m_pMaster;
        delete m_pConfig;
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
    EngineMaster* m_pMaster;
};

TEST_F(EngineMasterTest, SingleChannelOutputWorks) {
    EngineChannelMock* pChannel = new EngineChannelMock("[Test1]", EngineChannel::CENTER);
    m_pMaster->addChannel(pChannel);

    // Pretend that the channel processed the buffer by stuffing it with 1.0's
    CSAMPLE* pChannelBuffer = const_cast<CSAMPLE*>(m_pMaster->getChannelBuffer("[Test1]"));
    // We assume it uses MAX_BUFFER_LEN. This should probably be fixed.
    FillBuffer(pChannelBuffer, 1.0f, MAX_BUFFER_LEN);

    // Instruct the mock to claim it is active and not PFL.
    EXPECT_CALL(*pChannel, isActive())
            .Times(1)
            .WillOnce(Return(true));
    EXPECT_CALL(*pChannel, isPFL())
            .Times(1)
            .WillOnce(Return(false));

    // Instruct the mock to just return when process() gets called.
    EXPECT_CALL(*pChannel, process(_, _, MAX_BUFFER_LEN))
            .Times(1)
            .WillOnce(Return());

    m_pMaster->process(NULL, NULL, MAX_BUFFER_LEN);

    // Check that the master output contains the channel data.
    const CSAMPLE* pMasterBuffer = m_pMaster->getMasterBuffer();
    AssertWholeBufferEquals(pMasterBuffer, 1.0f, MAX_BUFFER_LEN);

    // Check that the headphone output does not contain the channel data.
    const CSAMPLE* pHeadphoneBuffer = m_pMaster->getHeadphoneBuffer();
    AssertWholeBufferEquals(pHeadphoneBuffer, 0.0f, MAX_BUFFER_LEN);
}

TEST_F(EngineMasterTest, TwoChannelOutputWorks) {
    EngineChannelMock* pChannel1 = new EngineChannelMock("[Test1]", EngineChannel::CENTER);
    m_pMaster->addChannel(pChannel1);
    EngineChannelMock* pChannel2 = new EngineChannelMock("[Test2]", EngineChannel::CENTER);
    m_pMaster->addChannel(pChannel2);

    // Pretend that the channel processed the buffer by stuffing it with 1.0's
    CSAMPLE* pChannel1Buffer = const_cast<CSAMPLE*>(m_pMaster->getChannelBuffer("[Test1]"));
    CSAMPLE* pChannel2Buffer = const_cast<CSAMPLE*>(m_pMaster->getChannelBuffer("[Test2]"));

    // We assume it uses MAX_BUFFER_LEN. This should probably be fixed.
    FillBuffer(pChannel1Buffer, 1.0f, MAX_BUFFER_LEN);
    FillBuffer(pChannel2Buffer, 2.0f, MAX_BUFFER_LEN);

    // Instruct channel 1 to claim it is active and not PFL.
    EXPECT_CALL(*pChannel1, isActive())
            .Times(1)
            .WillOnce(Return(true));
    EXPECT_CALL(*pChannel1, isPFL())
            .Times(1)
            .WillOnce(Return(false));

    // Instruct channel 2 to claim it is active and not PFL.
    EXPECT_CALL(*pChannel2, isActive())
            .Times(1)
            .WillOnce(Return(true));
    EXPECT_CALL(*pChannel2, isPFL())
            .Times(1)
            .WillOnce(Return(false));

    // Instruct the mock to just return when process() gets called.
    EXPECT_CALL(*pChannel1, process(_, _, MAX_BUFFER_LEN))
            .Times(1)
            .WillOnce(Return());
    EXPECT_CALL(*pChannel2, process(_, _, MAX_BUFFER_LEN))
            .Times(1)
            .WillOnce(Return());

    m_pMaster->process(NULL, NULL, MAX_BUFFER_LEN);

    // Check that the master output contains the sum of the channel data.
    const CSAMPLE* pMasterBuffer = m_pMaster->getMasterBuffer();
    AssertWholeBufferEquals(pMasterBuffer, 3.0f, MAX_BUFFER_LEN);

    // Check that the headphone output does not contain any channel data.
    const CSAMPLE* pHeadphoneBuffer = m_pMaster->getHeadphoneBuffer();
    AssertWholeBufferEquals(pHeadphoneBuffer, 0.0f, MAX_BUFFER_LEN);
}

TEST_F(EngineMasterTest, TwoChannelPFLOutputWorks) {
    EngineChannelMock* pChannel1 = new EngineChannelMock("[Test1]", EngineChannel::CENTER);
    m_pMaster->addChannel(pChannel1);
    EngineChannelMock* pChannel2 = new EngineChannelMock("[Test2]", EngineChannel::CENTER);
    m_pMaster->addChannel(pChannel2);

    // Pretend that the channel processed the buffer by stuffing it with 1.0's
    CSAMPLE* pChannel1Buffer = const_cast<CSAMPLE*>(m_pMaster->getChannelBuffer("[Test1]"));
    CSAMPLE* pChannel2Buffer = const_cast<CSAMPLE*>(m_pMaster->getChannelBuffer("[Test2]"));

    // We assume it uses MAX_BUFFER_LEN. This should probably be fixed.
    FillBuffer(pChannel1Buffer, 1.0f, MAX_BUFFER_LEN);
    FillBuffer(pChannel2Buffer, 2.0f, MAX_BUFFER_LEN);

    // Instruct channel 1 to claim it is active and not PFL.
    EXPECT_CALL(*pChannel1, isActive())
            .Times(1)
            .WillOnce(Return(true));
    EXPECT_CALL(*pChannel1, isPFL())
            .Times(1)
            .WillOnce(Return(true));

    // Instruct channel 2 to claim it is active and not PFL.
    EXPECT_CALL(*pChannel2, isActive())
            .Times(1)
            .WillOnce(Return(true));
    EXPECT_CALL(*pChannel2, isPFL())
            .Times(1)
            .WillOnce(Return(true));

    // Instruct the mock to just return when process() gets called.
    EXPECT_CALL(*pChannel1, process(_, _, MAX_BUFFER_LEN))
            .Times(1)
            .WillOnce(Return());
    EXPECT_CALL(*pChannel2, process(_, _, MAX_BUFFER_LEN))
            .Times(1)
            .WillOnce(Return());

    m_pMaster->process(NULL, NULL, MAX_BUFFER_LEN);

    // Check that the master output contains the sum of the channel data.
    const CSAMPLE* pMasterBuffer = m_pMaster->getMasterBuffer();
    AssertWholeBufferEquals(pMasterBuffer, 3.0f, MAX_BUFFER_LEN);

    // Check that the headphone output does not contain any channel data.
    const CSAMPLE* pHeadphoneBuffer = m_pMaster->getHeadphoneBuffer();
    AssertWholeBufferEquals(pHeadphoneBuffer, 3.0f, MAX_BUFFER_LEN);
}

TEST_F(EngineMasterTest, ThreeChannelOutputWorks) {
    EngineChannelMock* pChannel1 = new EngineChannelMock("[Test1]", EngineChannel::CENTER);
    m_pMaster->addChannel(pChannel1);
    EngineChannelMock* pChannel2 = new EngineChannelMock("[Test2]", EngineChannel::CENTER);
    m_pMaster->addChannel(pChannel2);
    EngineChannelMock* pChannel3 = new EngineChannelMock("[Test3]", EngineChannel::CENTER);
    m_pMaster->addChannel(pChannel3);

    // Pretend that the channel processed the buffer by stuffing it with 1.0's
    CSAMPLE* pChannel1Buffer = const_cast<CSAMPLE*>(m_pMaster->getChannelBuffer("[Test1]"));
    CSAMPLE* pChannel2Buffer = const_cast<CSAMPLE*>(m_pMaster->getChannelBuffer("[Test2]"));
    CSAMPLE* pChannel3Buffer = const_cast<CSAMPLE*>(m_pMaster->getChannelBuffer("[Test3]"));

    // We assume it uses MAX_BUFFER_LEN. This should probably be fixed.
    FillBuffer(pChannel1Buffer, 1.0f, MAX_BUFFER_LEN);
    FillBuffer(pChannel2Buffer, 2.0f, MAX_BUFFER_LEN);
    FillBuffer(pChannel3Buffer, 3.0f, MAX_BUFFER_LEN);

    // Instruct channel 1 to claim it is active and not PFL.
    EXPECT_CALL(*pChannel1, isActive())
            .Times(1)
            .WillOnce(Return(true));
    EXPECT_CALL(*pChannel1, isPFL())
            .Times(1)
            .WillOnce(Return(false));

    // Instruct channel 2 to claim it is active and not PFL.
    EXPECT_CALL(*pChannel2, isActive())
            .Times(1)
            .WillOnce(Return(true));
    EXPECT_CALL(*pChannel2, isPFL())
            .Times(1)
            .WillOnce(Return(false));

    // Instruct channel 3 to claim it is active and not PFL.
    EXPECT_CALL(*pChannel3, isActive())
            .Times(1)
            .WillOnce(Return(true));
    EXPECT_CALL(*pChannel3, isPFL())
            .Times(1)
            .WillOnce(Return(false));

    // Instruct the mock to just return when process() gets called.
    EXPECT_CALL(*pChannel1, process(_, _, MAX_BUFFER_LEN))
            .Times(1)
            .WillOnce(Return());
    EXPECT_CALL(*pChannel2, process(_, _, MAX_BUFFER_LEN))
            .Times(1)
            .WillOnce(Return());
    EXPECT_CALL(*pChannel3, process(_, _, MAX_BUFFER_LEN))
            .Times(1)
            .WillOnce(Return());

    m_pMaster->process(NULL, NULL, MAX_BUFFER_LEN);

    // Check that the master output contains the sum of the channel data.
    const CSAMPLE* pMasterBuffer = m_pMaster->getMasterBuffer();
    AssertWholeBufferEquals(pMasterBuffer, 6.0f, MAX_BUFFER_LEN);

    // Check that the headphone output does not contain any channel data.
    const CSAMPLE* pHeadphoneBuffer = m_pMaster->getHeadphoneBuffer();
    AssertWholeBufferEquals(pHeadphoneBuffer, 0.0f, MAX_BUFFER_LEN);
}

TEST_F(EngineMasterTest, ThreeChannelPFLOutputWorks) {
    EngineChannelMock* pChannel1 = new EngineChannelMock("[Test1]", EngineChannel::CENTER);
    m_pMaster->addChannel(pChannel1);
    EngineChannelMock* pChannel2 = new EngineChannelMock("[Test2]", EngineChannel::CENTER);
    m_pMaster->addChannel(pChannel2);
    EngineChannelMock* pChannel3 = new EngineChannelMock("[Test3]", EngineChannel::CENTER);
    m_pMaster->addChannel(pChannel3);

    // Pretend that the channel processed the buffer by stuffing it with 1.0's
    CSAMPLE* pChannel1Buffer = const_cast<CSAMPLE*>(m_pMaster->getChannelBuffer("[Test1]"));
    CSAMPLE* pChannel2Buffer = const_cast<CSAMPLE*>(m_pMaster->getChannelBuffer("[Test2]"));
    CSAMPLE* pChannel3Buffer = const_cast<CSAMPLE*>(m_pMaster->getChannelBuffer("[Test3]"));

    // We assume it uses MAX_BUFFER_LEN. This should probably be fixed.
    FillBuffer(pChannel1Buffer, 1.0f, MAX_BUFFER_LEN);
    FillBuffer(pChannel2Buffer, 2.0f, MAX_BUFFER_LEN);
    FillBuffer(pChannel3Buffer, 3.0f, MAX_BUFFER_LEN);

    // Instruct channel 1 to claim it is active and not PFL.
    EXPECT_CALL(*pChannel1, isActive())
            .Times(1)
            .WillOnce(Return(true));
    EXPECT_CALL(*pChannel1, isPFL())
            .Times(1)
            .WillOnce(Return(true));

    // Instruct channel 2 to claim it is active and not PFL.
    EXPECT_CALL(*pChannel2, isActive())
            .Times(1)
            .WillOnce(Return(true));
    EXPECT_CALL(*pChannel2, isPFL())
            .Times(1)
            .WillOnce(Return(true));

    // Instruct channel 3 to claim it is active and not PFL.
    EXPECT_CALL(*pChannel3, isActive())
            .Times(1)
            .WillOnce(Return(true));
    EXPECT_CALL(*pChannel3, isPFL())
            .Times(1)
            .WillOnce(Return(true));

    // Instruct the mock to just return when process() gets called.
    EXPECT_CALL(*pChannel1, process(_, _, MAX_BUFFER_LEN))
            .Times(1)
            .WillOnce(Return());
    EXPECT_CALL(*pChannel2, process(_, _, MAX_BUFFER_LEN))
            .Times(1)
            .WillOnce(Return());
    EXPECT_CALL(*pChannel3, process(_, _, MAX_BUFFER_LEN))
            .Times(1)
            .WillOnce(Return());

    m_pMaster->process(NULL, NULL, MAX_BUFFER_LEN);

    // Check that the master output contains the sum of the channel data.
    const CSAMPLE* pMasterBuffer = m_pMaster->getMasterBuffer();
    AssertWholeBufferEquals(pMasterBuffer, 6.0f, MAX_BUFFER_LEN);

    // Check that the headphone output does not contain any channel data.
    const CSAMPLE* pHeadphoneBuffer = m_pMaster->getHeadphoneBuffer();
    AssertWholeBufferEquals(pHeadphoneBuffer, 6.0f, MAX_BUFFER_LEN);
}

TEST_F(EngineMasterTest, SingleChannelPFLOutputWorks) {
    EngineChannelMock* pChannel = new EngineChannelMock("[Test1]", EngineChannel::CENTER);
    m_pMaster->addChannel(pChannel);

    // Pretend that the channel processed the buffer by stuffing it with 1.0's
    CSAMPLE* pChannelBuffer = const_cast<CSAMPLE*>(m_pMaster->getChannelBuffer("[Test1]"));
    // We assume it uses MAX_BUFFER_LEN. This should probably be fixed.
    FillBuffer(pChannelBuffer, 1.0f, MAX_BUFFER_LEN);

    // Instruct the mock to claim it is active and PFL
    EXPECT_CALL(*pChannel, isActive())
            .Times(1)
            .WillOnce(Return(true));
    EXPECT_CALL(*pChannel, isPFL())
            .Times(1)
            .WillOnce(Return(true));

    // Instruct the mock to just return when process() gets called.
    EXPECT_CALL(*pChannel, process(_, _, _))
            .Times(1)
            .WillOnce(Return());

    m_pMaster->process(NULL, NULL, MAX_BUFFER_LEN);

    // Check that the master output contains the channel data.
    const CSAMPLE* pMasterBuffer = m_pMaster->getMasterBuffer();
    AssertWholeBufferEquals(pMasterBuffer, 1.0f, MAX_BUFFER_LEN);

    // Check that the headphone output contains the channel data.
    const CSAMPLE* pHeadphoneBuffer = m_pMaster->getHeadphoneBuffer();
    AssertWholeBufferEquals(pHeadphoneBuffer, 1.0f, MAX_BUFFER_LEN);
}

}  // namespace
