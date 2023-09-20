#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QtDebug>

#include "control/controlproxy.h"
#include "engine/channels/enginechannel.h"
#include "engine/enginemixer.h"
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
            EngineMixer* pEngineMixer)
            : EngineChannel(pEngineMixer->registerChannelGroup(group),
                      defaultOrientation,
                      nullptr,
                      /*isTalkoverChannel*/ false,
                      /*isPrimarydeck*/ true) {
    }

    void applyVolume(CSAMPLE* pBuff, const int iBufferSize) {
        Q_UNUSED(pBuff);
        Q_UNUSED(iBufferSize);
    }

    MOCK_METHOD0(updateActiveState, ActiveState());
    MOCK_METHOD0(isActive, bool());
    MOCK_CONST_METHOD0(isMainMixEnabled, bool());
    MOCK_CONST_METHOD0(isPflEnabled, bool());
    MOCK_METHOD2(process, void(CSAMPLE* pInOut, const int iBufferSize));
    MOCK_CONST_METHOD1(collectFeatures, void(GroupFeatureState* pGroupFeatures));
    MOCK_METHOD1(postProcess, void(const int iBufferSize));
};

class EngineMixerTest : public BaseSignalPathTest {
  protected:
    void assertMainBufferMatchesGolden(const QString& testName) {
        assertBufferMatchesReference(m_pEngineMixer->getMainBuffer(),
                kMaxEngineSamples,
                QString("%1-main").arg(testName));
    };

    void assertHeadphoneBufferMatchesGolden(const QString& testName) {
        assertBufferMatchesReference(m_pEngineMixer->getHeadphoneBuffer(),
                kMaxEngineSamples,
                QString("%1-headphone").arg(testName));
    };
};

TEST_F(EngineMixerTest, SingleChannelOutputWorks) {
    const QString testName = "SingleChannelOutputWorks";

    EngineChannelMock* pChannel = new EngineChannelMock(
            "[Test1]", EngineChannel::CENTER, m_pEngineMixer);
    m_pEngineMixer->addChannel(pChannel);

    // Pretend that the channel processed the buffer by stuffing it with 1.0's
    CSAMPLE* pChannelBuffer = const_cast<CSAMPLE*>(m_pEngineMixer->getChannelBuffer("[Test1]"));
    // We assume it uses MAX_BUFFER_LEN. This should probably be fixed.
    SampleUtil::fill(pChannelBuffer, 0.1f, kMaxEngineSamples);

    // Instruct the mock to claim it is active, main and not PFL.
    EXPECT_CALL(*pChannel, updateActiveState())
            .Times(1)
            .WillOnce(Return(EngineChannel::ActiveState::Active));
    EXPECT_CALL(*pChannel, isActive())
            .Times(1)
            .WillOnce(Return(true));
    EXPECT_CALL(*pChannel, isMainMixEnabled())
            .Times(1)
            .WillOnce(Return(true));
    EXPECT_CALL(*pChannel, isPflEnabled())
            .Times(1)
            .WillOnce(Return(false));
    EXPECT_CALL(*pChannel, collectFeatures(_))
            .Times(1);
    EXPECT_CALL(*pChannel, postProcess(kMaxEngineSamples))
            .Times(1);

    // Instruct the mock to just return when process() gets called.
    EXPECT_CALL(*pChannel, process(_, kMaxEngineSamples))
            .Times(1)
            .WillOnce(Return());

    m_pEngineMixer->process(kMaxEngineSamples);

    // Check that the main output contains the channel data.
    assertMainBufferMatchesGolden(testName);

    // Check that the headphone output does not contain the channel data.
    assertHeadphoneBufferMatchesGolden(testName);
}

