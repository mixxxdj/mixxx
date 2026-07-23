#include "preferences/dialog/dlgpreffingerprint.h"

#include <QUrl>

#include "library/library_prefs.h"
#include "library/trackcollection.h"
#include "library/trackcollectionmanager.h"
#include "moc_dlgpreffingerprint.cpp"
#include "util/desktophelper.h"
#include "util/string.h"

using namespace mixxx::library::prefs;

namespace {

const QString kAcoustIdApiKeyUrl =
        QStringLiteral("https://acoustid.org/api-key");

} // anonymous namespace

DlgPrefFingerprint::DlgPrefFingerprint(
        QWidget* parent,
        UserSettingsPointer pConfig,
        TrackCollectionManager* pTrackCollectionManager)
        : DlgPreferencePage(parent),
          m_pConfig(pConfig),
          m_pTrackCollectionManager(pTrackCollectionManager) {
    setupUi(this);

    // Build the "Get your key ↗" link using the preference page's link colour
    // helper — same pattern as DlgPrefLibrary's manual and BPM links.
    createLinkColor();
    labelAcoustIdApiKeyLink->setText(coloredLinkString(
            m_pLinkColor,
            tr("Get your key ↗"),
            kAcoustIdApiKeyUrl));
    connect(labelAcoustIdApiKeyLink,
            &QLabel::linkActivated,
            [](const QString& url) {
                mixxx::DesktopHelper::openUrl(url);
            });

    // When the user toggles fingerprint analysis, update the group box state.
    connect(checkBoxFingerprintEnabled,
            &QCheckBox::toggled,
            this,
            &DlgPrefFingerprint::slotFingerprintEnabledToggled);

    // Update auto-submit checkbox state when API key text changes
    connect(lineEditAcoustIdApiKey,
            &QLineEdit::textChanged,
            this,
            [this]() {
                if (checkBoxFingerprintEnabled->isChecked()) {
                    const bool hasKey = !lineEditAcoustIdApiKey->text().trimmed().isEmpty();
                    checkBoxAcoustIdAutoSubmit->setEnabled(hasKey);
                    if (!hasKey) {
                        checkBoxAcoustIdAutoSubmit->setChecked(false);
                    }
                }
            });

    connect(btnClearAllFingerprints,
            &QPushButton::clicked,
            this,
            &DlgPrefFingerprint::slotClearAllFingerprints);

    slotUpdate();
    // No connections needed — the checkbox has no side effects on other
    // widgets. slotApply() reads its state directly.
}

void DlgPrefFingerprint::slotUpdate() {
    const bool fingerprintEnabled = m_pConfig->getValue(
            kFingerprintAnalysisEnabledConfigKey, false);
    checkBoxFingerprintEnabled->setChecked(fingerprintEnabled);

    const QString apiKey = m_pConfig->getValue(
            kAcoustIdUserApiKeyConfigKey, QString());
    lineEditAcoustIdApiKey->setText(apiKey);

    const bool autoSubmit = m_pConfig->getValue(
            kAcoustIdAutoSubmitConfigKey, false);
    checkBoxAcoustIdAutoSubmit->setChecked(autoSubmit);

    const double matchThreshold = m_pConfig->getValue(
            kCmrtMatchThresholdConfigKey, kCmrtMatchThresholdDefault);
    // Stored as a 0.0-1.0 fraction; the spin box shows it as a percentage.
    spinBoxCmrtMatchThreshold->setValue(matchThreshold * 100.0);

    setAcoustIdGroupEnabled(fingerprintEnabled);
}

void DlgPrefFingerprint::slotApply() {
    m_pConfig->set(
            kFingerprintAnalysisEnabledConfigKey,
            ConfigValue{checkBoxFingerprintEnabled->isChecked()});

    // Trim whitespace — users sometimes accidentally paste a trailing space
    // when copying the key from the browser.
    const QString apiKey = lineEditAcoustIdApiKey->text().trimmed();
    m_pConfig->set(
            kAcoustIdUserApiKeyConfigKey,
            ConfigValue{apiKey});

    m_pConfig->set(
            kAcoustIdAutoSubmitConfigKey,
            ConfigValue{checkBoxAcoustIdAutoSubmit->isChecked()});

    m_pConfig->set(
            kCmrtMatchThresholdConfigKey,
            ConfigValue{spinBoxCmrtMatchThreshold->value() / 100.0});
}

void DlgPrefFingerprint::slotResetToDefaults() {
    checkBoxFingerprintEnabled->setChecked(false);
    lineEditAcoustIdApiKey->clear();
    checkBoxAcoustIdAutoSubmit->setChecked(false);
    spinBoxCmrtMatchThreshold->setValue(kCmrtMatchThresholdDefault * 100.0);
    // Reflect the cleared state immediately in the UI.
    setAcoustIdGroupEnabled(false);
}

void DlgPrefFingerprint::slotFingerprintEnabledToggled(bool enabled) {
    setAcoustIdGroupEnabled(enabled);
}

void DlgPrefFingerprint::setAcoustIdGroupEnabled(bool fingerprintEnabled) {
    // The group box as a whole is only usable when fingerprinting is on —
    // without fingerprints there is nothing to submit.
    groupBoxAcoustId->setEnabled(fingerprintEnabled);

    // Auto-submit additionally requires a non-empty key. If the key field is
    // empty, disable the checkbox and uncheck it so the config cannot be left
    // in an inconsistent state (auto-submit=true but no key stored).
    if (fingerprintEnabled) {
        const bool hasKey = !lineEditAcoustIdApiKey->text().trimmed().isEmpty();
        checkBoxAcoustIdAutoSubmit->setEnabled(hasKey);
        if (!hasKey) {
            checkBoxAcoustIdAutoSubmit->setChecked(false);
        }
    }
}

void DlgPrefFingerprint::slotClearAllFingerprints() {
    // Confirmation dialog — same pattern as slotRemoveDir() in DlgPrefLibrary.
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setWindowTitle(tr("Clear All Fingerprints?"));
    msgBox.setText(tr(
            "This will delete all stored Chromaprint fingerprints, AcoustID queue "
            "entries, and CMRT group memberships for every track in your library."
            "Tracks will be re-fingerprinted the next time fingerprint analysis runs."
            "This cannot be undone."));
    QPushButton* clearButton = msgBox.addButton(
            tr("Clear All"), QMessageBox::DestructiveRole);
    msgBox.addButton(QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Cancel);
    msgBox.exec();

    if (msgBox.clickedButton() != clearButton) {
        return;
    }

    if (!m_pTrackCollectionManager) {
        qWarning() << "DlgPrefFingerprint::slotClearAllFingerprints: "
                      "no TrackCollectionManager available";
        return;
    }

    TrackFingerprintDao& dao =
            m_pTrackCollectionManager->internalCollection()->getTrackFingerprintDAO();
    const int cleared = dao.clearAllFingerprintData();

    QMessageBox::information(
            this,
            tr("Fingerprints Cleared"),
            tr("Cleared fingerprint data for %n track(s).", "", cleared));
}
