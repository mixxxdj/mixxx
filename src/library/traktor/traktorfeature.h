// traktorfeature.h
// Created 9/26/2010 by Tobias Rafreider

#ifndef TRAKTOR_FEATURE_H
#define TRAKTOR_FEATURE_H

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
#include "library/baseexternalplaylistmodel.h"
#include "library/baseexternaltrackmodel.h"
#include "library/treeitemmodel.h"
#include "preferences/usersettings.h"

class BaseSqlTableModel;
class BaseTrackCache;
class Library;
class QModelIndex;
class QObject;
class QSqlQuery;
class QXmlStreamReader;
class TrackCollectionManager;
class TreeItem;
template<class T>
class QSharedPointer;

class TraktorTrackModel : public BaseExternalTrackModel {
  public:
    TraktorTrackModel(QObject* parent,
                      TrackCollectionManager* pTrackCollectionManager,
                      QSharedPointer<BaseTrackCache> trackSource);
    virtual bool isColumnHiddenByDefault(int column);
};

class TraktorPlaylistModel : public BaseExternalPlaylistModel {
  public:
    TraktorPlaylistModel(QObject* parent,
                         TrackCollectionManager* pTrackCollectionManager,
                         QSharedPointer<BaseTrackCache> trackSource);
    virtual bool isColumnHiddenByDefault(int column);
};

class TraktorFeature : public BaseExternalLibraryFeature {
    Q_OBJECT
  public:
    TraktorFeature(Library* pLibrary, UserSettingsPointer pConfig);
    virtual ~TraktorFeature();

    QVariant title() override;
    QIcon getIcon() override;
    static bool isSupported();

    TreeItemModel* getChildModel() override;

  public slots:
    void activate() override;
    void activateChild(const QModelIndex& index) override;
    void refreshLibraryModels();
    void onTrackCollectionLoaded();

  private:
    BaseSqlTableModel* getPlaylistModelForPlaylist(const QString& playlist) override;
    TreeItem* importLibrary(const QString& file);
    // parses a track in the music collection
    void parseTrack(QXmlStreamReader &xml, QSqlQuery &query);
    // Iterates over all playliost and folders and constructs the childmodel
    TreeItem* parsePlaylists(QXmlStreamReader &xml);
    // processes a particular playlist
    void parsePlaylistEntries(QXmlStreamReader& xml,
            const QString& playlist_path,
            QSqlQuery query_insert_into_playlist,
            QSqlQuery query_insert_into_playlisttracks);
    void clearTable(const QString& table_name);
    static QString getTraktorMusicDatabase();
    // private fields
    TreeItemModel m_childModel;
    // A separate db connection for the worker parsing thread
    QSqlDatabase m_database;
    TraktorTrackModel* m_pTraktorTableModel;
    TraktorPlaylistModel* m_pTraktorPlaylistModel;

    bool m_isActivated;
    bool m_cancelImport;
    QFutureWatcher<TreeItem*> m_future_watcher;
    QFuture<TreeItem*> m_future;
    QString m_title;

    QSharedPointer<BaseTrackCache> m_trackSource;
    QIcon m_icon;
};

#endif // TRAKTOR_FEATURE_H