TEST_F(EngineMixerTest, SingleChannelPFLOutputWorks) {
    const QString testName = "SingleChannelPFLOutputWorks";

    EngineChannelMock* pChannel = new EngineChannelMock(
            "[Test1]", EngineChannel::CENTER, m_pEngineMixer);
    m_pEngineMixer->addChannel(pChannel);

    // Pretend that the channel processed the buffer by stuffing it with 1.0's
    CSAMPLE* pChannelBuffer = const_cast<CSAMPLE*>(m_pEngineMixer->getChannelBuffer("[Test1]"));
    // We assume it uses MAX_BUFFER_LEN. This should probably be fixed.
    SampleUtil::fill(pChannelBuffer, 0.1f, kMaxEngineSamples);

    // Instruct the mock to claim it is active, not main and PFL
    EXPECT_CALL(*pChannel, updateActiveState())
            .Times(1)
            .WillOnce(Return(EngineChannel::ActiveState::Active));
    EXPECT_CALL(*pChannel, isActive())
            .Times(1)
            .WillOnce(Return(true));
    EXPECT_CALL(*pChannel, isMainMixEnabled())
            .Times(1)
            .WillOnce(Return(false));
    EXPECT_CALL(*pChannel, isPflEnabled())
            .Times(1)
            .WillOnce(Return(true));
    EXPECT_CALL(*pChannel, collectFeatures(_))
            .Times(1);
    EXPECT_CALL(*pChannel, postProcess(kMaxEngineSamples))
            .Times(1);

    // Instruct the mock to just return when process() gets called.
    EXPECT_CALL(*pChannel, process(_, _))
            .Times(1)
            .WillOnce(Return());

    m_pEngineMixer->process(kMaxEngineSamples);

    // Check that the main output is empty.
    assertMainBufferMatchesGolden(testName);

    // Check that the headphone output contains the channel data.
    assertHeadphoneBufferMatchesGolden(testName);
}

TEST_F(EngineMixerTest, TwoChannelOutputWorks) {
    const QString testName = "TwoChannelOutputWorks";

    EngineChannelMock* pChannel1 = new EngineChannelMock(
            "[Test1]", EngineChannel::CENTER, m_pEngineMixer);
    m_pEngineMixer->addChannel(pChannel1);
    EngineChannelMock* pChannel2 = new EngineChannelMock(
            "[Test2]", EngineChannel::CENTER, m_pEngineMixer);
    m_pEngineMixer->addChannel(pChannel2);

    // Pretend that the channel processed the buffer by stuffing it with 1.0's
    CSAMPLE* pChannel1Buffer = const_cast<CSAMPLE*>(m_pEngineMixer->getChannelBuffer("[Test1]"));
    CSAMPLE* pChannel2Buffer = const_cast<CSAMPLE*>(m_pEngineMixer->getChannelBuffer("[Test2]"));

    // We assume it uses MAX_BUFFER_LEN. This should probably be fixed.
    SampleUtil::fill(pChannel1Buffer, 0.1f, kMaxEngineSamples);
    SampleUtil::fill(pChannel2Buffer, 0.2f, kMaxEngineSamples);

    // Instruct channel 1 to claim it is active, main and not PFL.
    EXPECT_CALL(*pChannel1, updateActiveState())
            .Times(1)
            .WillOnce(Return(EngineChannel::ActiveState::Active));
    EXPECT_CALL(*pChannel1, isActive())
            .Times(1)
            .WillOnce(Return(true));
    EXPECT_CALL(*pChannel1, isMainMixEnabled())
            .Times(1)
            .WillOnce(Return(true));
    EXPECT_CALL(*pChannel1, isPflEnabled())
            .Times(1)
            .WillOnce(Return(false));
    EXPECT_CALL(*pChannel1, collectFeatures(_))
            .Times(1);
    EXPECT_CALL(*pChannel1, postProcess(kMaxEngineSamples))
            .Times(1);

    // Instruct channel 2 to claim it is active, main and not PFL.
    EXPECT_CALL(*pChannel2, updateActiveState())
            .Times(1)
            .WillOnce(Return(EngineChannel::ActiveState::Active));
    EXPECT_CALL(*pChannel2, isActive())
            .Times(1)
            .WillOnce(Return(true));
    EXPECT_CALL(*pChannel2, isMainMixEnabled())
            .Times(1)
            .WillOnce(Return(true));
    EXPECT_CALL(*pChannel2, isPflEnabled())
            .Times(1)
            .WillOnce(Return(false));
    EXPECT_CALL(*pChannel2, collectFeatures(_))
            .Times(1);
    EXPECT_CALL(*pChannel2, postProcess(kMaxEngineSamples))
            .Times(1);

    // Instruct the mock to just return when process() gets called.
    EXPECT_CALL(*pChannel1, process(_, kMaxEngineSamples))
            .Times(1)
            .WillOnce(Return());
    EXPECT_CALL(*pChannel2, process(_, kMaxEngineSamples))
            .Times(1)
            .WillOnce(Return());

    m_pEngineMixer->process(kMaxEngineSamples);

    // Check that the main output contains the sum of the channel data.
    assertMainBufferMatchesGolden(testName);

    // Check that the headphone output does not contain any channel data.
    assertHeadphoneBufferMatchesGolden(testName);
}

