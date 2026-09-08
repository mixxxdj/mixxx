#pragma once

#include "preferences/dialog/dlgpreferencepage.h"
#include "preferences/dialog/ui_dlgpreffingerprint.h"
#include "preferences/usersettings.h"

class TrackCollectionManager;

class DlgPrefFingerprint : public DlgPreferencePage, public Ui::DlgPrefFingerprintDlg {
    Q_OBJECT
  public:
    DlgPrefFingerprint(QWidget* parent,
            UserSettingsPointer pConfig,
            TrackCollectionManager* pTrackCollectionManager);
    ~DlgPrefFingerprint() override = default;

  public slots:
    void slotApply() override;
    void slotUpdate() override;
    void slotResetToDefaults() override;

  private slots:
    // Called when the fingerprint-enabled checkbox is toggled.
    // Enables or disables the AcoustID submission group box to match.
    void slotFingerprintEnabledToggled(bool enabled);
    void slotClearAllFingerprints();

  private:
    // Applies the enabled/disabled state to the AcoustID group and, within it,
    // the auto-submit checkbox (which additionally requires a non-empty key).
    void setAcoustIdGroupEnabled(bool fingerprintEnabled);

    UserSettingsPointer m_pConfig;
    TrackCollectionManager* m_pTrackCollectionManager;
};
