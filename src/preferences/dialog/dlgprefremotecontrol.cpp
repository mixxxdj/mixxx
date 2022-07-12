#include <remote/remote.h>

#include "preferences/dialog/dlgprefremotecontrol.h"

DlgPrefRemoteControl::DlgPrefRemoteControl(QWidget *pParent,UserSettingsPointer  pSettings) : DlgPreferencePage(pParent){
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
          
}

void DlgPrefRemoteControl::slotResetToDefaults(){
          
}
