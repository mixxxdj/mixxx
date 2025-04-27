#pragma once

#include <memory>

#include <preferences/usersettings.h>

#include "preferences/dialog/dlgpreferencepage.h"
#include "preferences/dialog/ui_dlgprefremotecontrol.h"

class QWidget;
class QUrl;
class RemoteControl;

class DlgPrefRemoteControl: public DlgPreferencePage,
                         public Ui::DlgPrefRemoteControl {
    Q_OBJECT;
  public:
    DlgPrefRemoteControl(QWidget *pParent,UserSettingsPointer pSettings);
    virtual ~DlgPrefRemoteControl();
    
    virtual QUrl helpUrl() const override;
    
  public slots:
      void slotUpdate() override;
      void slotApply() override;
      void slotResetToDefaults() override;
  private:
      UserSettingsPointer m_pSettings; 
};
