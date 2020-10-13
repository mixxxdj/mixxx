
#ifndef CLEMENTINEFEATURE_H
#define CLEMENTINEFEATURE_H

#include <QStringListModel>
#include <QtSql>
#include <QFuture>
#include <QtConcurrentRun>
#include <QFutureWatcher>

#include "library/baseexternallibraryfeature.h"
#include "library/trackcollection.h"
#include "library/treeitemmodel.h"
#include "library/treeitem.h"
#include "library/clementine/clementinedbconnection.h"


class ClementinePlaylistModel;

class ClementineFeature : public BaseExternalLibraryFeature {
    Q_OBJECT
  public:
    ClementineFeature(Library* pLibrary, UserSettingsPointer pConfig);
    virtual ~ClementineFeature();
    static bool isSupported();
    static void prepareDbPath(UserSettingsPointer pConfig);

    virtual QVariant title();
    virtual QIcon getIcon();

    //QString getSettingsName() const override;

    virtual TreeItemModel* getChildModel();

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
    QIcon m_icon;

    static QString m_databaseFile;

    static const QString Clementine_MOUNT_KEY;
};

#endif // CLEMENTINEFEATURE_H
