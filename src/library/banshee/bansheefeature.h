#pragma once

#include <QStringListModel>
#include <QtSql>
#include <QFuture>
#include <QtConcurrentRun>
#include <QFutureWatcher>

#include "library/baseexternallibraryfeature.h"
#include "library/trackcollection.h"
#include "library/treeitemmodel.h"
#include "library/treeitem.h"
#include "library/banshee/bansheedbconnection.h"


class BansheePlaylistModel;

class BansheeFeature : public BaseExternalLibraryFeature {
    Q_OBJECT
  public:
    BansheeFeature(Library* pLibrary, UserSettingsPointer pConfig);
    virtual ~BansheeFeature();
    static bool isSupported();
    static void prepareDbPath(UserSettingsPointer pConfig);

    virtual QVariant title();

    virtual TreeItemModel* sidebarModel() const;

  public slots:
    virtual void activate();
    virtual void activateChild(const QModelIndex& index);

  private:
    virtual void appendTrackIdsFromRightClickIndex(QList<TrackId>* trackIds, QString* pPlaylist);

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
    bool m_cancelImport;

    static QString m_databaseFile;

    static const QString BANSHEE_MOUNT_KEY;
};
