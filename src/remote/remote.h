// created jan.koester
#pragma once

#include <QtCore>
#include <preferences/usersettings.h>

#include "library/trackcollectionmanager.h"
#include "util/logger.h"

class Library;

namespace mixxx {
   class RemoteController;
   class RemoteControl{
   public:
       RemoteControl(UserSettingsPointer pConfig,
                     std::shared_ptr<TrackCollectionManager> &trackscollmngr,
                     std::shared_ptr<DbConnectionPool> &database,
                     QObject* pParent = nullptr);
       virtual ~RemoteControl();
   private:
        std::shared_ptr<RemoteController>     m_RemoteController;
        QObject*                                              m_Parent;
   };
}; 