TEST_F(EngineMixerTest, TwoChannelPFLOutputWorks) {
    const QString testName = "TwoChannelPFLOutputWorks";

    EngineChannelMock* pChannel1 = new EngineChannelMock(
            "[Test1]", EngineChannel::CENTER, m_pEngineMixer);
    m_pEngineMixer->addChannel(pChannel1);
    EngineChannelMock* pChannel2 = new EngineChannelMock(
            "[Test2]", EngineChannel::CENTER, m_pEngineMixer);
    m_pEngineMixer->addChannel(pChannel2);

    // Pretend that the channel processed the buffer by stuffing it with 1.0's
    CSAMPLE* pChannel1Buffer = const_cast<CSAMPLE*>(m_pEngineMixer->getChannelBuffer("[Test1]"));
    CSAMPLE* pChannel2Buffer = const_cast<CSAMPLE*>(m_pEngineMixer->getChannelBuffer("[Test2]"));

    // We assume it uses MAX_BUFFER_LEN. This should probably be fixed.
    SampleUtil::fill(pChannel1Buffer, 0.1f, kMaxEngineSamples);
    SampleUtil::fill(pChannel2Buffer, 0.2f, kMaxEngineSamples);

    // Instruct channel 1 to claim it is active, main and PFL.
    EXPECT_CALL(*pChannel1, updateActiveState())
            .Times(1)
            .WillOnce(Return(EngineChannel::ActiveState::Active));
    EXPECT_CALL(*pChannel1, isActive())
            .Times(2)
            .WillRepeatedly(Return(true));
    EXPECT_CALL(*pChannel1, isMainMixEnabled())
            .Times(1)
            .WillOnce(Return(true));
    EXPECT_CALL(*pChannel1, isPflEnabled())
            .Times(1)
            .WillOnce(Return(true));
    EXPECT_CALL(*pChannel1, collectFeatures(_))
            .Times(1);
    EXPECT_CALL(*pChannel1, postProcess(kMaxEngineSamples))
            .Times(1);

    // Instruct channel 2 to claim it is active, main and PFL.
    EXPECT_CALL(*pChannel2, updateActiveState())
            .Times(1)
            .WillOnce(Return(EngineChannel::ActiveState::Active));
    EXPECT_CALL(*pChannel2, isActive())
            .Times(2)
            .WillRepeatedly(Return(true));
    EXPECT_CALL(*pChannel2, isMainMixEnabled())
            .Times(1)
            .WillOnce(Return(true));
    EXPECT_CALL(*pChannel2, isPflEnabled())
            .Times(1)
            .WillOnce(Return(true));
    EXPECT_CALL(*pChannel2, collectFeatures(_))
            .Times(1);
    EXPECT_CALL(*pChannel2, postProcess(kMaxEngineSamples))
            .Times(1);

    // Instruct the mock to just return when process() gets called.
    EXPECT_CALL(*pChannel1, process(_, kMaxEngineSamples))
            .Times(1)
            .WillOnce(Return());
    EXPECT_CALL(*pChannel2, process(_, kMaxEngineSamples))
            .Times(1)
            .WillOnce(Return());

    m_pEngineMixer->process(kMaxEngineSamples);

    // Check that the main output contains the sum of the channel data.
    assertMainBufferMatchesGolden(testName);

    // Check that the headphone output does not contain any channel data.
    assertHeadphoneBufferMatchesGolden(testName);
}

