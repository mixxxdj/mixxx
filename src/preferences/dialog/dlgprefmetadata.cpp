#include "preferences/dialog/dlgprefmetadata.h"

#include "broadcast/mpris/mprisservice.h"
#include "broadcast/scrobblingmanager.h"
#include "moc_dlgprefmetadata.cpp"

namespace {
#ifdef __MPRIS__
const QString kAppGroup = QStringLiteral("[App]");
const ConfigKey kEnabledMpris =
        ConfigKey(
                kAppGroup,
                QStringLiteral("enabled_mpris"));
const bool kEnabledMprisDefault = false;
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
    slotUpdate();
}

void DlgPrefMetadata::slotUpdate() {
#ifdef __MPRIS__
    enableDBusMPRISBox->setChecked(m_pSettings->getValue(kEnabledMpris, kEnabledMprisDefault));
#else
    enableDBusMPRISBox->hide();
#endif
}
