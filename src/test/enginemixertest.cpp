#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QString>
#include <QtDebug>
#include <cmath>
#include <memory>
#include <tuple>
#include <vector>

#include "engine/channels/enginechannel.h"
#include "engine/enginemixer.h"
#include "gtest/gtest.h"
#include "test/signalpathtest.h"
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

    void applyVolume(CSAMPLE* pBuff, const std::size_t bufferSize) {
        Q_UNUSED(pBuff);
        Q_UNUSED(bufferSize);
    }

    MOCK_METHOD(ActiveState, updateActiveState, (), (override));
    MOCK_METHOD(bool, isActive, (), (override));
    MOCK_METHOD(bool, isMainMixEnabled, (), (override, const));
    MOCK_METHOD(bool, isPflEnabled, (), (override, const));
    MOCK_METHOD(void, process, (CSAMPLE * pInOut, const std::size_t bufferSize), (override));
    MOCK_METHOD(void, collectFeatures, (GroupFeatureState * pGroupFeatures), (override, const));
    MOCK_METHOD(void, postProcess, (const std::size_t bufferSize), (override));
};

class EngineMixerTest
        : public BaseSignalPathTest,
          public testing::WithParamInterface<std::tuple<int, bool>> {
  protected:
    void assertMainBufferMatchesGolden(const QString& testName) {
        assertBufferMatchesReference(m_pEngineMixer->getMainBuffer(),
                QStringLiteral("%1-main").arg(testName));
    };

    void assertHeadphoneBufferMatchesGolden(const QString& testName) {
        assertBufferMatchesReference(m_pEngineMixer->getHeadphoneBuffer(),
                QStringLiteral("%1-headphone").arg(testName));
    };
    void assertBuffers() {
        QString testname = QString(::testing::UnitTest::GetInstance()->current_test_info()->name());
        assertMainBufferMatchesGolden(testname);
        assertHeadphoneBufferMatchesGolden(testname);
    }

    // this contains all the ugliness...
    std::pair<EngineChannelMock*, std::span<const CSAMPLE>> makeChannel(
            const QString& group, float bufferInitValue) {
        auto pChannelOwner = std::make_unique<EngineChannelMock>(
                group, EngineChannel::CENTER, m_pEngineMixer.get());
        auto* pChannel = pChannelOwner.get();
        m_pEngineMixer->addChannel(std::move(pChannelOwner));

        // Pretend that the channel processed the buffer by filling it with values
        auto buffer = m_pEngineMixer->getChannelBuffer(group);
        SampleUtil::fill(const_cast<CSAMPLE*>(buffer.data()), bufferInitValue, buffer.size());
        return {pChannel, buffer};
    }

  public:
    static std::string nameGenerator(
            const testing::TestParamInfo<EngineMixerTest::ParamType>& info) {
        auto [channelCount, isPfl] = info.param;
        std::string name;
        switch (channelCount) {
        case 1:
            name = "Single";
            break;
        case 2:
            name = "Two";
            break;
        case 3:
            name = "Three";
            break;
        }
        name += "Channel";
        if (isPfl) {
            name += "PFL";
        }
        return name;
    };
};

INSTANTIATE_TEST_SUITE_P(EngineMixerTestSuite,
        EngineMixerTest,
        testing::Combine(testing::Values(1, 2, 3), testing::Bool()),
        EngineMixerTest::nameGenerator);

