// rhythmboxfeature.h
// Created 8/23/2009 by RJ Ryan (rryan@mit.edu)

#ifndef RHYTHMBOXFEATURE_H
#define RHYTHMBOXFEATURE_H

#include <QByteArrayData>
#include <QFuture>
#include <QFutureWatcher>
#include <QIcon>
#include <QSharedPointer>
#include <QSqlDatabase>
#include <QString>
#include <QStringListModel>
#include <QVariant>
#include <QXmlStreamReader>
#include <QtConcurrentRun>
#include <QtSql>

#include "library/baseexternallibraryfeature.h"
#include "library/trackcollection.h"
#include "library/treeitemmodel.h"
#include "preferences/usersettings.h"

class BaseExternalTrackModel;
class BaseExternalPlaylistModel;
class BaseSqlTableModel;
class BaseTrackCache;
class Library;
class QModelIndex;
class QObject;
class QSqlQuery;
class QXmlStreamReader;
class TreeItem;

class RhythmboxFeature : public BaseExternalLibraryFeature {
    Q_OBJECT
 public:
    RhythmboxFeature(Library* pLibrary, UserSettingsPointer pConfig);
    virtual ~RhythmboxFeature();
    static bool isSupported();

    QVariant title();
    QIcon getIcon();

    TreeItemModel* getChildModel();
    // processes the music collection
    TreeItem* importMusicCollection();
    // processes the playlist entries
    TreeItem* importPlaylists();

  public slots:
    void activate();
    void activateChild(const QModelIndex& index);
    void onTrackCollectionLoaded();

  private:
    virtual BaseSqlTableModel* getPlaylistModelForPlaylist(const QString& playlist);
    // Removes all rows from a given table
    void clearTable(const QString& table_name);
    // reads the properties of a track and executes a SQL statement
    void importTrack(QXmlStreamReader &xml, QSqlQuery &query);
    // reads all playlist entries and executes a SQL statement
    void importPlaylist(QXmlStreamReader &xml, QSqlQuery &query, int playlist_id);

    BaseExternalTrackModel* m_pRhythmboxTrackModel;
    BaseExternalPlaylistModel* m_pRhythmboxPlaylistModel;

    // new DB object because of threads
    QSqlDatabase m_database;
    bool m_isActivated;
    QString m_title;

    QFutureWatcher<TreeItem*> m_track_watcher;
    QFuture<TreeItem*> m_track_future;
    TreeItemModel m_childModel;
    bool m_cancelImport;

    QSharedPointer<BaseTrackCache>  m_trackSource;
    QIcon m_icon;
};

#endif // RHYTHMBOXFEATURE_H
