#pragma once

#include <QFuture>
#include <QFutureWatcher>
#include <QStringListModel>
#include <QtConcurrentRun>
#include <QtSql>

#include "library/baseexternallibraryfeature.h"
#include "library/clementine/clementinedbconnection.h"
#include "library/clementine/clementineplaylistmodel.h"
#include "library/treeitem.h"
#include "library/treeitemmodel.h"


class ClementineFeature : public BaseExternalLibraryFeature {
    Q_OBJECT
  public:
    ClementineFeature(Library* pLibrary, UserSettingsPointer pConfig);
    ~ClementineFeature() override;
    static bool isSupported();

    QVariant title() override;
    QIcon getIcon() override;

    TreeItemModel* getChildModel() override;

  public slots:
    void activate() override;
    void activateChild(const QModelIndex& index) override;

  private:
    void appendTrackIdsFromRightClickIndex(QList<TrackId>* trackIds, QString* pPlaylist) override;

    ClementinePlaylistModel* m_pClementinePlaylistModel;
    TreeItemModel m_childModel;
    QStringList m_playlists;

    //a new DB connection for the worker thread
    ClementineDbConnection m_connection;

    QSqlDatabase m_database;
    bool m_isActivated;
    QString m_dbfile;

    QFutureWatcher<TreeItem*> m_future_watcher;
    QFuture<TreeItem*> m_future;
    QString m_title;
    bool m_cancelImport;
    QIcon m_icon;

    QString m_databaseFile;
};
