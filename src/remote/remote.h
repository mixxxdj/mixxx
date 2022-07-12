// created jan.koester
#pragma once

#include <QtCore>
#include <preferences/usersettings.h>
#include "util/logger.h"

namespace mixxx {
   class RemoteControl{
   public:
       RemoteControl(UserSettingsPointer pConfig, QObject* pParent = nullptr);
       ~RemoteControl();
   private:
        UserSettingsPointer m_pSettings;
   };
}; 
