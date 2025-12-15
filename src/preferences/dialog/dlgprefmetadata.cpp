#include "preferences/dialog/dlgprefmetadata.h"

#include "moc_dlgprefmetadata.cpp"

DlgPrefMetadata::DlgPrefMetadata(QWidget* pParent, const UserSettingsPointer& pSettings)
        : DlgPreferencePage(pParent),
          m_pSettings(pSettings) {
    setupUi(this);
}

void DlgPrefMetadata::slotApply() {
}

void DlgPrefMetadata::slotCancel() {
}

void DlgPrefMetadata::slotResetToDefaults() {
}

void DlgPrefMetadata::slotUpdate() {
}
