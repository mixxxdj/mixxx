#pragma once

#include "preferences/usersettings.h"

class Upgrade {
  public:
    Upgrade();
    ~Upgrade();

    UserSettingsPointer versionUpgrade(const QString& settingsPath);
    bool isFirstRun() { return m_bFirstRun; };
    bool rescanLibrary() {
        return m_bRescanLibrary;
    };
    bool upgradedFrom21OrEarlier() {
        return m_upgradedFrom21OrEarlier;
    };

  private:
    bool askReanalyzeBeats();
    bool askReScanLibrary();
    bool m_bFirstRun;
    bool m_bRescanLibrary;
    bool m_upgradedFrom21OrEarlier;
};
