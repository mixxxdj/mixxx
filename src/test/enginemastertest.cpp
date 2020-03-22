#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <QtDebug>

#include "control/controlproxy.h"
#include "engine/channels/enginechannel.h"
#include "engine/enginemaster.h"
#include "test/mixxxtest.h"
#include "test/signalpathtest.h"
#include "util/defs.h"
#include "util/sample.h"
#include "util/types.h"

using ::testing::Return;
using ::testing::_;

namespace {

class EngineChannelMock : public EngineChannel {
  public:
    EngineChannelMock(const QString& group,
                      ChannelOrientation defaultOrientation,
                      EngineMaster* pMaster)
            : EngineChannel(pMaster->registerChannelGroup(group),
                            defaultOrientation) {
    }

    void applyVolume(CSAMPLE* pBuff, const int iBufferSize) {
        Q_UNUSED(pBuff);
        Q_UNUSED(iBufferSize);
    }

    MOCK_METHOD0(isActive, bool());
    MOCK_CONST_METHOD0(isMasterEnabled, bool());
    MOCK_CONST_METHOD0(isPflEnabled, bool());
    MOCK_METHOD2(process, void(CSAMPLE* pInOut, const int iBufferSize));
    MOCK_CONST_METHOD1(collectFeatures, void(GroupFeatureState* pGroupFeatures));
    MOCK_METHOD1(postProcess, void(const int iBufferSize));
};

class EngineMasterTest : public BaseSignalPathTest {
  protected:
    void assertMasterBufferMatchesGolden(const QString& testName) {
          assertBufferMatchesReference(m_pEngineMaster->getMasterBuffer(), MAX_BUFFER_LEN,
              QString("%1-master").arg(testName));
    };

