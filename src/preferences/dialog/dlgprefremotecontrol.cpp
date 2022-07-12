#include <remote/remote.h>

#include "preferences/dialog/dlgprefremotecontrol.h"

DlgPrefRemoteControl::DlgPrefRemoteControl(QWidget *pParent,UserSettingsPointer  pSettings) : DlgPreferencePage(pParent){
    m_pSettings=pSettings;
    setupUi(this);
    if(QVariant(m_pSettings->get(ConfigKey("[RemoteControl]","addr")).value).toBool()){
        this->remoteactv->setChecked(true);
    }
    this->remoteaddr->setText(m_pSettings->get(ConfigKey("[RemoteControl]","addr")).value);
    this->remoteport->setText(m_pSettings->get(ConfigKey("[RemoteControl]","port")).value);
    this->remotepass->setText(m_pSettings->get(ConfigKey("[RemoteControl]","pass")).value);
};

DlgPrefRemoteControl::~DlgPrefRemoteControl(){
};

QUrl DlgPrefRemoteControl::helpUrl() const {
    return QUrl();
}

void DlgPrefRemoteControl::slotUpdate(){
          
}
      
void DlgPrefRemoteControl::slotApply(){
    if(this->remoteactv->isChecked()){
        m_pSettings->set(ConfigKey("[RemoteControl]","addr"),ConfigValue(true));
    }else{
        m_pSettings->set(ConfigKey("[RemoteControl]","addr"),ConfigValue(false));
    }
    
    m_pSettings->set(ConfigKey("[RemoteControl]","addr"),ConfigValue(this->remoteaddr->text()));
    m_pSettings->set(ConfigKey("[RemoteControl]","port"),ConfigValue(this->remoteport->text()));
    m_pSettings->set(ConfigKey("[RemoteControl]","pass"),ConfigValue(this->remotepass->text()));
    m_pSettings->save();
}

void DlgPrefRemoteControl::slotResetToDefaults(){
    this->remoteactv->setChecked(false);
    this->remoteaddr->setText("0.0.0.0");
    this->remoteport->setText("8443");
    this->remotepass->setText("mixxx");      
}
