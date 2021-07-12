#pragma once

#include <QFuture>
#include <QFutureWatcher>
#include <QStringListModel>
#include <QXmlStreamReader>
#include <QtConcurrentRun>
#include <QtSql>

#include "library/baseexternallibraryfeature.h"
#include "library/trackcollection.h"
#include "library/treeitemmodel.h"
#include "util/parented_ptr.h"

class BaseExternalTrackModel;
class BaseExternalPlaylistModel;

class RhythmboxFeature : public BaseExternalLibraryFeature {
    Q_OBJECT
 public:
    RhythmboxFeature(Library* pLibrary, UserSettingsPointer pConfig);
    virtual ~RhythmboxFeature();
    static bool isSupported();

    QVariant title();

    TreeItemModel* sidebarModel() const;
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
    parented_ptr<TreeItemModel> m_pSidebarModel;
    bool m_cancelImport;

    QSharedPointer<BaseTrackCache>  m_trackSource;
};