    void assertHeadphoneBufferMatchesGolden(const QString& testName) {
          assertBufferMatchesReference(m_pEngineMaster->getHeadphoneBuffer(), MAX_BUFFER_LEN,
              QString("%1-headphone").arg(testName));
    };
};

TEST_F(EngineMasterTest, SingleChannelOutputWorks) {
    const QString testName = "SingleChannelOutputWorks";

    EngineChannelMock* pChannel = new EngineChannelMock(
            "[Test1]", EngineChannel::CENTER, m_pEngineMaster);
    m_pEngineMaster->addChannel(pChannel);

    // Pretend that the channel processed the buffer by stuffing it with 1.0's
    CSAMPLE* pChannelBuffer = const_cast<CSAMPLE*>(m_pEngineMaster->getChannelBuffer("[Test1]"));
    // We assume it uses MAX_BUFFER_LEN. This should probably be fixed.
    SampleUtil::fill(pChannelBuffer, 0.1f, MAX_BUFFER_LEN);

    // Instruct the mock to claim it is active, master and not PFL.
    EXPECT_CALL(*pChannel, isActive())
            .Times(1)
            .WillOnce(Return(true));
    EXPECT_CALL(*pChannel, isMasterEnabled())
            .Times(1)
            .WillOnce(Return(true));
    EXPECT_CALL(*pChannel, isPflEnabled())
            .Times(1)
            .WillOnce(Return(false));

    // Instruct the mock to just return when process() gets called.
    EXPECT_CALL(*pChannel, process(_, MAX_BUFFER_LEN))
            .Times(1)
            .WillOnce(Return());

    m_pEngineMaster->process(MAX_BUFFER_LEN);

    // Check that the master output contains the channel data.
    assertMasterBufferMatchesGolden(testName);

    // Check that the headphone output does not contain the channel data.
    assertHeadphoneBufferMatchesGolden(testName);
}

TEST_F(EngineMasterTest, SingleChannelPFLOutputWorks) {
    const QString testName = "SingleChannelPFLOutputWorks";

    EngineChannelMock* pChannel = new EngineChannelMock(
            "[Test1]", EngineChannel::CENTER, m_pEngineMaster);
    m_pEngineMaster->addChannel(pChannel);

    // Pretend that the channel processed the buffer by stuffing it with 1.0's
    CSAMPLE* pChannelBuffer = const_cast<CSAMPLE*>(m_pEngineMaster->getChannelBuffer("[Test1]"));
    // We assume it uses MAX_BUFFER_LEN. This should probably be fixed.
    SampleUtil::fill(pChannelBuffer, 0.1f, MAX_BUFFER_LEN);

    // Instruct the mock to claim it is active, not master and PFL
    EXPECT_CALL(*pChannel, isActive())
            .Times(1)
            .WillOnce(Return(true));
    EXPECT_CALL(*pChannel, isMasterEnabled())
            .Times(1)
            .WillOnce(Return(false));
    EXPECT_CALL(*pChannel, isPflEnabled())
            .Times(1)
            .WillOnce(Return(true));

    // Instruct the mock to just return when process() gets called.
    EXPECT_CALL(*pChannel, process(_, _))
            .Times(1)
            .WillOnce(Return());

    m_pEngineMaster->process(MAX_BUFFER_LEN);

    // Check that the master output is empty.
    assertMasterBufferMatchesGolden(testName);

    // Check that the headphone output contains the channel data.
    assertHeadphoneBufferMatchesGolden(testName);
}

TEST_F(EngineMasterTest, TwoChannelOutputWorks) {
    const QString testName = "TwoChannelOutputWorks";

    EngineChannelMock* pChannel1 = new EngineChannelMock(
            "[Test1]", EngineChannel::CENTER, m_pEngineMaster);
    m_pEngineMaster->addChannel(pChannel1);
    EngineChannelMock* pChannel2 = new EngineChannelMock(
            "[Test2]", EngineChannel::CENTER, m_pEngineMaster);
    m_pEngineMaster->addChannel(pChannel2);

    // Pretend that the channel processed the buffer by stuffing it with 1.0's
    CSAMPLE* pChannel1Buffer = const_cast<CSAMPLE*>(m_pEngineMaster->getChannelBuffer("[Test1]"));
    CSAMPLE* pChannel2Buffer = const_cast<CSAMPLE*>(m_pEngineMaster->getChannelBuffer("[Test2]"));

    // We assume it uses MAX_BUFFER_LEN. This should probably be fixed.
    SampleUtil::fill(pChannel1Buffer, 0.1f, MAX_BUFFER_LEN);
    SampleUtil::fill(pChannel2Buffer, 0.2f, MAX_BUFFER_LEN);

    // Instruct channel 1 to claim it is active, master and not PFL.
    EXPECT_CALL(*pChannel1, isActive())
            .Times(1)
            .WillOnce(Return(true));
    EXPECT_CALL(*pChannel1, isMasterEnabled())
            .Times(1)
            .WillOnce(Return(true));
    EXPECT_CALL(*pChannel1, isPflEnabled())
            .Times(1)
            .WillOnce(Return(false));

    // Instruct channel 2 to claim it is active, master and not PFL.
    EXPECT_CALL(*pChannel2, isActive())
            .Times(1)
            .WillOnce(Return(true));
    EXPECT_CALL(*pChannel2, isMasterEnabled())
            .Times(1)
            .WillOnce(Return(true));
    EXPECT_CALL(*pChannel2, isPflEnabled())
            .Times(1)
            .WillOnce(Return(false));

    // Instruct the mock to just return when process() gets called.
    EXPECT_CALL(*pChannel1, process(_, MAX_BUFFER_LEN))
            .Times(1)
            .WillOnce(Return());
    EXPECT_CALL(*pChannel2, process(_, MAX_BUFFER_LEN))
            .Times(1)
            .WillOnce(Return());

    m_pEngineMaster->process(MAX_BUFFER_LEN);

    // Check that the master output contains the sum of the channel data.
    assertMasterBufferMatchesGolden(testName);

    // Check that the headphone output does not contain any channel data.
    assertHeadphoneBufferMatchesGolden(testName);
}

TEST_F(EngineMasterTest, TwoChannelPFLOutputWorks) {
    const QString testName = "TwoChannelPFLOutputWorks";

    EngineChannelMock* pChannel1 = new EngineChannelMock(
            "[Test1]", EngineChannel::CENTER, m_pEngineMaster);
    m_pEngineMaster->addChannel(pChannel1);
    EngineChannelMock* pChannel2 = new EngineChannelMock(
            "[Test2]", EngineChannel::CENTER, m_pEngineMaster);
    m_pEngineMaster->addChannel(pChannel2);

    // Pretend that the channel processed the buffer by stuffing it with 1.0's
    CSAMPLE* pChannel1Buffer = const_cast<CSAMPLE*>(m_pEngineMaster->getChannelBuffer("[Test1]"));
    CSAMPLE* pChannel2Buffer = const_cast<CSAMPLE*>(m_pEngineMaster->getChannelBuffer("[Test2]"));

    // We assume it uses MAX_BUFFER_LEN. This should probably be fixed.
    SampleUtil::fill(pChannel1Buffer, 0.1f, MAX_BUFFER_LEN);
    SampleUtil::fill(pChannel2Buffer, 0.2f, MAX_BUFFER_LEN);

    // Instruct channel 1 to claim it is active, master and PFL.
    EXPECT_CALL(*pChannel1, isActive())
            .Times(1)
            .WillOnce(Return(true));
    EXPECT_CALL(*pChannel1, isMasterEnabled())
            .Times(1)
            .WillOnce(Return(true));
    EXPECT_CALL(*pChannel1, isPflEnabled())
            .Times(1)
            .WillOnce(Return(true));

    // Instruct channel 2 to claim it is active, master and PFL.
    EXPECT_CALL(*pChannel2, isActive())
            .Times(1)
            .WillOnce(Return(true));
    EXPECT_CALL(*pChannel2, isMasterEnabled())
            .Times(1)
            .WillOnce(Return(true));
    EXPECT_CALL(*pChannel2, isPflEnabled())
            .Times(1)
            .WillOnce(Return(true));

    // Instruct the mock to just return when process() gets called.
    EXPECT_CALL(*pChannel1, process(_, MAX_BUFFER_LEN))
            .Times(1)
            .WillOnce(Return());
    EXPECT_CALL(*pChannel2, process(_, MAX_BUFFER_LEN))
            .Times(1)
            .WillOnce(Return());

    m_pEngineMaster->process(MAX_BUFFER_LEN);

    // Check that the master output contains the sum of the channel data.
    assertMasterBufferMatchesGolden(testName);

    // Check that the headphone output does not contain any channel data.
    assertHeadphoneBufferMatchesGolden(testName);
}

TEST_F(EngineMasterTest, ThreeChannelOutputWorks) {
    const QString testName = "ThreeChannelOutputWorks";

    EngineChannelMock* pChannel1 = new EngineChannelMock(
            "[Test1]", EngineChannel::CENTER, m_pEngineMaster);
    m_pEngineMaster->addChannel(pChannel1);
    EngineChannelMock* pChannel2 = new EngineChannelMock(
            "[Test2]", EngineChannel::CENTER, m_pEngineMaster);
    m_pEngineMaster->addChannel(pChannel2);
    EngineChannelMock* pChannel3 = new EngineChannelMock(
            "[Test3]", EngineChannel::CENTER, m_pEngineMaster);
    m_pEngineMaster->addChannel(pChannel3);

    // Pretend that the channel processed the buffer by stuffing it with 1.0's
    CSAMPLE* pChannel1Buffer = const_cast<CSAMPLE*>(m_pEngineMaster->getChannelBuffer("[Test1]"));
    CSAMPLE* pChannel2Buffer = const_cast<CSAMPLE*>(m_pEngineMaster->getChannelBuffer("[Test2]"));
    CSAMPLE* pChannel3Buffer = const_cast<CSAMPLE*>(m_pEngineMaster->getChannelBuffer("[Test3]"));

    // We assume it uses MAX_BUFFER_LEN. This should probably be fixed.
    SampleUtil::fill(pChannel1Buffer, 0.1f, MAX_BUFFER_LEN);
    SampleUtil::fill(pChannel2Buffer, 0.2f, MAX_BUFFER_LEN);
    SampleUtil::fill(pChannel3Buffer, 0.3f, MAX_BUFFER_LEN);

    // Instruct channel 1 to claim it is active, master and not PFL.
    EXPECT_CALL(*pChannel1, isActive())
            .Times(1)
            .WillOnce(Return(true));
    EXPECT_CALL(*pChannel1, isMasterEnabled())
            .Times(1)
            .WillOnce(Return(true));
    EXPECT_CALL(*pChannel1, isPflEnabled())
            .Times(1)
            .WillOnce(Return(false));

    // Instruct channel 2 to claim it is active, master and not PFL.
    EXPECT_CALL(*pChannel2, isActive())
            .Times(1)
            .WillOnce(Return(true));
    EXPECT_CALL(*pChannel2, isMasterEnabled())
            .Times(1)
            .WillOnce(Return(true));
    EXPECT_CALL(*pChannel2, isPflEnabled())
            .Times(1)
            .WillOnce(Return(false));

    // Instruct channel 3 to claim it is active, master and not PFL.
    EXPECT_CALL(*pChannel3, isActive())
            .Times(1)
            .WillOnce(Return(true));
    EXPECT_CALL(*pChannel3, isMasterEnabled())
            .Times(1)
            .WillOnce(Return(true));
    EXPECT_CALL(*pChannel3, isPflEnabled())
            .Times(1)
            .WillOnce(Return(false));

    // Instruct the mock to just return when process() gets called.
    EXPECT_CALL(*pChannel1, process(_, MAX_BUFFER_LEN))
            .Times(1)
            .WillOnce(Return());
    EXPECT_CALL(*pChannel2, process(_, MAX_BUFFER_LEN))
            .Times(1)
            .WillOnce(Return());
    EXPECT_CALL(*pChannel3, process(_, MAX_BUFFER_LEN))
            .Times(1)
            .WillOnce(Return());

    m_pEngineMaster->process(MAX_BUFFER_LEN);

    // Check that the master output contains the sum of the channel data.
    assertMasterBufferMatchesGolden(testName);

    // Check that the headphone output does not contain any channel data.
    assertHeadphoneBufferMatchesGolden(testName);
}

TEST_F(EngineMasterTest, ThreeChannelPFLOutputWorks) {
    const QString testName = "ThreeChannelPFLOutputWorks";

    EngineChannelMock* pChannel1 = new EngineChannelMock(
            "[Test1]", EngineChannel::CENTER, m_pEngineMaster);
    m_pEngineMaster->addChannel(pChannel1);
    EngineChannelMock* pChannel2 = new EngineChannelMock(
            "[Test2]", EngineChannel::CENTER, m_pEngineMaster);
    m_pEngineMaster->addChannel(pChannel2);
    EngineChannelMock* pChannel3 = new EngineChannelMock(
            "[Test3]", EngineChannel::CENTER, m_pEngineMaster);
    m_pEngineMaster->addChannel(pChannel3);

    // Pretend that the channel processed the buffer by stuffing it with 1.0's
    CSAMPLE* pChannel1Buffer = const_cast<CSAMPLE*>(m_pEngineMaster->getChannelBuffer("[Test1]"));
    CSAMPLE* pChannel2Buffer = const_cast<CSAMPLE*>(m_pEngineMaster->getChannelBuffer("[Test2]"));
    CSAMPLE* pChannel3Buffer = const_cast<CSAMPLE*>(m_pEngineMaster->getChannelBuffer("[Test3]"));

    // We assume it uses MAX_BUFFER_LEN. This should probably be fixed.
    SampleUtil::fill(pChannel1Buffer, 0.1f, MAX_BUFFER_LEN);
    SampleUtil::fill(pChannel2Buffer, 0.2f, MAX_BUFFER_LEN);
    SampleUtil::fill(pChannel3Buffer, 0.3f, MAX_BUFFER_LEN);

    // Instruct channel 1 to claim it is active, master and not PFL.
    EXPECT_CALL(*pChannel1, isActive())
            .Times(1)
            .WillOnce(Return(true));
    EXPECT_CALL(*pChannel1, isMasterEnabled())
            .Times(1)
            .WillOnce(Return(true));
    EXPECT_CALL(*pChannel1, isPflEnabled())
            .Times(1)
            .WillOnce(Return(true));

    // Instruct channel 2 to claim it is active, master and not PFL.
    EXPECT_CALL(*pChannel2, isActive())
            .Times(1)
            .WillOnce(Return(true));
    EXPECT_CALL(*pChannel2, isMasterEnabled())
            .Times(1)
            .WillOnce(Return(true));
    EXPECT_CALL(*pChannel2, isPflEnabled())
            .Times(1)
            .WillOnce(Return(true));

    // Instruct channel 3 to claim it is active, master and not PFL.
    EXPECT_CALL(*pChannel3, isActive())
            .Times(1)
            .WillOnce(Return(true));
    EXPECT_CALL(*pChannel3, isMasterEnabled())
            .Times(1)
            .WillOnce(Return(true));
    EXPECT_CALL(*pChannel3, isPflEnabled())
            .Times(1)
            .WillOnce(Return(true));

    // Instruct the mock to just return when process() gets called.
    EXPECT_CALL(*pChannel1, process(_, MAX_BUFFER_LEN))
            .Times(1)
            .WillOnce(Return());
    EXPECT_CALL(*pChannel2, process(_, MAX_BUFFER_LEN))
            .Times(1)
            .WillOnce(Return());
    EXPECT_CALL(*pChannel3, process(_, MAX_BUFFER_LEN))
            .Times(1)
            .WillOnce(Return());

    m_pEngineMaster->process(MAX_BUFFER_LEN);

    // Check that the master output contains the sum of the channel data.
    assertMasterBufferMatchesGolden(testName);

    // Check that the headphone output does not contain any channel data.
    assertHeadphoneBufferMatchesGolden(testName);
}

}  // namespace
