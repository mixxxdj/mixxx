// created jan.koester
#pragma once

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
                     std::shared_ptr<PlayerManager> &ainf
                    );
       virtual ~RemoteControl();

       // Tears down the running webserver (if any) and, if
       // [RemoteControl],actv is currently true, starts a new one with the
       // current settings. Call after the RemoteControl preferences are saved.
       void reload();
   private:
        UserSettingsPointer                   m_pConfig;
        std::shared_ptr<TrackCollectionManager> m_trackscollmngr;
        std::shared_ptr<Library>              m_library;
        std::shared_ptr<DbConnectionPool>     m_database;
        std::shared_ptr<PlayerManager>        m_ainf;
        std::shared_ptr<RemoteController>     m_RemoteController;
   };
};
