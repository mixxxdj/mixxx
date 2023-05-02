// created jan.koester
#pragma once

#include <QtCore>
#include <preferences/usersettings.h>
#include "util/logger.h"

class Library;

namespace stefanfrings {
    class HttpListener;
};

namespace mixxx {
   class RemoteController;
   class RemoteControl{
   public:
       RemoteControl(UserSettingsPointer pConfig,::Library *mlib, QObject* pParent = nullptr);
       virtual ~RemoteControl();
   private:
        UserSettingsPointer m_pSettings;
        std::shared_ptr<QSettings>                            m_HttpSettings;
        std::shared_ptr<QSettings>                            m_FileSettings;
        std::shared_ptr<QSettings>                            m_SessionSettings;
        std::shared_ptr<RemoteController>                     m_RemoteController;
        std::shared_ptr<::stefanfrings::HttpListener>         m_HttpListener;
   };
}; 
