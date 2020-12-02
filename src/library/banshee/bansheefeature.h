
#ifndef BANSHEEFEATURE_H
#define BANSHEEFEATURE_H

#include <QByteArrayData>
#include <QFuture>
#include <QFutureWatcher>
#include <QIcon>
#include <QSqlDatabase>
#include <QString>
#include <QStringList>
#include <QStringListModel>
#include <QVariant>
#include <QtConcurrentRun>
#include <QtSql>

#include "library/banshee/bansheedbconnection.h"
#include "library/baseexternallibraryfeature.h"
#include "library/trackcollection.h"
#include "library/treeitem.h"
#include "library/treeitemmodel.h"
#include "preferences/usersettings.h"

class BansheePlaylistModel;
class Library;
class QModelIndex;
class QObject;
class TrackId;
class TreeItem;
template<class T>
class QList;

class BansheeFeature : public BaseExternalLibraryFeature {
    Q_OBJECT
  public:
    BansheeFeature(Library* pLibrary, UserSettingsPointer pConfig);
    virtual ~BansheeFeature();
    static bool isSupported();
    static void prepareDbPath(UserSettingsPointer pConfig);

    virtual QVariant title();
    virtual QIcon getIcon();

    virtual TreeItemModel* getChildModel();

  public slots:
    virtual void activate();
    virtual void activateChild(const QModelIndex& index);

  private:
    virtual void appendTrackIdsFromRightClickIndex(QList<TrackId>* trackIds, QString* pPlaylist);

    BansheePlaylistModel* m_pBansheePlaylistModel;
    TreeItemModel m_childModel;
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
    QIcon m_icon;

    static QString m_databaseFile;

    static const QString BANSHEE_MOUNT_KEY;
};

#endif // BANSHEEFEATURE_H
