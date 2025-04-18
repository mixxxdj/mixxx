// created jan.koester
#pragma once

#include <QtCore>
#include <preferences/usersettings.h>

#include "library/trackcollectionmanager.h"
#include "library/dao/playlistdao.h"
#include "util/logger.h"
#include "mixer/playermanager.h"

class Library;

namespace mixxx {
   class RemoteController;
   class RemoteControl{
   public:
       RemoteControl(UserSettingsPointer pConfig,
                     std::shared_ptr<TrackCollectionManager> &trackscollmngr,
                     std::shared_ptr<Library> &library,
                     std::shared_ptr<DbConnectionPool> &database,
                     std::shared_ptr<PlayerManager> &ainf);
       virtual ~RemoteControl();
   private:
        std::shared_ptr<RemoteController>     m_RemoteController;
        QObject*                                              m_Parent;
   };
}; 
