#ifndef PREFERENCES_BROADCASTSETTINGS_H
#define PREFERENCES_BROADCASTSETTINGS_H

#include <QAbstractListModel>
#include <QMap>
#include <QSharedPointer>
#include <QString>
#include <QVariant>

#include "preferences/usersettings.h"
#include "track/track.h"
#include "broadcastprofile.h"

class BroadcastSettings : public QAbstractListModel {
  Q_OBJECT

  public:
    BroadcastSettings(UserSettingsPointer pConfig, QObject* parent = nullptr);

    BroadcastProfilePtr getProfileByName(const QString& profileName);
    void saveProfile(const BroadcastProfilePtr& profile);
    void saveAll();
    void deleteProfile(const BroadcastProfilePtr& profile);

    int rowCount(const QModelIndex& parent) const;
    QVariant data(const QModelIndex& index, int role) const;
    BroadcastProfilePtr profileAt(int index);
  private slots:
    void onProfileNameChanged(QString oldName, QString newName);

  private:
    void loadProfiles();
    QString filePathForProfile(const BroadcastProfilePtr& profile);
    QString filePathForProfile(const QString& profileName);
    QString getProfilesFolder();
    void loadLegacySettings(const BroadcastProfilePtr& profile);
    void addProfile(const BroadcastProfilePtr& profile);

    // Pointer to config object
    UserSettingsPointer m_pConfig;
    QMap<QString, BroadcastProfilePtr> m_profiles;
};

typedef QSharedPointer<BroadcastSettings> BroadcastSettingsPointer;

#endif /* PREFERENCES_BROADCASTSETTINGS_H */
