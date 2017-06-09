#ifndef PREFERENCES_BROADCASTSETTINGS_H
#define PREFERENCES_BROADCASTSETTINGS_H

#include <QString>
#include <QList>

#include "preferences/usersettings.h"
#include "track/track.h"
#include "broadcastprofile.h"

class BroadcastSettings {
  public:
    BroadcastSettings(UserSettingsPointer pConfig);
    ~BroadcastSettings();

    void setCurrentProfile(BroadcastProfile* profile);
    BroadcastProfile* getCurrentProfile();
    BroadcastProfile* getProfileByName(const QString& profileName);
    void deleteProfile(BroadcastProfile* profile);
    void saveAll();

  private:
    void loadProfiles();
    void saveProfile(BroadcastProfile* profile);
    QString filenameForProfile(BroadcastProfile* profile);

    // Pointer to config object
    UserSettingsPointer m_pConfig;
    QList<BroadcastProfile*> m_profiles;
    QString m_currentProfile;
};

#endif /* PREFERENCES_BROADCASTSETTINGS_H */
