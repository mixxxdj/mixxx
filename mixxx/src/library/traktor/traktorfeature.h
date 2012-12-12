// traktorfeature.h
// Created 9/26/2010 by Tobias Rafreider

#ifndef TRAKTOR_FEATURE_H
#define TRAKTOR_FEATURE_H

#include <QStringListModel>
#include <QtSql>
#include <QXmlStreamReader>
#include <QFuture>
#include <QtConcurrentRun>
#include <QFutureWatcher>

#include "library/baseexternallibraryfeature.h"
#include "library/baseexternaltrackmodel.h"
#include "library/baseexternalplaylistmodel.h"
#include "library/treeitemmodel.h"

class LibraryTableModel;
class MissingTableModel;
class TrackCollection;
class BaseExternalPlaylistModel;

class TraktorTrackModel : public BaseExternalTrackModel {
  public:
    TraktorTrackModel(QObject* parent,
                      TrackCollection* pTrackCollection);
    virtual bool isColumnHiddenByDefault(int column);
};

class TraktorPlaylistModel : public BaseExternalPlaylistModel {
  public:
    TraktorPlaylistModel(QObject* parent,
                         TrackCollection* pTrackCollection);
    virtual bool isColumnHiddenByDefault(int column);
};

class TraktorFeature : public BaseExternalLibraryFeature {
    Q_OBJECT
  public:
    TraktorFeature(QObject* parent, TrackCollection*);
    virtual ~TraktorFeature();

    QVariant title();
    QIcon getIcon();
    static bool isSupported();

    TreeItemModel* getChildModel();

  public slots:
    void activate();
    void activateChild(const QModelIndex& index);
    void refreshLibraryModels();
    void onTrackCollectionLoaded();

  private:
    virtual BaseSqlTableModel* getPlaylistModelForPlaylist(QString playlist);
    TreeItem* importLibrary(QString file);
    /** parses a track in the music collection **/
    void parseTrack(QXmlStreamReader &xml, QSqlQuery &query);
    /** Iterates over all playliost and folders and constructs the childmodel **/
    TreeItem* parsePlaylists(QXmlStreamReader &xml);
    /** processes a particular playlist **/
    void parsePlaylistEntries(QXmlStreamReader &xml, QString playlist_path,
    QSqlQuery query_insert_into_playlist, QSqlQuery query_insert_into_playlisttracks);
    void clearTable(QString table_name);
    static QString getTraktorMusicDatabase();
    //private fields
    TreeItemModel m_childModel;
    TrackCollection* m_pTrackCollection;
    //A separate db connection for the worker parsing thread
    QSqlDatabase m_database;
    TraktorTrackModel* m_pTraktorTableModel;
    TraktorPlaylistModel* m_pTraktorPlaylistModel;

    bool m_isActivated;
    bool m_cancelImport;
    QFutureWatcher<TreeItem*> m_future_watcher;
    QFuture<TreeItem*> m_future;
    QString m_title;
};

#endif /* TRAKTOR_FEATURE_H */
