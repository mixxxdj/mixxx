#include "skin/skincontrols.h"

#include <QString>

namespace {
const QString kSkinGroup = QStringLiteral("[Skin]");
} // namespace

SkinControls::SkinControls()
        : m_showEffectRack(ConfigKey(kSkinGroup, QStringLiteral("show_effectrack")),
                  true,
                  true),
          m_showLibraryCoverArt(ConfigKey(kSkinGroup, QStringLiteral("show_library_coverart")),
                  true,
                  true),
          m_showMicrophones(ConfigKey(kSkinGroup, QStringLiteral("show_microphones")),
                  true,
                  true),
          m_showPreviewDecks(ConfigKey(kSkinGroup, QStringLiteral("show_preview_decks")),
                  true,
                  true),
          m_showSamplers(ConfigKey(kSkinGroup, QStringLiteral("show_samplers")),
                  true,
                  true),
          m_show4EffectUnits(ConfigKey(kSkinGroup, QStringLiteral("show_4effectunits")),
                  true,
                  false),
          m_showCoverArt(ConfigKey(kSkinGroup, QStringLiteral("show_coverart")),
                  true,
                  true),
          m_showMaximizedLibrary(ConfigKey(kSkinGroup, QStringLiteral("show_maximized_library")),
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
          m_showVinylControl(ConfigKey(kSkinGroup, QStringLiteral("show_vinylcontrol")),
                  true,
                  false),
          m_hightlightInputAllow(ConfigKey(kSkinGroup, QStringLiteral("highlight_input_allow")),
                  true,
                  false),
          m_hightlightChannel1(ConfigKey(kSkinGroup, QStringLiteral("highlight_[Channel1]")),
                  false,
                  false),
          m_hightlightChannel2(ConfigKey(kSkinGroup, QStringLiteral("highlight_[Channel2]")),
                  false,
                  false),
          m_hightlightChannel3(ConfigKey(kSkinGroup, QStringLiteral("highlight_[Channel3]")),
                  false,
                  false),
          m_hightlightChannel4(ConfigKey(kSkinGroup, QStringLiteral("highlight_[Channel4]")),
                  false,
                  false) {
    m_showEffectRack.setButtonMode(ControlPushButton::TOGGLE);
    m_showLibraryCoverArt.setButtonMode(ControlPushButton::TOGGLE);
    m_showMicrophones.setButtonMode(ControlPushButton::TOGGLE);
    m_showPreviewDecks.setButtonMode(ControlPushButton::TOGGLE);
    m_showSamplers.setButtonMode(ControlPushButton::TOGGLE);
    m_show4EffectUnits.setButtonMode(ControlPushButton::TOGGLE);
    m_showCoverArt.setButtonMode(ControlPushButton::TOGGLE);
    m_showMaximizedLibrary.setButtonMode(ControlPushButton::TOGGLE);
    m_showMixer.setButtonMode(ControlPushButton::TOGGLE);
    m_showSettings.setButtonMode(ControlPushButton::TOGGLE);
    m_showSpinnies.setButtonMode(ControlPushButton::TOGGLE);
    m_showVinylControl.setButtonMode(ControlPushButton::TOGGLE);
    m_hightlightInputAllow.setButtonMode(ControlPushButton::TOGGLE);
    m_hightlightChannel1.setButtonMode(ControlPushButton::TOGGLE);
    m_hightlightChannel2.setButtonMode(ControlPushButton::TOGGLE);
    m_hightlightChannel3.setButtonMode(ControlPushButton::TOGGLE);
    m_hightlightChannel4.setButtonMode(ControlPushButton::TOGGLE);

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
