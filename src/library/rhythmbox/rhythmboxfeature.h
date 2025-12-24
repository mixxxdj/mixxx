#pragma once

#include <QFuture>
#include <QFutureWatcher>

#include "library/baseexternallibraryfeature.h"
#include "util/parented_ptr.h"

class BaseExternalTrackModel;
class BaseExternalPlaylistModel;
class QXmlStreamReader;
class BaseTrackCache;

class RhythmboxFeature : public BaseExternalLibraryFeature {
    Q_OBJECT
 public:
    RhythmboxFeature(Library* pLibrary, UserSettingsPointer pConfig);
    ~RhythmboxFeature() override;
    static bool isSupported();

    QVariant title() override;

    TreeItemModel* sidebarModel() const override;
    // processes the music collection
    TreeItem* importMusicCollection();
    // processes the playlist entries
    TreeItem* importPlaylists();

  public slots:
    void activate() override;
    void activateChild(const QModelIndex& index) override;
    void onTrackCollectionLoaded();

  protected:
    std::unique_ptr<BaseSqlTableModel> createPlaylistModelForPlaylist(
            const QVariant& data) override;

  private:
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
    // TODO: Wrap this flag in `std::atomic` (as in `ITunesFeature`)
    bool m_cancelImport;

    QSharedPointer<BaseTrackCache>  m_trackSource;
};
