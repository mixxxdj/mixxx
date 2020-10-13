#pragma once

#include <QStringListModel>
#include <QtSql>
#include <QFuture>
#include <QtConcurrentRun>
#include <QFutureWatcher>

#include "library/features/baseexternalfeature/baseexternallibraryfeature.h"
#include "library/trackcollection.h"
#include "library/treeitemmodel.h"
#include "library/treeitem.h"
#include "library/features/clementine/clementinedbconnection.h"


class ClementinePlaylistModel;

class ClementineFeature : public BaseExternalLibraryFeature {
    Q_OBJECT
  public:
    ClementineFeature(UserSettingsPointer pConfig,
                                   Library* pLibrary,
                                   QObject* parent,
                                   TrackCollection* pTrackCollection);
    virtual ~ClementineFeature();
    static bool isSupported();
    static void prepareDbPath(UserSettingsPointer pConfig);

    QVariant title() override;
    QString getIconPath() override;
    QString getSettingsName() const override;

    virtual QPointer<TreeItemModel> getChildModel();

  public slots:
    virtual void activate();
    virtual void activateChild(const QModelIndex& index);

  private:
    virtual void appendTrackIdsFromRightClickIndex(QList<TrackId>* trackIds, QString* pPlaylist);

    ClementinePlaylistModel* m_pClementinePlaylistModel;
    TreeItemModel m_childModel;
    QStringList m_playlists;
    TrackCollection* m_pTrackCollection;
    //a new DB connection for the worker thread

    ClementineDbConnection m_connection;

    QSqlDatabase m_database;
    bool m_isActivated;
    QString m_dbfile;

    QFutureWatcher<TreeItem*> m_future_watcher;
    QFuture<TreeItem*> m_future;
    QString m_title;
    bool m_cancelImport;

    static QString m_databaseFile;

    static const QString Clementine_MOUNT_KEY;
};
