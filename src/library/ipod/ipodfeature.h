
#ifndef IPODFEATURE_H
#define IPODFEATURE_H

#include <QStringListModel>
#include <QtSql>
#include <QFuture>
#include <QtConcurrentRun>
#include <QFutureWatcher>

#include "library/baseexternallibraryfeature.h"
#include "library/trackcollection.h"
#include "library/treeitemmodel.h"
#include "library/treeitem.h"

extern "C"
{
#include <gpod/itdb.h>
}

class IPodPlaylistModel;
class GPodItdb;

class IPodFeature : public BaseExternalLibraryFeature {
    Q_OBJECT
  public:
    IPodFeature(QObject* parent, TrackCollection* pTrackCollection);
    virtual ~IPodFeature();
    static bool isSupported();

    virtual QVariant title();
    virtual QIcon getIcon();

    virtual TreeItemModel* getChildModel();

  public slots:
    void activate();
    void activate(bool forceReload);
    void activateChild(const QModelIndex& index);
    void onTrackCollectionLoaded();

  private:
    virtual void appendTrackIdsFromRightClickIndex(QList<int>* trackIds, QString* pPlaylist);
    // returns the invisible rootItem for the sidebar model
    TreeItem* importLibrary();
    QString detectMountPoint(QString iPodMountPoint);

    IPodPlaylistModel* m_pIPodPlaylistModel;
    TreeItemModel m_childModel;
    QStringList m_playlists;
    TrackCollection* m_pTrackCollection;
    //a new DB connection for the worker thread
    QSqlDatabase m_database;
    bool m_isActivated;
    QString m_dbfile;

    QFutureWatcher<TreeItem*> m_future_watcher;
    QFuture<TreeItem*> m_future;
    QString m_title;
    bool m_cancelImport;

    QString m_dbItunesRoot;
    QString m_mixxxItunesRoot;

    GPodItdb* m_gPodItdb;

    Itdb_iTunesDB* m_itdb;

    static const QString IPOD_MOUNT_KEY;
};

#endif // IPODFEATURE_H