TEST_F(EngineMixerTest, ThreeChannelOutputWorks) {
    const QString testName = "ThreeChannelOutputWorks";

    EngineChannelMock* pChannel1 = new EngineChannelMock(
            "[Test1]", EngineChannel::CENTER, m_pEngineMixer);
    m_pEngineMixer->addChannel(pChannel1);
    EngineChannelMock* pChannel2 = new EngineChannelMock(
            "[Test2]", EngineChannel::CENTER, m_pEngineMixer);
    m_pEngineMixer->addChannel(pChannel2);
    EngineChannelMock* pChannel3 = new EngineChannelMock(
            "[Test3]", EngineChannel::CENTER, m_pEngineMixer);
    m_pEngineMixer->addChannel(pChannel3);

    // Pretend that the channel processed the buffer by stuffing it with 1.0's
    CSAMPLE* pChannel1Buffer = const_cast<CSAMPLE*>(m_pEngineMixer->getChannelBuffer("[Test1]"));
    CSAMPLE* pChannel2Buffer = const_cast<CSAMPLE*>(m_pEngineMixer->getChannelBuffer("[Test2]"));
    CSAMPLE* pChannel3Buffer = const_cast<CSAMPLE*>(m_pEngineMixer->getChannelBuffer("[Test3]"));

    // We assume it uses MAX_BUFFER_LEN. This should probably be fixed.
    SampleUtil::fill(pChannel1Buffer, 0.1f, kMaxEngineSamples);
    SampleUtil::fill(pChannel2Buffer, 0.2f, kMaxEngineSamples);
    SampleUtil::fill(pChannel3Buffer, 0.3f, kMaxEngineSamples);

    // Instruct channel 1 to claim it is active, main and not PFL.
    EXPECT_CALL(*pChannel1, updateActiveState())
            .Times(1)
            .WillOnce(Return(EngineChannel::ActiveState::Active));
    EXPECT_CALL(*pChannel1, isActive())
            .Times(1)
            .WillOnce(Return(true));
    EXPECT_CALL(*pChannel1, isMainMixEnabled())
            .Times(1)
            .WillOnce(Return(true));
    EXPECT_CALL(*pChannel1, isPflEnabled())
            .Times(1)
            .WillOnce(Return(false));
    EXPECT_CALL(*pChannel1, collectFeatures(_))
            .Times(1);
    EXPECT_CALL(*pChannel1, postProcess(kMaxEngineSamples))
            .Times(1);

    // Instruct channel 2 to claim it is active, main and not PFL.
    EXPECT_CALL(*pChannel2, updateActiveState())
            .Times(1)
            .WillOnce(Return(EngineChannel::ActiveState::Active));
    EXPECT_CALL(*pChannel2, isActive())
            .Times(1)
            .WillOnce(Return(true));
    EXPECT_CALL(*pChannel2, isMainMixEnabled())
            .Times(1)
            .WillOnce(Return(true));
    EXPECT_CALL(*pChannel2, isPflEnabled())
            .Times(1)
            .WillOnce(Return(false));
    EXPECT_CALL(*pChannel2, collectFeatures(_))
            .Times(1);
    EXPECT_CALL(*pChannel2, postProcess(kMaxEngineSamples))
            .Times(1);

    // Instruct channel 3 to claim it is active, main and not PFL.
    EXPECT_CALL(*pChannel3, updateActiveState())
            .Times(1)
            .WillOnce(Return(EngineChannel::ActiveState::Active));
    EXPECT_CALL(*pChannel3, isActive())
            .Times(1)
            .WillOnce(Return(true));
    EXPECT_CALL(*pChannel3, isMainMixEnabled())
            .Times(1)
            .WillOnce(Return(true));
    EXPECT_CALL(*pChannel3, isPflEnabled())
            .Times(1)
            .WillOnce(Return(false));
    EXPECT_CALL(*pChannel3, collectFeatures(_))
            .Times(1);
    EXPECT_CALL(*pChannel3, postProcess(kMaxEngineSamples))
            .Times(1);

    // Instruct the mock to just return when process() gets called.
    EXPECT_CALL(*pChannel1, process(_, kMaxEngineSamples))
            .Times(1)
            .WillOnce(Return());
    EXPECT_CALL(*pChannel2, process(_, kMaxEngineSamples))
            .Times(1)
            .WillOnce(Return());
    EXPECT_CALL(*pChannel3, process(_, kMaxEngineSamples))
            .Times(1)
            .WillOnce(Return());

    m_pEngineMixer->process(kMaxEngineSamples);

    // Check that the main output contains the sum of the channel data.
    assertMainBufferMatchesGolden(testName);

    // Check that the headphone output does not contain any channel data.
    assertHeadphoneBufferMatchesGolden(testName);
}

