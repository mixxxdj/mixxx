// rhythmboxfeature.h
// Created 8/23/2009 by RJ Ryan (rryan@mit.edu)

#ifndef RHYTHMBOXFEATURE_H
#define RHYTHMBOXFEATURE_H

#include <QStringListModel>
#include <QtSql>
#include <QXmlStreamReader>
#include <QFuture>
#include <QtConcurrentRun>
#include <QFutureWatcher>

#include "library/features/baseexternalfeature/baseexternallibraryfeature.h"
#include "library/treeitemmodel.h"
#include "library/trackcollection.h"

class BaseExternalTrackModel;
class BaseExternalPlaylistModel;

class RhythmboxFeature : public BaseExternalLibraryFeature {
    Q_OBJECT
 public:
    RhythmboxFeature(UserSettingsPointer pConfig,
                     Library* pLibrary,
                     QObject* parent,
                     TrackCollection* pTrackCollection);
    virtual ~RhythmboxFeature();
    static bool isSupported();

    QVariant title();
    QString getIconPath() override;
    QString getSettingsName() const override;

    QPointer<TreeItemModel> getChildModel();
    // processes the music collection
    TreeItem* importMusicCollection();
    // processes the playlist entries
    TreeItem* importPlaylists();

  public slots:
    void activate();
    void activateChild(const QModelIndex& index);
    void onTrackCollectionLoaded();

  private:
    virtual BaseSqlTableModel* getPlaylistModelForPlaylist(QString playlist);
    // Removes all rows from a given table
    void clearTable(QString table_name);
    // reads the properties of a track and executes a SQL statement
    void importTrack(QXmlStreamReader &xml, QSqlQuery &query);
    // reads all playlist entries and executes a SQL statement
    void importPlaylist(QXmlStreamReader &xml, QSqlQuery &query, int playlist_id);

    BaseExternalTrackModel* m_pRhythmboxTrackModel;
    BaseExternalPlaylistModel* m_pRhythmboxPlaylistModel;

    TrackCollection* m_pTrackCollection;
    // new DB object because of threads
    QSqlDatabase m_database;
    bool m_isActivated;
    QString m_title;

    QFutureWatcher<TreeItem*> m_track_watcher;
    QFuture<TreeItem*> m_track_future;
    TreeItemModel m_childModel;
    bool m_cancelImport;

    QSharedPointer<BaseTrackCache>  m_trackSource;
};

#endif // RHYTHMBOXFEATURE_H
