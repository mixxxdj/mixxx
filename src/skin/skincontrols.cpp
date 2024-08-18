#include "skin/skincontrols.h"

#include <QString>

namespace {
const QString kSkinGroup = QStringLiteral("[Skin]");
} // namespace

SkinControls::SkinControls()
        : m_showEffectRack(ConfigKey(kSkinGroup, QStringLiteral("show_effectrack")),
                  true,
                  ControlConfigFlag::Persist),
          m_showLibraryCoverArt(ConfigKey(kSkinGroup, QStringLiteral("show_library_coverart")),
                  true,
                  ControlConfigFlag::Persist),
          m_showMicrophones(ConfigKey(kSkinGroup, QStringLiteral("show_microphones")),
                  true,
                  ControlConfigFlag::Persist),
          m_showPreviewDecks(ConfigKey(kSkinGroup, QStringLiteral("show_preview_decks")),
                  true,
                  ControlConfigFlag::Persist),
          m_showSamplers(ConfigKey(kSkinGroup, QStringLiteral("show_samplers")),
                  true,
                  ControlConfigFlag::Persist),
          m_show4EffectUnits(ConfigKey(kSkinGroup, QStringLiteral("show_4effectunits")),
                  false,
                  ControlConfigFlag::Persist),
          m_showCoverArt(ConfigKey(kSkinGroup, QStringLiteral("show_coverart")),
                  true,
                  ControlConfigFlag::Persist),
          m_showMaximizedLibrary(ConfigKey(kSkinGroup, QStringLiteral("show_maximized_library")),
                  false,
                  ControlConfigFlag::Persist),
          m_showMixer(ConfigKey(kSkinGroup, QStringLiteral("show_mixer")),
                  true,
                  ControlConfigFlag::Persist),
          m_showSettings(ConfigKey(kSkinGroup, QStringLiteral("show_settings")),
                  false,
                  ControlConfigFlag::None),
          m_showSpinnies(ConfigKey(kSkinGroup, QStringLiteral("show_spinnies")),
                  true,
                  ControlConfigFlag::Persist),
          m_showVinylControl(ConfigKey(kSkinGroup, QStringLiteral("show_vinylcontrol")),
                  false,
                  ControlConfigFlag::Persist) {
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