TEST_F(EngineMixerTest, ThreeChannelPFLOutputWorks) {
    const QString testName = "ThreeChannelPFLOutputWorks";

    EngineChannelMock* pChannel1 = new EngineChannelMock(
            "[Test1]", EngineChannel::CENTER, m_pEngineMixer);
    m_pEngineMixer->addChannel(pChannel1);
    EngineChannelMock* pChannel2 = new EngineChannelMock(
            "[Test2]", EngineChannel::CENTER, m_pEngineMixer);
    m_pEngineMixer->addChannel(pChannel2);
    EngineChannelMock* pChannel3 = new EngineChannelMock(
            "[Test3]", EngineChannel::CENTER, m_pEngineMixer);
    m_pEngineMixer->addChannel(pChannel3);

    // Pretend that the channel processed the buffer by stuffing it with 1.0's
    CSAMPLE* pChannel1Buffer = const_cast<CSAMPLE*>(m_pEngineMixer->getChannelBuffer("[Test1]"));
    CSAMPLE* pChannel2Buffer = const_cast<CSAMPLE*>(m_pEngineMixer->getChannelBuffer("[Test2]"));
    CSAMPLE* pChannel3Buffer = const_cast<CSAMPLE*>(m_pEngineMixer->getChannelBuffer("[Test3]"));

    // We assume it uses MAX_BUFFER_LEN. This should probably be fixed.
    SampleUtil::fill(pChannel1Buffer, 0.1f, kMaxEngineSamples);
    SampleUtil::fill(pChannel2Buffer, 0.2f, kMaxEngineSamples);
    SampleUtil::fill(pChannel3Buffer, 0.3f, kMaxEngineSamples);

    // Instruct channel 1 to claim it is active, main and not PFL.
    EXPECT_CALL(*pChannel1, updateActiveState())
            .Times(1)
            .WillOnce(Return(EngineChannel::ActiveState::Active));
    EXPECT_CALL(*pChannel1, isActive())
            .Times(2)
            .WillRepeatedly(Return(true));
    EXPECT_CALL(*pChannel1, isMainMixEnabled())
            .Times(1)
            .WillOnce(Return(true));
    EXPECT_CALL(*pChannel1, isPflEnabled())
            .Times(1)
            .WillOnce(Return(true));
    EXPECT_CALL(*pChannel1, collectFeatures(_))
            .Times(1);
    EXPECT_CALL(*pChannel1, postProcess(kMaxEngineSamples))
            .Times(1);

    // Instruct channel 2 to claim it is active, main and not PFL.
    EXPECT_CALL(*pChannel2, updateActiveState())
            .Times(1)
            .WillOnce(Return(EngineChannel::ActiveState::Active));
    EXPECT_CALL(*pChannel2, isActive())
            .Times(2)
            .WillRepeatedly(Return(true));
    EXPECT_CALL(*pChannel2, isMainMixEnabled())
            .Times(1)
            .WillOnce(Return(true));
    EXPECT_CALL(*pChannel2, isPflEnabled())
            .Times(1)
            .WillOnce(Return(true));
    EXPECT_CALL(*pChannel2, collectFeatures(_))
            .Times(1);
    EXPECT_CALL(*pChannel2, postProcess(kMaxEngineSamples))
            .Times(1);

    // Instruct channel 3 to claim it is active, main and not PFL.
    EXPECT_CALL(*pChannel3, updateActiveState())
            .Times(1)
            .WillOnce(Return(EngineChannel::ActiveState::Active));
    EXPECT_CALL(*pChannel3, isActive())
            .Times(2)
            .WillRepeatedly(Return(true));
    EXPECT_CALL(*pChannel3, isMainMixEnabled())
            .Times(1)
            .WillOnce(Return(true));
    EXPECT_CALL(*pChannel3, isPflEnabled())
            .Times(1)
            .WillOnce(Return(true));
    EXPECT_CALL(*pChannel3, collectFeatures(_))
            .Times(1);
    EXPECT_CALL(*pChannel3, postProcess(kMaxEngineSamples))
            .Times(1);

    // Instruct the mock to just return when process() gets called.
    EXPECT_CALL(*pChannel1, process(_, kMaxEngineSamples))
            .Times(1)
            .WillOnce(Return());
    EXPECT_CALL(*pChannel2, process(_, kMaxEngineSamples))
            .Times(1)
            .WillOnce(Return());
    EXPECT_CALL(*pChannel3, process(_, kMaxEngineSamples))
            .Times(1)
            .WillOnce(Return());

    m_pEngineMixer->process(kMaxEngineSamples);

    // Check that the main output contains the sum of the channel data.
    assertMainBufferMatchesGolden(testName);

    // Check that the headphone output does not contain any channel data.
    assertHeadphoneBufferMatchesGolden(testName);
}

}  // namespace
