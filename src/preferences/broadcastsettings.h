#ifndef PREFERENCES_BROADCASTSETTINGS_H
#define PREFERENCES_BROADCASTSETTINGS_H

#include <QMap>
#include <QSharedPointer>
#include <QString>

#include "preferences/usersettings.h"
#include "preferences/broadcastsettingsmodel.h"
#include "preferences/broadcastprofile.h"

class BroadcastSettings : public QObject {
  Q_OBJECT

  public:
    BroadcastSettings(UserSettingsPointer pConfig, QObject* parent = nullptr);

    bool saveProfile(BroadcastProfilePtr profile);
    void saveAll();
    QList<BroadcastProfilePtr> profiles();
    BroadcastProfilePtr profileAt(int index);

    void applyModel(BroadcastSettingsModel* pModel);

  signals:
    void profileAdded(BroadcastProfilePtr profile);
    void profileRemoved(BroadcastProfilePtr profile);
    void profileRenamed(QString oldName, BroadcastProfilePtr profile);
    void profilesChanged();

  private slots:
    void onProfileNameChanged(QString oldName, QString newName);
    void onConnectionStatusChanged(int newStatus);

  private:
    void loadProfiles();
    bool addProfile(BroadcastProfilePtr profile);

    QString filePathForProfile(BroadcastProfilePtr profile);
    QString filePathForProfile(const QString& profileName);
    bool deleteFileForProfile(BroadcastProfilePtr profile);
    QString getProfilesFolder();

    void loadLegacySettings(BroadcastProfilePtr profile);

    // Pointer to config object
    UserSettingsPointer m_pConfig;
    QMap<QString, BroadcastProfilePtr> m_profiles;
};

typedef QSharedPointer<BroadcastSettings> BroadcastSettingsPointer;

#endif /* PREFERENCES_BROADCASTSETTINGS_H */
