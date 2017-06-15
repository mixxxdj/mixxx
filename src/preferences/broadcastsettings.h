#ifndef PREFERENCES_BROADCASTSETTINGS_H
#define PREFERENCES_BROADCASTSETTINGS_H

#include <QAbstractListModel>
#include <QString>
#include <QList>
#include <QObject>
#include <QVariant>

#include "preferences/usersettings.h"
#include "track/track.h"
#include "broadcastprofile.h"

class BroadcastSettings : public QAbstractListModel {
  Q_OBJECT

  public:
    BroadcastSettings(UserSettingsPointer pConfig, QObject *parent = nullptr);
    ~BroadcastSettings();

    BroadcastProfile* newProfile();
    void setCurrentProfile(BroadcastProfile* profile);
    BroadcastProfile* getCurrentProfile();
    BroadcastProfile* getProfileByName(const QString& profileName);
    void renameProfile(BroadcastProfile* profile, const QString& newName);
    void deleteProfile(BroadcastProfile* profile);
    void saveProfile(BroadcastProfile* profile);
    void saveAll();

    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    //QVariant headerData(int section, Qt::Orientation orientation,
    //                    int role = Qt::DisplayRole);

  private:
    void loadProfiles();
    QString filenameForProfile(BroadcastProfile* profile);
    QString filenameForProfile(const QString& profileName);
    QString getProfilesFolder();

    bool addProfile(BroadcastProfile* profile);

    // Pointer to config object
    UserSettingsPointer m_pConfig;
    QList<BroadcastProfile*> m_profiles;
    QString m_currentProfile;
};

#endif /* PREFERENCES_BROADCASTSETTINGS_H */