TEST_P(EngineMixerTest, OutputWorks) {
    const auto [channelCount, isPfl] = GetParam();

    std::vector<std::pair<EngineChannelMock*, std::span<const CSAMPLE>>> channels;
    channels.reserve(channelCount);
    for (int i = 1; i <= channelCount; ++i) {
        QString group = QStringLiteral("[Test%1]").arg(i);
        CSAMPLE bufferInitValue = i * 0.1f;
        channels.push_back(makeChannel(group, bufferInitValue));
    }

    for (auto [pChannel, buffer] : channels) {
        // Instruct the mock to claim it is active and main
        EXPECT_CALL(*pChannel, updateActiveState())
                .Times(1)
                .WillOnce(Return(EngineChannel::ActiveState::Active));
        // behavior needs to differ slightly depending on whether channel is PFL
        if (isPfl) {
            EXPECT_CALL(*pChannel, isActive())
                    .Times(2)
                    .WillRepeatedly(Return(true));
        } else {
            EXPECT_CALL(*pChannel, isActive())
                    .Times(1)
                    .WillOnce(Return(true));
        }
        EXPECT_CALL(*pChannel, isMainMixEnabled())
                .Times(1)
                .WillOnce(Return(true));
        EXPECT_CALL(*pChannel, isPflEnabled())
                .Times(1)
                .WillOnce(Return(isPfl));
        EXPECT_CALL(*pChannel, collectFeatures(_))
                .Times(1);
        EXPECT_CALL(*pChannel, postProcess(static_cast<int>(buffer.size())))
                .Times(1);

        // Instruct the mock to just return when process() gets called.
        EXPECT_CALL(*pChannel, process(_, static_cast<int>(buffer.size())))
                .Times(1)
                .WillOnce(Return());
    }

    m_pEngineMixer->process(static_cast<int>(channels.at(0).second.size()));

    assertBuffers();
}

// Regression tests for the headMix pot behaviour in split cue mode:
// * at headMix center the PFL ear must carry pure PFL and the other ear pure
//   main mix, both at unity gain,
// * fading towards one side must ramp down only the opposite ear,
// * the main mix must never leak into the PFL ear,
// * the PFL mix must never leak into the main mix ear,
// * neither ear may exceed unity gain.
class EngineMixerSplitCueTest : public BaseSignalPathTest {
  protected:
    EngineChannelMock* makeChannel(const QString& group, CSAMPLE value) {
        auto pChannelOwner = std::make_unique<EngineChannelMock>(
                group, EngineChannel::CENTER, m_pEngineMixer.get());
        auto* pChannel = pChannelOwner.get();
        m_pEngineMixer->addChannel(std::move(pChannelOwner));

        // Pretend that the channel renders a constant signal: a real channel
        // re-renders its buffer on every callback, so do the same from the
        // mocked process(). Any gain that the mixer applies to the channel
        // buffer in place (e.g. the main mix ramp) is therefore reset before
        // the next callback instead of accumulating across callbacks.
        EXPECT_CALL(*pChannel, process(_, _))
                .Times(testing::AtLeast(1))
                .WillRepeatedly(testing::Invoke(
                        [value](CSAMPLE* pBuffer, const std::size_t frames) {
                            SampleUtil::fill(pBuffer, value, frames);
                        }));

        // Prime the channel buffer for the very first callback.
        auto buffer = m_pEngineMixer->getChannelBuffer(group);
        SampleUtil::fill(const_cast<CSAMPLE*>(buffer.data()), value, buffer.size());
        return pChannel;
    }

    void expectActive(EngineChannelMock* pChannel, bool isPfl) {
        EXPECT_CALL(*pChannel, updateActiveState())
                .Times(testing::AtLeast(1))
                .WillRepeatedly(Return(EngineChannel::ActiveState::Active));
        EXPECT_CALL(*pChannel, isActive())
                .Times(testing::AtLeast(1))
                .WillRepeatedly(Return(true));
        EXPECT_CALL(*pChannel, isMainMixEnabled())
                .Times(testing::AtLeast(1))
                .WillRepeatedly(Return(true));
        EXPECT_CALL(*pChannel, isPflEnabled())
                .Times(testing::AtLeast(1))
                .WillRepeatedly(Return(isPfl));
        EXPECT_CALL(*pChannel, collectFeatures(_))
                .Times(testing::AtLeast(1));
        EXPECT_CALL(*pChannel, postProcess(_))
                .Times(testing::AtLeast(1));
    }

