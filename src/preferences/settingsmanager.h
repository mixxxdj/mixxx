#pragma once

#ifdef __BROADCAST__
#include "preferences/broadcastsettings.h"
#endif
#include "preferences/usersettings.h"

class SettingsManager {
  public:
    explicit SettingsManager(const QString& settingsPath);
    virtual ~SettingsManager();

    UserSettingsPointer settings() const {
        return m_pSettings;
    }

#ifdef __BROADCAST__
    BroadcastSettingsPointer broadcastSettings() const {
        return m_pBroadcastSettings;
    }
#endif

    void save() {
        m_pSettings->save();
    }

    bool shouldRescanLibrary() {
        return m_bShouldRescanLibrary;
    }

    bool upgradedFrom21OrEarlier() {
        return m_upgradedFrom21OrEarlier;
    };

  private:
    UserSettingsPointer m_pSettings;
    bool m_bShouldRescanLibrary;
    bool m_upgradedFrom21OrEarlier;
#ifdef __BROADCAST__
    BroadcastSettingsPointer m_pBroadcastSettings;
#endif
};
