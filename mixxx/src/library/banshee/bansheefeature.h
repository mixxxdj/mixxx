#pragma once

#include <QFuture>
#include <QFutureWatcher>

#include "library/baseexternallibraryfeature.h"
#include "library/banshee/bansheedbconnection.h"

class BansheePlaylistModel;

class BansheeFeature : public BaseExternalLibraryFeature {
    Q_OBJECT
  public:
    BansheeFeature(Library* pLibrary, UserSettingsPointer pConfig);
    ~BansheeFeature() override;
    static bool isSupported();
    static void prepareDbPath(UserSettingsPointer pConfig);

    QVariant title() override;

    TreeItemModel* sidebarModel() const override;

  public slots:
    void activate() override;
    void activateChild(const QModelIndex& index) override;

  private:
    void appendTrackIdsFromRightClickIndex(QList<TrackId>* trackIds, QString* pPlaylist) override;

    BansheePlaylistModel* m_pBansheePlaylistModel;
    parented_ptr<TreeItemModel> m_pSidebarModel;
    QStringList m_playlists;

    //a new DB connection for the worker thread
    BansheeDbConnection m_connection;

    QSqlDatabase m_database;
    bool m_isActivated;
    QString m_dbfile;

    QFutureWatcher<TreeItem*> m_future_watcher;
    QFuture<TreeItem*> m_future;
    QString m_title;
    // TODO: Wrap this flag in `std::atomic` (as in `ITunesFeature`)
    bool m_cancelImport;

    static QString m_databaseFile;

    static const QString BANSHEE_MOUNT_KEY;
};
