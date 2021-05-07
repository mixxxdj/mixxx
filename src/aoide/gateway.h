#pragma once

#include <QAtomicInteger>
#include <QByteArray>
#include <QDir>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QStringList>
#include <QUrl>

#include "aoide/json/collection.h"
#include "aoide/json/playlist.h"
#include "aoide/json/track.h"
#include "aoide/tracksearchoverlayfilter.h"
#include "aoide/util.h"
#include "network/jsonwebtask.h"
#include "util/optional.h"
#include "util/parented_ptr.h"

class Track;

namespace aoide {

class CreateCollectionTask;
class CreateCollectedPlaylistTask;
class DeleteCollectionTask;
class DeletePlaylistTask;
class ListCollectionsTask;
class ListTagsTask;
class ListTagsFacetsTask;
class ListCollectedPlaylistsTask;
class PlaylistAppendTrackEntriesByUrlTask;
class PurgeTracksTask;
class RelocateCollectedTracksTask;
class ReplaceCollectedTracksTask;
class ResolveCollectedTracksTask;
class SearchCollectedTracksTask;
class UpdateCollectionTask;

/// Manages network communication with an aoide service.
///
/// It ensures that all pending write tasks are finished before
/// shutting down the client.
///
/// All public methods are thread-safe and can be invoked from
/// any thread. The network tasks live in the same thread as
/// the gateway instance.
class Gateway : public QObject {
    Q_OBJECT

  public:
    explicit Gateway(
            const QUrl& baseUrl,
            QObject* parent = nullptr);
    ~Gateway() override;

    void invokeShutdown(
            int timeoutMillis = 0);

    ListCollectionsTask* listCollections(
            const QString& kind = {},
            const Pagination& pagination = {});

    CreateCollectionTask* createCollection(
            json::Collection collection);
    UpdateCollectionTask* updateCollection(
            json::CollectionEntity collectionEntity);
    DeleteCollectionTask* deleteCollection(
            QString collectionUid);

    SearchCollectedTracksTask* searchTracks(
            const QString& collectionUid,
            const QJsonObject& baseQuery = {},
            const TrackSearchOverlayFilter& overlayFilter = {},
            const QStringList& searchTerms = {},
            const Pagination& pagination = {});

    ResolveCollectedTracksTask* resolveTracksByUrl(
            const QString& collectionUid,
            QList<QUrl> trackUrls);

    ReplaceCollectedTracksTask* replaceTracks(
            const QString& collectionUid,
            const QList<json::Track>& tracks);
    RelocateCollectedTracksTask* relocateTracks(
            const QString& collectionUid,
            const QList<QPair<QString, QString>>& trackRelocations);
    RelocateCollectedTracksTask* relocateAllTracks(
            const QString& collectionUid,
            const QDir& oldRootDir,
            const QDir& newRootDir);
    PurgeTracksTask* purgeTracks(
            const QString& collectionUid,
            const QStringList& trackLocations);
    PurgeTracksTask* purgeAllTracks(
            const QString& collectionUid,
            const QDir& rootDir);

    ListCollectedPlaylistsTask* listCollectedPlaylists(
            const QString& collectionUid,
            const QString& kind);

    CreateCollectedPlaylistTask* createPlaylist(
            const QString& collectionUid,
            const json::Playlist& playlist);
    DeletePlaylistTask* deletePlaylist(
            QString playlistUid);
    PlaylistAppendTrackEntriesByUrlTask* playlistAppendTrackEntriesByUrl(
            QString collectionUid,
            json::EntityHeader playlistEntityHeader,
            QList<QUrl> trackUrls);

    ListTagsTask* listTags(
            const QString& collectionUid,
            const std::optional<QStringList>& facets = std::nullopt,
            const Pagination& pagination = {});
    ListTagsFacetsTask* listTagsFacets(
            const QString& collectionUid,
            const std::optional<QStringList>& facets = std::nullopt,
            const Pagination& pagination = {});

  public slots:
    void slotShutdown(
            int timeoutMillis);

  signals:
    void shuttingDown();

  private slots:
    void slotNetworkTaskAborted(
            const QUrl& requestUrl);
    void slotWebTaskNetworkError(
            QNetworkReply::NetworkError errorCode,
            const QString& errorString,
            const mixxx::network::WebResponseWithContent responseWithContent);
    void slotJsonWebTaskFailed(
            const mixxx::network::JsonWebResponse& response);
    void slotWriteTaskDestroyed();

  private:
    template<typename T, typename... Args>
    T* newReadingNetworkTask(Args&&... args) {
        auto* const pNetworkTask = new T(std::forward<Args>(args)...);
        pNetworkTask->moveToThread(thread());
        pNetworkTask->setParent(this);
        connect(pNetworkTask,
                &mixxx::network::NetworkTask::aborted,
                this,
                &Gateway::slotNetworkTaskAborted);
        return pNetworkTask;
    }
    template<typename T, typename... Args>
    T* newReadingWebTask(Args&&... args) {
        auto* const pWebTask = newReadingNetworkTask<T>(std::forward<Args>(args)...);
        connect(pWebTask,
                &mixxx::network::WebTask::networkError,
                this,
                &Gateway::slotWebTaskNetworkError);
        return pWebTask;
    }
    template<typename T, typename... Args>
    T* newReadingJsonWebTask(Args&&... args) {
        auto* const pJsonWebTask = newReadingWebTask<T>(std::forward<Args>(args)...);
        connect(pJsonWebTask,
                &mixxx::network::JsonWebTask::failed,
                this,
                &Gateway::slotJsonWebTaskFailed);
        return pJsonWebTask;
    }
    template<typename T, typename... Args>
    T* newWritingNetworkTask(Args&&... args) {
        auto* const pNetworkTask = newReadingNetworkTask<T>(std::forward<Args>(args)...);
        connectPendingWriteTask(pNetworkTask);
        return pNetworkTask;
    }
    template<typename T, typename... Args>
    T* newWritingJsonWebTask(Args&&... args) {
        auto* const pJsonWebTask = newReadingJsonWebTask<T>(std::forward<Args>(args)...);
        connectPendingWriteTask(pJsonWebTask);
        return pJsonWebTask;
    }

    void connectPendingWriteTask(
            mixxx::network::NetworkTask* task);

    const QUrl m_baseUrl;

    const parented_ptr<QNetworkAccessManager> m_networkAccessManager;

    QAtomicInteger<quint32> m_pendingWriteTasks;

    enum class State {
        Active,
        ShutdownPending,
        ShuttingDown,
    };
    State m_state;

    int m_shutdownTimeoutMillis;
};

} // namespace aoide
