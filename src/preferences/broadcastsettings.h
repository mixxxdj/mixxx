#ifndef PREFERENCES_BROADCASTSETTINGS_H
#define PREFERENCES_BROADCASTSETTINGS_H

#include <QList>
#include <QSharedPointer>
#include <QString>

#include "preferences/usersettings.h"
#include "track/track.h"
#include "broadcastprofile.h"

class BroadcastSettings {
  public:
    BroadcastSettings(UserSettingsPointer pConfig);

    void setCurrentProfile(BroadcastProfilePtr profile);
    BroadcastProfilePtr getCurrentProfile();
    BroadcastProfilePtr getProfileByName(const QString& profileName);
    void saveProfile(BroadcastProfilePtr profile);
    void saveAll();
    void deleteProfile(BroadcastProfilePtr profile);

  private:
    void loadProfiles();
    QString filePathForProfile(BroadcastProfilePtr profile);
    QString filePathForProfile(const QString& profileName);
    QString getProfilesFolder();
    void loadLegacySettings(BroadcastProfilePtr profile);

    // Pointer to config object
    UserSettingsPointer m_pConfig;
    QList<BroadcastProfilePtr> m_profiles;
    QString m_currentProfile;
};

typedef QSharedPointer<BroadcastSettings> BroadcastSettingsPointer;

#endif /* PREFERENCES_BROADCASTSETTINGS_H */
