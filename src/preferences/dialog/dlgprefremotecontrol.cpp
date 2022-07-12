#include <remote/remote.h>

#include "preferences/dialog/dlgprefremotecontrol.h"

DlgPrefRemoteControl::DlgPrefRemoteControl(QWidget *pParent,UserSettingsPointer  pSettings) : DlgPreferencePage(pParent){
    m_pSettings=pSettings;
    setupUi(this);
};

DlgPrefRemoteControl::~DlgPrefRemoteControl(){
};

QUrl DlgPrefRemoteControl::helpUrl() const {
    return QUrl();
}

void DlgPrefRemoteControl::slotUpdate(){
          
}
      
void DlgPrefRemoteControl::slotApply(){
    m_pSettings->setValue(ConfigKey("RemoteControl","addr"),ConfigValue(remoteaddr->text()));
    m_pSettings->setValue(ConfigKey("RemoteControl","port"),ConfigValue(remoteport->text()));
    m_pSettings->setValue(ConfigKey("RemoteControl","pass"),ConfigValue(remotepass->text()));
    m_pSettings->save();
}

void DlgPrefRemoteControl::slotResetToDefaults(){
          
}
