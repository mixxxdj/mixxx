#pragma once

#include "preferences/dialog/dlgpreferencepage.h"
#include "preferences/dialog/ui_dlgpreffingerprint.h"
#include "preferences/usersettings.h"

class TrackCollectionManager;

class DlgPrefFingerprint : public DlgPreferencePage, public Ui::DlgPrefFingerprintDlg {
    Q_OBJECT
  public:
    DlgPrefFingerprint(QWidget* parent, UserSettingsPointer pConfig);
    ~DlgPrefFingerprint() override = default;

  public slots:
    void slotApply() override;
    void slotUpdate() override;
    void slotResetToDefaults() override;

  private:
    UserSettingsPointer m_pConfig;
};
