#ifndef PREFERENCES_BROADCASTSETTINGS_H
#define PREFERENCES_BROADCASTSETTINGS_H

#include <QAbstractTableModel>
#include <QAbstractItemDelegate>
#include <QMap>
#include <QSharedPointer>
#include <QString>
#include <QVariant>

#include "preferences/usersettings.h"
#include "track/track.h"
#include "broadcastprofile.h"

class BroadcastSettings : public QAbstractTableModel {
  Q_OBJECT

  public:
    BroadcastSettings(UserSettingsPointer pConfig, QObject* parent = nullptr);

    BroadcastProfilePtr getProfileByName(const QString& profileName);
    bool saveProfile(const BroadcastProfilePtr& profile);
    void saveAll();
    BroadcastProfilePtr createProfile(const QString& profileName);
    bool addProfile(const BroadcastProfilePtr& profile);
    void deleteProfile(const BroadcastProfilePtr& profile);

    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    int columnCount(const QModelIndex& parent = QModelIndex()) const;
    QVariant headerData(int section, Qt::Orientation orientation,
            int role = Qt::DisplayRole) const;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex& index) const;
    bool setData(const QModelIndex& index, const QVariant& value,
            int role = Qt::EditRole);
    QAbstractItemDelegate* delegateForColumn(const int i, QObject* parent);

    BroadcastProfilePtr profileAt(int index);
    QList<BroadcastProfilePtr> profiles();

  signals:
    void profileAdded(BroadcastProfilePtr profile);
    void profileRemoved(BroadcastProfilePtr profile);
    void profileRenamed(QString oldName, BroadcastProfilePtr profile);
    void profilesChanged();

  private slots:
    void onProfileNameChanged(QString oldName, QString newName);
    void onConnectionStatusChanged(int newStatus);

  private:
    static QString connectionStatusString(BroadcastProfilePtr profile);

    void loadProfiles();
    QString filePathForProfile(const BroadcastProfilePtr& profile);
    QString filePathForProfile(const QString& profileName);
    bool deleteFileForProfile(const BroadcastProfilePtr& profile);
    bool deleteFileForProfile(const QString& profileName);
    QString getProfilesFolder();
    void loadLegacySettings(const BroadcastProfilePtr& profile);

    // Pointer to config object
    UserSettingsPointer m_pConfig;
    QMap<QString, BroadcastProfilePtr> m_profiles;
};

typedef QSharedPointer<BroadcastSettings> BroadcastSettingsPointer;

#endif /* PREFERENCES_BROADCASTSETTINGS_H */
