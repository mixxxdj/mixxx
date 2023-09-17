#include "uicontrols.h"

#include <QString>

namespace {
const QString kSkinGroup = QStringLiteral("[Skin]");
}

UIControls::UIControls()
        : m_effectRack1Show(ConfigKey(QStringLiteral("[EffectRack1]"),
                                    QStringLiteral("show")),
                  true,
                  true),
          m_libraryShowCoverArt(ConfigKey(QStringLiteral("[Library]"),
                                        QStringLiteral("show_coverart")),
                  true,
                  true),
          m_microphoneShowMicrophone(ConfigKey(QStringLiteral("[Microphone]"),
                                             QStringLiteral("show_microphone")),
                  true,
                  true),
          m_previewDeckShowPreviewDeck(
                  ConfigKey(QStringLiteral("[PreviewDeck]"),
                          QStringLiteral("show_previewdeck")),
                  true,
                  true),
          m_samplersShowSamplers(ConfigKey(QStringLiteral("[Samplers]"),
                                         QStringLiteral("show_samplers")),
                  true,
                  true),
          m_skinShow4EffectUnits(
                  ConfigKey(kSkinGroup, QStringLiteral("show_4effectunits")),
                  true,
                  false),
          m_skinShowCoverArt(
                  ConfigKey(kSkinGroup, QStringLiteral("show_coverart")),
                  true,
                  true),
          m_skinShowMaximizedLibrary(
                  ConfigKey(
                          kSkinGroup, QStringLiteral("show_maximized_library")),
                  true,
                  false),
          m_skinShowMixer(ConfigKey(kSkinGroup, QStringLiteral("show_mixer")),
                  true,
                  true),
          m_skinShowSettings(
                  ConfigKey(kSkinGroup, QStringLiteral("show_settings")),
                  false,
                  false),
          m_skinShowSpinnies(
                  ConfigKey(kSkinGroup, QStringLiteral("show_spinnies")),
                  true,
                  true),
          m_vinylControlShowVinylControl(
                  ConfigKey(QStringLiteral("[VinylControl]"),
                          QStringLiteral("show_vinylcontrol")),
                  true,
                  false) {
    m_effectRack1Show.setButtonMode(ControlPushButton::TOGGLE);
    m_libraryShowCoverArt.setButtonMode(ControlPushButton::TOGGLE);
    m_microphoneShowMicrophone.setButtonMode(ControlPushButton::TOGGLE);
    m_previewDeckShowPreviewDeck.setButtonMode(ControlPushButton::TOGGLE);
    m_samplersShowSamplers.setButtonMode(ControlPushButton::TOGGLE);
    m_skinShow4EffectUnits.setButtonMode(ControlPushButton::TOGGLE);
    m_skinShowCoverArt.setButtonMode(ControlPushButton::TOGGLE);
    m_skinShowMaximizedLibrary.setButtonMode(ControlPushButton::TOGGLE);
    m_skinShowMixer.setButtonMode(ControlPushButton::TOGGLE);
    m_skinShowSettings.setButtonMode(ControlPushButton::TOGGLE);
    m_skinShowSpinnies.setButtonMode(ControlPushButton::TOGGLE);
    m_vinylControlShowVinylControl.setButtonMode(ControlPushButton::TOGGLE);
    m_skinShowMaximizedLibrary.addAlias(ConfigKey(
            QStringLiteral("[Master]"), QStringLiteral("maximize_library")));
}