    // Process a second time so that all ramping gains (headphone main gain,
    // channel gains) have settled at their target value before asserting.
    void processSettled() {
        m_pEngineMixer->process(kProcessBufferSize);
        m_pEngineMixer->process(kProcessBufferSize);
    }

    void assertHeadphoneBufferIs(CSAMPLE left, CSAMPLE right, const std::string& context) {
        const auto head = m_pEngineMixer->getHeadphoneBuffer();
        for (std::size_t i = 0; i < static_cast<std::size_t>(kProcessBufferSize); i += 2) {
            EXPECT_NEAR(left, head[i], 1e-5)
                    << "left ear (PFL) at " << context << ", sample " << i;
            EXPECT_NEAR(right, head[i + 1], 1e-5)
                    << "right ear (main) at " << context << ", sample " << i;
            // Neither ear may exceed unity gain.
            EXPECT_LE(std::abs(head[i]), 1.0f) << context;
            EXPECT_LE(std::abs(head[i + 1]), 1.0f) << context;
        }
    }
};

TEST_F(EngineMixerSplitCueTest, HeadMixSplitCueMixing) {
    // [PflTest] feeds both the PFL cue and the main mix, [MainOnlyTest] only
    // the main mix. Different constants make any leakage of the main mix into
    // the PFL ear visible in the expected left ear values.
    constexpr CSAMPLE kPflValue = 0.5f;
    constexpr CSAMPLE kMainOnlyValue = 0.25f;
    constexpr CSAMPLE kPflMix = kPflValue;
    constexpr CSAMPLE kMainMix = kPflValue + kMainOnlyValue;

    expectActive(makeChannel(QStringLiteral("[PflTest]"), kPflValue), true);
    expectActive(makeChannel(QStringLiteral("[MainOnlyTest]"), kMainOnlyValue), false);

    ControlObject::set(ConfigKey(m_sMainGroup, "headSplit"), 1.0);

    struct Expectation {
        CSAMPLE headMix;
        CSAMPLE leftEar;
        CSAMPLE rightEar;
    };
    const Expectation expectations[] = {
            // headMix = -1: full PFL -> pure cue left, silence right
            {-1.0f, kPflMix, 0.0f},
            // faded towards PFL: only the main ear is ramped down
            {-0.5f, kPflMix, 0.5f * kMainMix},
            // center: pure PFL left, pure main right, both at unity
            {0.0f, kPflMix, kMainMix},
            // faded towards main: only the PFL ear is ramped down
            {0.5f, 0.5f * kPflMix, kMainMix},
            // headMix = +1: full main -> silence left, pure main right
            {1.0f, 0.0f, kMainMix},
    };

    for (const auto& e : expectations) {
        ControlObject::set(ConfigKey(m_sMainGroup, "headMix"), e.headMix);
        processSettled();
        assertHeadphoneBufferIs(e.leftEar,
                e.rightEar,
                QStringLiteral("split cue, headMix=%1")
                        .arg(e.headMix)
                        .toStdString());
    }
}

TEST_F(EngineMixerSplitCueTest, HeadMixStereoCueMixingUnchanged) {
    // With split cue disabled the classic crossfader-like blend between PFL
    // and main mix on both ears must be preserved.
    constexpr CSAMPLE kPflValue = 0.5f;
    constexpr CSAMPLE kMainOnlyValue = 0.25f;

    expectActive(makeChannel(QStringLiteral("[PflTest]"), kPflValue), true);
    expectActive(makeChannel(QStringLiteral("[MainOnlyTest]"), kMainOnlyValue), false);

    ControlObject::set(ConfigKey(m_sMainGroup, "headSplit"), 0.0);
    ControlObject::set(ConfigKey(m_sMainGroup, "headMix"), 0.0);
    processSettled();

    // At center both signals are mixed at half gain into both ears.
    const CSAMPLE expected = 0.5f * kPflValue + 0.5f * (kPflValue + kMainOnlyValue);
    assertHeadphoneBufferIs(expected, expected, "stereo cue, headMix=0");
}

} // namespace
