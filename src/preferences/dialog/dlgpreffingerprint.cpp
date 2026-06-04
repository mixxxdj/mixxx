#include "preferences/dialog/dlgpreffingerprint.h"

#include "library/library_prefs.h"
#include "library/trackcollection.h"
#include "library/trackcollectionmanager.h"
#include "moc_dlgpreffingerprint.cpp"

using namespace mixxx::library::prefs;

DlgPrefFingerprint::DlgPrefFingerprint(QWidget* parent, UserSettingsPointer pConfig)
        : DlgPreferencePage(parent),
          m_pConfig(pConfig) {
    setupUi(this);
    slotUpdate();
    // No connections needed — the checkbox has no side effects on other
    // widgets. slotApply() reads its state directly.
}

void DlgPrefFingerprint::slotUpdate() {
    checkBoxFingerprintEnabled->setChecked(
            m_pConfig->getValue(
                    mixxx::library::prefs::kFingerprintAnalysisEnabledConfigKey, false));
}

void DlgPrefFingerprint::slotApply() {
    m_pConfig->set(
            kFingerprintAnalysisEnabledConfigKey,
            ConfigValue{checkBoxFingerprintEnabled->isChecked()});
}

void DlgPrefFingerprint::slotResetToDefaults() {
    checkBoxFingerprintEnabled->setChecked(false);
}
