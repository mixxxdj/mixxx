#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QString>
#include <QtDebug>
#include <memory>
#include <tuple>
#include <vector>

#include "control/controlobject.h"
#include "engine/channels/enginechannel.h"
#include "engine/enginemixer.h"
#include "gtest/gtest.h"
#include "test/signalpathtest.h"
#include "util/sample.h"
#include "util/types.h"

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::Return;

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

class EngineMixerBandMeterTest : public BaseSignalPathTest {
  protected:
    static constexpr std::size_t kBufferSize = 2048;
    static constexpr CSAMPLE kSignal = 0.5f;
    static constexpr double kTolerance = 0.01;

    // [AI-generated] Processes enough buffers for the gain ramp and the
    // filters of the band meter to settle. [/AI-generated]
    void processUntilSettled() {
        for (int bufferIndex = 0; bufferIndex < 20; ++bufferIndex) {
            m_pEngineMixer->process(kBufferSize);
        }
    }
};

TEST_F(EngineMixerBandMeterTest, bandMetersFollowThePostFaderSignal) {
    const QString group = QStringLiteral("[TestDeck]");
    const ConfigKey lowKey(group, QStringLiteral("band_meter_low"));
    const ConfigKey midKey(group, QStringLiteral("band_meter_mid"));
    const ConfigKey highKey(group, QStringLiteral("band_meter_high"));
    const ConfigKey volumeKey(group, QStringLiteral("volume"));
    const ConfigKey enabledKey(QStringLiteral("[App]"), QStringLiteral("band_meters_enabled"));

    auto pChannelOwner = std::make_unique<EngineChannelMock>(
            group, EngineChannel::CENTER, m_pEngineMixer.get());
    EngineChannelMock* pChannel = pChannelOwner.get();
    m_pEngineMixer->addChannel(std::move(pChannelOwner));

    EXPECT_CALL(*pChannel, updateActiveState())
            .WillRepeatedly(Return(EngineChannel::ActiveState::Active));
    EXPECT_CALL(*pChannel, isActive()).WillRepeatedly(Return(true));
    EXPECT_CALL(*pChannel, isMainMixEnabled()).WillRepeatedly(Return(true));
    EXPECT_CALL(*pChannel, isPflEnabled()).WillRepeatedly(Return(false));
    EXPECT_CALL(*pChannel, collectFeatures(_)).Times(AnyNumber());
    EXPECT_CALL(*pChannel, postProcess(_)).Times(AnyNumber());
    // [AI-generated] The channel outputs a constant signal, which only the
    // low band passes. [/AI-generated]
    EXPECT_CALL(*pChannel, process(_, _))
            .WillRepeatedly([](CSAMPLE* pBuffer, std::size_t bufferSize) {
                SampleUtil::fill(pBuffer, kSignal, static_cast<SINT>(bufferSize));
            });

    processUntilSettled();
    EXPECT_DOUBLE_EQ(0.0, ControlObject::get(lowKey));

    ControlObject::set(enabledKey, 1.0);
    processUntilSettled();
    EXPECT_NEAR(kSignal, ControlObject::get(lowKey), kTolerance);
    EXPECT_NEAR(0.0, ControlObject::get(midKey), kTolerance);
    EXPECT_NEAR(0.0, ControlObject::get(highKey), kTolerance);

    ControlObject::set(volumeKey, 0.5);
    processUntilSettled();
    EXPECT_NEAR(kSignal * 0.5, ControlObject::get(lowKey), kTolerance);

    ControlObject::set(volumeKey, 0.0);
    processUntilSettled();
    EXPECT_NEAR(0.0, ControlObject::get(lowKey), kTolerance);

    ControlObject::set(volumeKey, 1.0);
    processUntilSettled();
    EXPECT_NEAR(kSignal, ControlObject::get(lowKey), kTolerance);

    ControlObject::set(enabledKey, 0.0);
    processUntilSettled();
    EXPECT_DOUBLE_EQ(0.0, ControlObject::get(lowKey));
}

} // namespace
