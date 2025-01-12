#include <gtest/gtest.h>

#include <QString>
#include <gsl/pointers>

#include "control/control.h"
#include "control/controlindicatortimer.h"
#include "control/controlproxy.h"
#include "effects/effectsmanager.h"
#include "engine/channelhandle.h"
#include "engine/enginemixer.h"
#include "mixer/playermanager.h"
#include "skin/skincontrols.h"
#include "soundio/soundmanager.h"
#include "test/mixxxtest.h"
#include "waveform/guitick.h"

namespace {

const QString kAppGroup = QStringLiteral("[App]");
const QString kMainGroup = QStringLiteral("[Main]");
const QString kLegacyGroup = QStringLiteral("[Master]");
const QString kSkinGroup = QStringLiteral("[Skin]");

class ControlObjectAliasTest : public MixxxTest {
};

TEST_F(ControlObjectAliasTest, GuiTick) {
    auto guiTick = GuiTick();

    auto periodFull = ControlProxy(ConfigKey(kAppGroup, QStringLiteral("gui_tick_full_period_s")));
    auto periodFullLegacy = ControlProxy(ConfigKey(kLegacyGroup, QStringLiteral("guiTickTime")));
    EXPECT_DOUBLE_EQ(periodFull.get(), periodFullLegacy.get());

    auto period50ms = ControlProxy(ConfigKey(kAppGroup, QStringLiteral("gui_tick_50ms_period_s")));
    auto period50msLegacy = ControlProxy(ConfigKey(kLegacyGroup, QStringLiteral("guiTick50ms")));
    EXPECT_DOUBLE_EQ(period50ms.get(), period50msLegacy.get());
}

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

    auto latency = ControlProxy(ConfigKey(kAppGroup, QStringLiteral("output_latency_ms")));
    auto latencyLegacy = ControlProxy(ConfigKey(kLegacyGroup, QStringLiteral("latency")));
    EXPECT_DOUBLE_EQ(latency.get(), latencyLegacy.get());

    auto audioLatencyUsage = ControlProxy(
            ConfigKey(kAppGroup, QStringLiteral("audio_latency_usage")));
    auto audioLatencyUsageLegacy = ControlProxy(
            ConfigKey(kLegacyGroup, QStringLiteral("audio_latency_usage")));
    EXPECT_DOUBLE_EQ(audioLatencyUsage.get(), audioLatencyUsageLegacy.get());

    auto audioLatencyOverload = ControlProxy(
            ConfigKey(kAppGroup, QStringLiteral("audio_latency_overload")));
    auto audioLatencyOverloadLegacy = ControlProxy(
            ConfigKey(kLegacyGroup, QStringLiteral("audio_latency_overload")));
    EXPECT_DOUBLE_EQ(audioLatencyOverload.get(), audioLatencyOverloadLegacy.get());

    auto audioLatencyOverloadCount = ControlProxy(ConfigKey(
            kAppGroup, QStringLiteral("audio_latency_overload_count")));
    auto audioLatencyOverloadCountLegacy = ControlProxy(ConfigKey(
            kLegacyGroup, QStringLiteral("audio_latency_overload_count")));
    EXPECT_DOUBLE_EQ(audioLatencyOverloadCount.get(), audioLatencyOverloadCountLegacy.get());

    auto vuMeter = ControlProxy(ConfigKey(kMainGroup, QStringLiteral("vu_meter")));
    auto vuMeterLegacy = ControlProxy(ConfigKey(kLegacyGroup, QStringLiteral("VuMeter")));
    EXPECT_DOUBLE_EQ(vuMeter.get(), vuMeterLegacy.get());

    auto vuMeterLeft = ControlProxy(ConfigKey(kMainGroup, QStringLiteral("vu_meter_left")));
    auto vuMeterLeftLegacy = ControlProxy(ConfigKey(kLegacyGroup, QStringLiteral("VuMeterL")));
    EXPECT_DOUBLE_EQ(vuMeterLeft.get(), vuMeterLeftLegacy.get());

