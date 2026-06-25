#include "skin/skincontrols.h"

#include <QString>

namespace {
const QString kSkinGroup = QStringLiteral("[Skin]");
} // namespace

SkinControls::SkinControls()
        : m_showEffectRack(
                  ConfigKey(kSkinGroup, QStringLiteral("show_effectrack")),
                  true,
                  true),
          m_showLibraryCoverArt(
                  ConfigKey(
                          kSkinGroup, QStringLiteral("show_library_coverart")),
                  true,
                  true),
          m_showMicrophones(
                  ConfigKey(kSkinGroup, QStringLiteral("show_microphones")),
                  true,
                  true),
          m_showPreviewDecks(
                  ConfigKey(kSkinGroup, QStringLiteral("show_preview_decks")),
                  true,
                  true),
          m_showSamplers(ConfigKey(kSkinGroup, QStringLiteral("show_samplers")),
                  true,
                  true),
          m_show4EffectUnits(
                  ConfigKey(kSkinGroup, QStringLiteral("show_4effectunits")),
                  true,
                  false),
          m_showCoverArt(ConfigKey(kSkinGroup, QStringLiteral("show_coverart")),
                  true,
                  true),
          m_showMaximizedLibrary(
                  ConfigKey(
                          kSkinGroup, QStringLiteral("show_maximized_library")),
                  true,
                  false),
          m_showMixer(ConfigKey(kSkinGroup, QStringLiteral("show_mixer")),
                  true,
                  true),
          m_showSettings(ConfigKey(kSkinGroup, QStringLiteral("show_settings")),
                  false,
                  false),
          m_showSpinnies(ConfigKey(kSkinGroup, QStringLiteral("show_spinnies")),
                  true,
                  true),
          m_showVinylControl(
                  ConfigKey(kSkinGroup, QStringLiteral("show_vinylcontrol")),
                  true,
                  false),
          m_showWaveforms(
                  ConfigKey(kSkinGroup, QStringLiteral("show_waveforms")),
                  true,
                  true),
          m_showHotcues(ConfigKey(kSkinGroup, QStringLiteral("show_hotcues")),
                  true,
                  true),
          m_show8Hotcues(
                  ConfigKey(kSkinGroup, QStringLiteral("show_8_hotcues")),
                  true,
                  true),
          m_showIntroOutroCues(ConfigKey(kSkinGroup,
                                       QStringLiteral("show_intro_outro_cues")),
                  true,
                  true),
          m_showLoopControls(
                  ConfigKey(kSkinGroup, QStringLiteral("show_loop_controls")),
                  true,
                  true),
          m_showBeatjumpControls(
                  ConfigKey(
                          kSkinGroup, QStringLiteral("show_beatjump_controls")),
                  true,
                  true),
          m_showRateControls(
                  ConfigKey(kSkinGroup, QStringLiteral("show_rate_controls")),
                  true,
                  true),
          m_showRateControlButtons(
                  ConfigKey(kSkinGroup,
                          QStringLiteral("show_rate_control_buttons")),
                  true,
                  true),
          m_showKeyControls(
                  ConfigKey(kSkinGroup, QStringLiteral("show_key_controls")),
                  true,
                  true),
          m_showEqKnobs(ConfigKey(kSkinGroup, QStringLiteral("show_eq_knobs")),
                  true,
                  true),
          m_showEqKillButtons(
                  ConfigKey(kSkinGroup, QStringLiteral("show_eq_kill_buttons")),
                  true,
                  true),
          m_showXfader(ConfigKey(kSkinGroup, QStringLiteral("show_xfader")),
                  true,
                  true),
          m_showMainHeadMixer(
                  ConfigKey(kSkinGroup, QStringLiteral("show_main_head_mixer")),
                  true,
                  true),
          m_equal4deckWaveforms(
                  ConfigKey(
                          kSkinGroup, QStringLiteral("equal_4deck_waveforms")),
                  true,
                  false),
          m_timingShiftButtons(
                  ConfigKey(kSkinGroup, QStringLiteral("timing_shift_buttons")),
                  true,
                  false),
          m_showSuperKnobs(
                  ConfigKey(kSkinGroup, QStringLiteral("show_superknobs")),
                  true,
                  false),
          m_showSamplerFx(
                  ConfigKey(kSkinGroup, QStringLiteral("show_sampler_fx")),
                  true,
                  false) {
    m_showEffectRack.setButtonMode(mixxx::control::ButtonMode::Toggle);
    m_showLibraryCoverArt.setButtonMode(mixxx::control::ButtonMode::Toggle);
    m_showMicrophones.setButtonMode(mixxx::control::ButtonMode::Toggle);
    m_showPreviewDecks.setButtonMode(mixxx::control::ButtonMode::Toggle);
    m_showSamplers.setButtonMode(mixxx::control::ButtonMode::Toggle);
    m_show4EffectUnits.setButtonMode(mixxx::control::ButtonMode::Toggle);
    m_showCoverArt.setButtonMode(mixxx::control::ButtonMode::Toggle);
    m_showMaximizedLibrary.setButtonMode(mixxx::control::ButtonMode::Toggle);
    m_showMixer.setButtonMode(mixxx::control::ButtonMode::Toggle);
    m_showSettings.setButtonMode(mixxx::control::ButtonMode::Toggle);
    m_showSpinnies.setButtonMode(mixxx::control::ButtonMode::Toggle);
    m_showVinylControl.setButtonMode(mixxx::control::ButtonMode::Toggle);
    m_showWaveforms.setButtonMode(mixxx::control::ButtonMode::Toggle);
    m_showHotcues.setButtonMode(mixxx::control::ButtonMode::Toggle);
    m_show8Hotcues.setButtonMode(mixxx::control::ButtonMode::Toggle);
    m_showIntroOutroCues.setButtonMode(mixxx::control::ButtonMode::Toggle);
    m_showLoopControls.setButtonMode(mixxx::control::ButtonMode::Toggle);
    m_showBeatjumpControls.setButtonMode(mixxx::control::ButtonMode::Toggle);
    m_showRateControls.setButtonMode(mixxx::control::ButtonMode::Toggle);
    m_showRateControlButtons.setButtonMode(mixxx::control::ButtonMode::Toggle);
    m_showKeyControls.setButtonMode(mixxx::control::ButtonMode::Toggle);
    m_showEqKnobs.setButtonMode(mixxx::control::ButtonMode::Toggle);
    m_showEqKillButtons.setButtonMode(mixxx::control::ButtonMode::Toggle);
    m_showXfader.setButtonMode(mixxx::control::ButtonMode::Toggle);
    m_showMainHeadMixer.setButtonMode(mixxx::control::ButtonMode::Toggle);
    m_equal4deckWaveforms.setButtonMode(mixxx::control::ButtonMode::Toggle);
    m_timingShiftButtons.setButtonMode(mixxx::control::ButtonMode::Toggle);
    m_showSuperKnobs.setButtonMode(mixxx::control::ButtonMode::Toggle);
    m_showSamplerFx.setButtonMode(mixxx::control::ButtonMode::Toggle);

    m_showEffectRack.addAlias(ConfigKey(QStringLiteral("[EffectRack1]"), QStringLiteral("show")));
    m_showLibraryCoverArt.addAlias(ConfigKey(
            QStringLiteral("[Library]"), QStringLiteral("show_coverart")));
    m_showMicrophones.addAlias(ConfigKey(
            QStringLiteral("[Microphone]"), QStringLiteral("show_microphone")));
    m_showPreviewDecks.addAlias(ConfigKey(QStringLiteral("[PreviewDeck]"),
            QStringLiteral("show_previewdeck")));
    m_showSamplers.addAlias(ConfigKey(
            QStringLiteral("[Samplers]"), QStringLiteral("show_samplers")));
    m_showMaximizedLibrary.addAlias(ConfigKey(
            QStringLiteral("[Master]"), QStringLiteral("maximize_library")));
}
