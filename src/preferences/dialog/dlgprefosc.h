#pragma once

#include "preferences/dialog/dlgpreferencepage.h"
#include "preferences/dialog/ui_dlgprefoscdlg.h"
#include "preferences/usersettings.h"

class QWidget;

class DlgPrefOsc : public DlgPreferencePage, public Ui::DlgPrefOscDlg {
    Q_OBJECT
  public:
    DlgPrefOsc(QWidget* pParent, UserSettingsPointer pConfig);

  public slots:
    void slotUpdate() override;
    void slotApply() override;
    void slotResetToDefaults() override;

  private slots:

  private:
    UserSettingsPointer m_pConfig;
};
