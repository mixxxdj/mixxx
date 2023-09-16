#include <gtest/gtest.h>

#include <QString>

#include "control/control.h"
#include "control/controlindicatortimer.h"
#include "control/controlproxy.h"
#include "effects/effectsmanager.h"
#include "engine/channelhandle.h"
#include "engine/enginemixer.h"
#include "mixer/playermanager.h"
#include "soundio/soundmanager.h"
#include "test/mixxxtest.h"
#include "waveform/guitick.h"

namespace {

const QString kAppGroup = QStringLiteral("[App]");
const QString kLegacyGroup = QStringLiteral("[Master]");

class ControlObjectAliasTest : public MixxxTest {
};

#ifndef MIXXX_USE_QML
TEST_F(ControlObjectAliasTest, GuiTick) {
    auto guiTick = GuiTick();

    auto periodFull = ControlProxy(ConfigKey(kAppGroup, QStringLiteral("gui_tick_full_period_s")));
    auto periodFullLegacy = ControlProxy(ConfigKey(kLegacyGroup, QStringLiteral("guiTickTime")));
    EXPECT_DOUBLE_EQ(periodFull.get(), periodFullLegacy.get());

    auto period50ms = ControlProxy(ConfigKey(kAppGroup, QStringLiteral("gui_tick_50ms_period_s")));
    auto period50msLegacy = ControlProxy(ConfigKey(kLegacyGroup, QStringLiteral("guiTick50ms")));
    EXPECT_DOUBLE_EQ(period50ms.get(), period50msLegacy.get());
}
#endif

TEST_F(ControlObjectAliasTest, ControlIndicatorTimer) {
    auto controlIndicatorTimer = mixxx::ControlIndicatorTimer();

    auto indicator250ms = ControlProxy(ConfigKey(kAppGroup, QStringLiteral("indicator_250ms")));
    auto indicator250msLegacy = ControlProxy(
            ConfigKey(kLegacyGroup, QStringLiteral("indicator_250millis")));
    EXPECT_DOUBLE_EQ(indicator250ms.get(), indicator250msLegacy.get());

    auto indicator500ms = ControlProxy(ConfigKey(kAppGroup, QStringLiteral("indicator_500ms")));
    auto indicator500msLegacy = ControlProxy(
            ConfigKey(kLegacyGroup, QStringLiteral("indicator_500millis")));
    EXPECT_DOUBLE_EQ(indicator500ms.get(), indicator500msLegacy.get());
}

TEST_F(ControlObjectAliasTest, EngineMixer) {
    auto pChannelHandleFactory = std::make_shared<ChannelHandleFactory>();
    auto pEffectsManager = std::make_shared<EffectsManager>(m_pConfig, pChannelHandleFactory);
    auto pEngineMixer = std::make_shared<EngineMixer>(
            m_pConfig,
            "[Master]",
            pEffectsManager.get(),
            pChannelHandleFactory,
            true);

    auto sampleRate = ControlProxy(ConfigKey(kAppGroup, QStringLiteral("samplerate")));
    auto sampleRateLegacy = ControlProxy(ConfigKey(kLegacyGroup, QStringLiteral("samplerate")));
    EXPECT_DOUBLE_EQ(sampleRate.get(), sampleRateLegacy.get());
}

TEST_F(ControlObjectAliasTest, PlayerManager) {
    auto pChannelHandleFactory = std::make_shared<ChannelHandleFactory>();
    auto pEffectsManager = std::make_shared<EffectsManager>(m_pConfig, pChannelHandleFactory);
    auto pEngineMixer = std::make_shared<EngineMixer>(
            m_pConfig,
            "[Master]",
            pEffectsManager.get(),
            pChannelHandleFactory,
            true);
    auto controlIndicatorTimer = mixxx::ControlIndicatorTimer();
    auto pSoundManager = std::make_shared<SoundManager>(m_pConfig, pEngineMixer.get());
    pEngineMixer->registerNonEngineChannelSoundIO(pSoundManager.get());
    auto pPlayerManager = std::make_shared<PlayerManager>(m_pConfig,
            pSoundManager.get(),
            pEffectsManager.get(),
            pEngineMixer.get());
    pPlayerManager->addConfiguredDecks();
    pPlayerManager->addSampler();

    auto numAuxiliaries = ControlProxy(ConfigKey(kAppGroup, QStringLiteral("num_auxiliaries")));
    auto numAuxiliariesLegacy = ControlProxy(
            ConfigKey(kLegacyGroup, QStringLiteral("num_auxiliaries")));
    EXPECT_DOUBLE_EQ(numAuxiliaries.get(), numAuxiliariesLegacy.get());

    auto numDecks = ControlProxy(ConfigKey(kAppGroup, QStringLiteral("num_decks")));
    auto numDecksLegacy = ControlProxy(ConfigKey(kLegacyGroup, QStringLiteral("num_decks")));
    EXPECT_DOUBLE_EQ(numDecks.get(), numDecksLegacy.get());

    auto numMicrophones = ControlProxy(ConfigKey(kAppGroup, QStringLiteral("num_microphones")));
    auto numMicrophonesLegacy = ControlProxy(
            ConfigKey(kLegacyGroup, QStringLiteral("num_microphones")));
    EXPECT_DOUBLE_EQ(numMicrophones.get(), numMicrophonesLegacy.get());

    auto numPreviewDecks = ControlProxy(ConfigKey(kAppGroup, QStringLiteral("num_preview_decks")));
    auto numPreviewDecksLegacy = ControlProxy(
            ConfigKey(kLegacyGroup, QStringLiteral("num_preview_decks")));
    EXPECT_DOUBLE_EQ(numPreviewDecks.get(), numPreviewDecksLegacy.get());

    auto numSamplers = ControlProxy(ConfigKey(kAppGroup, QStringLiteral("num_samplers")));
    auto numSamplersLegacy = ControlProxy(ConfigKey(kLegacyGroup, QStringLiteral("num_samplers")));
    EXPECT_DOUBLE_EQ(numSamplers.get(), numSamplersLegacy.get());
}

} // namespace
