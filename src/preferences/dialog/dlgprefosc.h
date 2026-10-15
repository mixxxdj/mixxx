#pragma once

#include "preferences/dialog/dlgpreferencepage.h"
#include "preferences/dialog/ui_dlgprefosc.h"
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
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
    void slotToggleOsc(Qt::CheckState state);
#else
    void slotToggleOsc(int buttonState);
#endif

  private:
    UserSettingsPointer m_pConfig;
};