    auto vuMeterRight = ControlProxy(ConfigKey(kMainGroup, QStringLiteral("vu_meter_right")));
    auto vuMeterRightLegacy = ControlProxy(ConfigKey(kLegacyGroup, QStringLiteral("VuMeterR")));
    EXPECT_DOUBLE_EQ(vuMeterRight.get(), vuMeterRightLegacy.get());

    auto peakIndicator = ControlProxy(ConfigKey(kMainGroup, QStringLiteral("peak_indicator")));
    auto peakIndicatorLegacy = ControlProxy(ConfigKey(kLegacyGroup, QStringLiteral("VuMeter")));
    EXPECT_DOUBLE_EQ(peakIndicator.get(), peakIndicatorLegacy.get());

    auto peakIndicatorLeft = ControlProxy(
            ConfigKey(kMainGroup, QStringLiteral("peak_indicator_left")));
    auto peakIndicatorLeftLegacy = ControlProxy(
            ConfigKey(kLegacyGroup, QStringLiteral("PeakIndicatorL")));
    EXPECT_DOUBLE_EQ(peakIndicatorLeft.get(), peakIndicatorLeftLegacy.get());

    auto peakIndicatorRight = ControlProxy(
            ConfigKey(kMainGroup, QStringLiteral("peak_indicator_right")));
    auto peakIndicatorRightLegacy = ControlProxy(
            ConfigKey(kLegacyGroup, QStringLiteral("PeakIndicatorR")));
    EXPECT_DOUBLE_EQ(peakIndicatorRight.get(), peakIndicatorRightLegacy.get());
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
    pEngineMixer->registerNonEngineChannelSoundIO(gsl::make_not_null(pSoundManager.get()));
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

TEST_F(ControlObjectAliasTest, SkinControls) {
    auto skinControls = SkinControls();

    auto showMaximizedLibary = ControlProxy(
            ConfigKey(kSkinGroup, QStringLiteral("show_maximized_library")));
    auto showMaximizedLibaryLegacy = ControlProxy(
            ConfigKey(kLegacyGroup, QStringLiteral("maximize_library")));
    EXPECT_DOUBLE_EQ(showMaximizedLibary.get(), showMaximizedLibaryLegacy.get());

    auto showEffectRack = ControlProxy(
            ConfigKey(kSkinGroup, QStringLiteral("show_effectrack")));
    auto showEffectRackLegacy = ControlProxy(
            ConfigKey(QStringLiteral("[EffectRack1]"), QStringLiteral("show")));
    EXPECT_DOUBLE_EQ(showEffectRack.get(), showEffectRackLegacy.get());

    auto showLibraryCoverArt = ControlProxy(
            ConfigKey(kSkinGroup, QStringLiteral("show_library_coverart")));
    auto showLibraryCoverArtLegacy = ControlProxy(
            ConfigKey(QStringLiteral("[Library]"), QStringLiteral("show_coverart")));
    EXPECT_DOUBLE_EQ(showLibraryCoverArt.get(), showLibraryCoverArtLegacy.get());

    auto showMicrophones = ControlProxy(
            ConfigKey(kSkinGroup, QStringLiteral("show_microphones")));
    auto showMicrophonesLegacy = ControlProxy(
            ConfigKey(QStringLiteral("[Microphone]"), QStringLiteral("show_microphone")));
    EXPECT_DOUBLE_EQ(showMicrophones.get(), showMicrophonesLegacy.get());

    auto showPreviewDecks = ControlProxy(
            ConfigKey(kSkinGroup, QStringLiteral("show_preview_decks")));
    auto showPreviewDecksLegacy = ControlProxy(
            ConfigKey(QStringLiteral("[PreviewDeck]"), QStringLiteral("show_previewdeck")));
    EXPECT_DOUBLE_EQ(showPreviewDecks.get(), showPreviewDecksLegacy.get());

    auto showSamplers = ControlProxy(
            ConfigKey(kSkinGroup, QStringLiteral("show_samplers")));
    auto showSamplersLegacy = ControlProxy(
            ConfigKey(QStringLiteral("[Samplers]"), QStringLiteral("show_samplers")));
    EXPECT_DOUBLE_EQ(showSamplers.get(), showSamplersLegacy.get());
}

} // namespace
