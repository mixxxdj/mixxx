#include "preferences/dialog/dlgprefmetadata.h"

#ifdef __MPRIS__
#include "broadcast/mpris/mprisservice.h"
#endif
#ifdef __MACOS_MEDIAPLAYER__
#include "broadcast/macos/macosmediaplayerservice.h"
#endif
#include "broadcast/scrobblingmanager.h"
#include "moc_dlgprefmetadata.cpp"

namespace {
#if defined(__MPRIS__) || defined(__MACOS_MEDIAPLAYER__)
const QString kAppGroup = QStringLiteral("[App]");
#endif
#ifdef __MPRIS__
const ConfigKey kEnabledMpris =
        ConfigKey(
                kAppGroup,
                QStringLiteral("enabled_mpris"));
const bool kEnabledMprisDefault = false;
#endif
#ifdef __MACOS_MEDIAPLAYER__
const ConfigKey kEnabledMacOSMediaPlayer =
        ConfigKey(
                kAppGroup,
                QStringLiteral("enabled_macos_mediaplayer"));
const bool kEnabledkEnabledMacOSMediaPlayerDefault = false;
#endif
} // namespace

DlgPrefMetadata::DlgPrefMetadata(QWidget* pParent,
        const UserSettingsPointer& pSettings,
        std::shared_ptr<ScrobblingManager> pScrobblingManager)
        : DlgPreferencePage(pParent),
          m_pSettings(pSettings),
          m_pScrobblingManager(pScrobblingManager) {
    setupUi(this);
}

void DlgPrefMetadata::slotApply() {
#ifdef __MPRIS__
    m_pSettings->setValue(kEnabledMpris, enableDBusMPRISBox->isChecked());

    if (m_pScrobblingManager->isScrobblingServiceActivated<MprisService>() ==
            enableDBusMPRISBox->isChecked()) {
        return;
    }
    if (!enableDBusMPRISBox->isChecked()) {
        m_pScrobblingManager->removeScrobblingService<MprisService>();
    } else {
        m_pScrobblingManager->addScrobblingService<MprisService>();
    }
#endif
#ifdef __MACOS_MEDIAPLAYER__
    m_pSettings->setValue(kEnabledMacOSMediaPlayer, enableMacOSMediaPlayerBox->isChecked());

    if (m_pScrobblingManager->isScrobblingServiceActivated<MacOSMediaPlayerService>() ==
            enableMacOSMediaPlayerBox->isChecked()) {
        return;
    }
    if (!enableMacOSMediaPlayerBox->isChecked()) {
        m_pScrobblingManager->removeScrobblingService<MacOSMediaPlayerService>();
    } else {
        m_pScrobblingManager->addScrobblingService<MacOSMediaPlayerService>();
    }
#endif
}

void DlgPrefMetadata::slotCancel() {
#ifdef __MPRIS__
    enableDBusMPRISBox->setChecked(m_pSettings->getValue(kEnabledMpris, kEnabledMprisDefault));
#endif
}

void DlgPrefMetadata::slotResetToDefaults() {
#ifdef __MPRIS__
    m_pSettings->setValue(kEnabledMpris, kEnabledMprisDefault);
#endif
#ifdef __MACOS_MEDIAPLAYER__
    m_pSettings->setValue(kEnabledMacOSMediaPlayer, kEnabledkEnabledMacOSMediaPlayerDefault);
#endif
    slotUpdate();
}

void DlgPrefMetadata::slotUpdate() {
#ifdef __MPRIS__
    enableDBusMPRISBox->setChecked(m_pSettings->getValue(kEnabledMpris, kEnabledMprisDefault));
#else
    MPRISBox->hide();
#endif
#ifdef __MACOS_MEDIAPLAYER__
    enableMacOSMediaPlayerBox->setChecked(m_pSettings->getValue(
            kEnabledMacOSMediaPlayer, kEnabledkEnabledMacOSMediaPlayerDefault));
#else
    MacOSMediaPlayer->hide();
#endif
}
