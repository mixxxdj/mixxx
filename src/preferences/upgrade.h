#pragma once

#include "preferences/usersettings.h"

class Upgrade {
  public:
    Upgrade();
    ~Upgrade();

    UserSettingsPointer versionUpgrade(const QString& settingsPath);
    bool isFirstRun() { return m_bFirstRun; };
    bool rescanLibrary() {return m_bRescanLibrary; };

  private:
    bool askReanalyzeBeats();
    bool askReScanLibrary();
    bool m_bFirstRun;
    bool m_bRescanLibrary;
};
