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
          m_showPreparationWindow(ConfigKey(kSkinGroup, QStringLiteral("show_preparation_window")),
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
                  false) {
    m_showEffectRack.setButtonMode(mixxx::control::ButtonMode::Toggle);
    m_showLibraryCoverArt.setButtonMode(mixxx::control::ButtonMode::Toggle);
    m_showMicrophones.setButtonMode(mixxx::control::ButtonMode::Toggle);
    m_showPreviewDecks.setButtonMode(mixxx::control::ButtonMode::Toggle);
    m_showSamplers.setButtonMode(mixxx::control::ButtonMode::Toggle);
    m_show4EffectUnits.setButtonMode(mixxx::control::ButtonMode::Toggle);
    m_showCoverArt.setButtonMode(mixxx::control::ButtonMode::Toggle);
    m_showMaximizedLibrary.setButtonMode(mixxx::control::ButtonMode::Toggle);
    m_showPreparationWindow.setButtonMode(mixxx::control::ButtonMode::Toggle);
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
    m_showPreparationWindow.addAlias(ConfigKey(
            QStringLiteral("[Master]"), QStringLiteral("show_preparation_window")));
}
