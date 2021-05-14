#pragma once

#include <QStringListModel>
#include <QtSql>
#include <QXmlStreamReader>
#include <QFuture>
#include <QtConcurrentRun>
#include <QFutureWatcher>

#include "library/baseexternallibraryfeature.h"
#include "library/treeitemmodel.h"
#include "library/trackcollection.h"

class BaseExternalTrackModel;
class BaseExternalPlaylistModel;

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
