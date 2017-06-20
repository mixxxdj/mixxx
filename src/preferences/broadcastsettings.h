#ifndef PREFERENCES_BROADCASTSETTINGS_H
#define PREFERENCES_BROADCASTSETTINGS_H

#include <vector>
#include <QSharedPointer>
#include <QString>

#include "preferences/usersettings.h"
#include "track/track.h"
#include "broadcastprofile.h"

class BroadcastSettings {
  public:
    BroadcastSettings(UserSettingsPointer pConfig);

    void setCurrentProfile(const BroadcastProfilePtr& profile);
    const BroadcastProfilePtr& getCurrentProfile();
    const BroadcastProfilePtr& getProfileByName(const QString& profileName);
    void saveProfile(const BroadcastProfilePtr& profile);
    void saveAll();
    void deleteProfile(const BroadcastProfilePtr& profile);

  private:
    void loadProfiles();
    QString filePathForProfile(const BroadcastProfilePtr& profile);
    QString filePathForProfile(const QString& profileName);
    QString getProfilesFolder();
    void loadLegacySettings(const BroadcastProfilePtr& profile);

    // Pointer to config object
    UserSettingsPointer m_pConfig;
    std::vector<BroadcastProfilePtr> m_profiles;
    QString m_currentProfile;
};

typedef QSharedPointer<BroadcastSettings> BroadcastSettingsPointer;

#endif /* PREFERENCES_BROADCASTSETTINGS_H */
