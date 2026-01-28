#pragma once

#include <QStringListModel>
#include <QXmlStreamReader>
#include <QFuture>
#include <QtConcurrentRun>
#include <QFutureWatcher>

#include "library/baseexternallibraryfeature.h"
#include "library/baseexternaltrackmodel.h"
#include "library/baseexternalplaylistmodel.h"
#include "library/treeitemmodel.h"

class TraktorTrackModel : public BaseExternalTrackModel {
    Q_OBJECT
  public:
    TraktorTrackModel(QObject* parent,
                      TrackCollectionManager* pTrackCollectionManager,
                      QSharedPointer<BaseTrackCache> trackSource);
    virtual bool isColumnHiddenByDefault(int column);
};

class TraktorPlaylistModel : public BaseExternalPlaylistModel {
    Q_OBJECT
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
    static bool isSupported();

    TreeItemModel* sidebarModel() const override;

  public slots:
    void activate() override;
    void activateChild(const QModelIndex& index) override;
    void refreshLibraryModels();
    void onTrackCollectionLoaded();

  private:
    std::unique_ptr<BaseSqlTableModel> createPlaylistModelForPlaylist(
            const QVariant& data) override;
    TreeItem* importLibrary(const QString& file);
    // parses a track in the music collection
    void parseTrack(QXmlStreamReader &xml, QSqlQuery &query);
    // Iterates over all playliost and folders and constructs the childmodel
    TreeItem* parsePlaylists(QXmlStreamReader &xml);
    // processes a particular playlist
    void parsePlaylistEntries(QXmlStreamReader& xml,
            const QString& playlist_path,
            QSqlQuery* pQueryInsertIntoPlaylist,
            QSqlQuery* pQueryInsertIntoPlaylistTracks);
    void clearTable(const QString& table_name);
    static QString getTraktorMusicDatabase();
    // private fields
    parented_ptr<TreeItemModel> m_pSidebarModel;
    // A separate db connection for the worker parsing thread
    QSqlDatabase m_database;
    TraktorTrackModel* m_pTraktorTableModel;
    TraktorPlaylistModel* m_pTraktorPlaylistModel;

    bool m_isActivated;
    // TODO: Wrap this flag in `std::atomic` (as in `ITunesFeature`)
    bool m_cancelImport;
    QFutureWatcher<TreeItem*> m_future_watcher;
    QFuture<TreeItem*> m_future;
    QString m_title;

    QSharedPointer<BaseTrackCache> m_trackSource;
};
