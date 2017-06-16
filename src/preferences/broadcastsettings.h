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
    ~BroadcastSettings();

    void setCurrentProfile(BroadcastProfile* profile);
    BroadcastProfile* getCurrentProfile();
    BroadcastProfile* getProfileByName(const QString& profileName);
    void saveProfile(BroadcastProfile* profile);
    void saveAll();
    void deleteProfile(BroadcastProfile* profile);

  private:
    void loadProfiles();
    QString filenameForProfile(BroadcastProfile* profile);
    QString filenameForProfile(const QString& profileName);
    QString getProfilesFolder();
    void loadLegacySettings(BroadcastProfile* profile);

    // Pointer to config object
    UserSettingsPointer m_pConfig;
    QList<BroadcastProfile*> m_profiles;
    QString m_currentProfile;
};

typedef QSharedPointer<BroadcastSettings> BroadcastSettingsPointer;

#endif /* PREFERENCES_BROADCASTSETTINGS_H */
