#include "aoide/gateway.h"

#include <QMetaObject>
#include <QThread>
#include <mutex> // std::once_flag

#include "aoide/web/createcollectedplaylisttask.h"
#include "aoide/web/createcollectiontask.h"
#include "aoide/web/deletecollectiontask.h"
#include "aoide/web/deleteplaylisttask.h"
#include "aoide/web/listcollectedplayliststask.h"
#include "aoide/web/listcollectionstask.h"
#include "aoide/web/listtagsfacetstask.h"
#include "aoide/web/listtagstask.h"
#include "aoide/web/playlistappendtrackentriesbyurltask.h"
#include "aoide/web/purgecollectedtrackstask.h"
#include "aoide/web/relocatecollectedtrackstask.h"
#include "aoide/web/replacecollectedtrackstask.h"
#include "aoide/web/resolvecollectedtrackstask.h"
#include "aoide/web/searchcollectedtrackstask.h"
#include "aoide/web/shutdowntask.h"
#include "aoide/web/updatecollectiontask.h"
#include "util/encodedurl.h"
#include "util/logger.h"

namespace {

const mixxx::Logger kLogger("aoide Gateway");

std::once_flag registerMetaTypesOnceFlag;

} // anonymous namespace

namespace aoide {

namespace {

void registerMetaTypesOnce() {
    qRegisterMetaType<Pagination>("aoide::Pagination");

    qRegisterMetaType<json::EntityUid>("aoide::json::EntityUid");
    qRegisterMetaType<json::EntityRevision>("aoide::json::EntityRevision");
    qRegisterMetaType<json::EntityHeader>("aoide::json::EntityHeader");

    qRegisterMetaType<json::Collection>("aoide::json::Collection");
    qRegisterMetaType<json::CollectionEntity>("aoide::json::CollectionEntity");
    qRegisterMetaType<QVector<json::CollectionEntity>>("aoide::QVector<json::CollectionEntity>");

    qRegisterMetaType<json::Title>("aoide::json::Title");
    qRegisterMetaType<json::TitleVector>("aoide::json::TitleVector");
    qRegisterMetaType<json::Actor>("aoide::json::Actor");
    qRegisterMetaType<json::ActorVector>("aoide::json::ActorVector");

    qRegisterMetaType<json::TrackOrAlbum>("aoide::json::TrackOrAlbum");
    qRegisterMetaType<json::Release>("aoide::json::Release");
    qRegisterMetaType<json::Album>("aoide::json::Album");
    qRegisterMetaType<json::Track>("aoide::json::Track");
    qRegisterMetaType<json::TrackEntity>("aoide::json::TrackEntity");
    qRegisterMetaType<QVector<json::TrackEntity>>("aoide::QVector<json::TrackEntity>");

    qRegisterMetaType<json::TagFacetCount>("aoide::json::TagFacetCount");
    qRegisterMetaType<QVector<json::TagFacetCount>>("aoide::QVector<json::TagFacetCount>");
    qRegisterMetaType<json::TagCount>("aoide::json::TagCount");
    qRegisterMetaType<QVector<json::TagCount>>("aoide::QVector<json::TagCount>");

    qRegisterMetaType<json::Playlist>("aoide::json::Playlist");
    qRegisterMetaType<json::PlaylistEntry>("aoide::json::PlaylistEntry");
    qRegisterMetaType<json::PlaylistEntity>("aoide::json::PlaylistEntity");
    qRegisterMetaType<QVector<json::PlaylistEntity>>("aoide::QVector<json::PlaylistEntity>");
    qRegisterMetaType<json::PlaylistWithEntriesSummary>("aoide::json::PlaylistWithEntriesSummary");
    qRegisterMetaType<json::PlaylistWithEntriesSummary>("aoide::json::PlaylistWithEntriesSummary");
    qRegisterMetaType<json::PlaylistWithEntriesSummaryEntity>(
            "aoide::json::PlaylistWithEntriesSummaryEntity");
    qRegisterMetaType<QVector<json::PlaylistWithEntriesSummaryEntity>>(
            "aoide::QVector<json::PlaylistWithEntriesSummaryEntity>");

    qRegisterMetaType<aoide::TrackSearchOverlayFilter>(
            "aoide::TrackSearchOverlayFilter");
}

} // anonymous namespace

Gateway::Gateway(
        const QUrl& baseUrl,
        QObject* parent)
        : QObject(parent),
          m_baseUrl(std::move(baseUrl)),
          m_networkAccessManager(new QNetworkAccessManager(this)),
          m_pendingWriteTasks(0),
          m_state(State::Active),
          m_shutdownTimeoutMillis(0) {
    std::call_once(registerMetaTypesOnceFlag, registerMetaTypesOnce);
    DEBUG_ASSERT(m_baseUrl.isValid());
}

Gateway::~Gateway() = default;

void Gateway::invokeShutdown(int timeoutMillis) {
    QMetaObject::invokeMethod(
            this,
            [this, timeoutMillis] {
                this->slotShutdown(timeoutMillis);
            });
}

void Gateway::slotShutdown(int timeoutMillis) {
    if (m_state == State::ShuttingDown) {
        return;
    }
    m_state = State::ShutdownPending;
    m_shutdownTimeoutMillis = timeoutMillis;
    const auto pendingWriteTasks = m_pendingWriteTasks.loadAcquire();
    if (pendingWriteTasks > 0) {
        kLogger.info()
                << "Delaying shutdown until"
                << pendingWriteTasks
                << "pending write task(s) have been finished";
        return;
    }
    kLogger.info()
            << "Shutting down";
    auto* task = new ShutdownTask(
            m_networkAccessManager,
            m_baseUrl);
    m_state = State::ShuttingDown;
    // The started task will be deleted implicitly after
    // receiving a reply.
    task->invokeStart(
            m_shutdownTimeoutMillis);
    emit shuttingDown();
}

void Gateway::slotNetworkTaskAborted(
        const QUrl& requestUrl) {
    auto* const pWebTask =
            qobject_cast<mixxx::network::WebTask*>(sender());
    VERIFY_OR_DEBUG_ASSERT(pWebTask) {
        return;
    }
    DEBUG_ASSERT(pWebTask->parent() == this);
    kLogger.info()
            << pWebTask
            << "Web task aborted"
            << requestUrl;
    pWebTask->deleteLater();
}

void Gateway::slotWebTaskNetworkError(
        QNetworkReply::NetworkError errorCode,
        const QString& errorString,
        const mixxx::network::WebResponseWithContent responseWithContent) {
    auto* const pWebTask =
            qobject_cast<mixxx::network::WebTask*>(sender());
    VERIFY_OR_DEBUG_ASSERT(pWebTask) {
        return;
    }
    DEBUG_ASSERT(pWebTask->parent() == this);
    kLogger.warning()
            << pWebTask
            << "Web task failed with network error"
            << errorCode
            << errorString
            << responseWithContent;
    pWebTask->deleteLater();
}

void Gateway::slotJsonWebTaskFailed(
        const mixxx::network::JsonWebResponse& response) {
    auto* const pJsonWebTask =
            qobject_cast<mixxx::network::JsonWebTask*>(sender());
    VERIFY_OR_DEBUG_ASSERT(pJsonWebTask) {
        return;
    }
    DEBUG_ASSERT(pJsonWebTask->parent() == this);
    kLogger.warning()
            << pJsonWebTask
            << "JSON web task failed"
            << response;
    pJsonWebTask->deleteLater();
}

void Gateway::slotWriteTaskDestroyed() {
    const auto pendingWriteTasks = m_pendingWriteTasks.fetchAndSubRelease(1);
    DEBUG_ASSERT(pendingWriteTasks > 0);
    Q_UNUSED(pendingWriteTasks)
    if (m_state == State::ShutdownPending) {
        // Retry and continue delayed shutdown
        slotShutdown(m_shutdownTimeoutMillis);
    }
}

void Gateway::connectPendingWriteTask(
        mixxx::network::NetworkTask* task) {
    // Will be invoked from multiple threads and must be thread-safe!
    connect(task,
            &QObject::destroyed,
            this,
            &Gateway::slotWriteTaskDestroyed);
    m_pendingWriteTasks.fetchAndAddAcquire(1);
}

CreateCollectionTask* Gateway::createCollection(
        json::Collection collection) {
    return newWritingJsonWebTask<CreateCollectionTask>(
            m_networkAccessManager,
            m_baseUrl,
            std::move(collection));
}

UpdateCollectionTask* Gateway::updateCollection(
        json::CollectionEntity collectionEntity) {
    return newWritingJsonWebTask<UpdateCollectionTask>(
            m_networkAccessManager,
            m_baseUrl,
            std::move(collectionEntity));
}

DeleteCollectionTask* Gateway::deleteCollection(
        QString collectionUid) {
    return newWritingJsonWebTask<DeleteCollectionTask>(
            m_networkAccessManager,
            m_baseUrl,
            std::move(collectionUid));
}

ListCollectionsTask* Gateway::listCollections(
        const QString& kind,
        const Pagination& pagination) {
    return newReadingNetworkTask<ListCollectionsTask>(
            m_networkAccessManager,
            m_baseUrl,
            kind,
            pagination);
}

SearchCollectedTracksTask* Gateway::searchTracks(
        const QString& collectionUid,
        const QJsonObject& baseQuery,
        const TrackSearchOverlayFilter& overlayFilter,
        const QStringList& searchTerms,
        const Pagination& pagination) {
    return newReadingNetworkTask<SearchCollectedTracksTask>(
            m_networkAccessManager,
            m_baseUrl,
            collectionUid,
            baseQuery,
            overlayFilter,
            searchTerms,
            pagination);
}

ReplaceCollectedTracksTask* Gateway::replaceTracks(
        const QString& collectionUid,
        const QList<json::Track>& tracks) {
    return newWritingJsonWebTask<ReplaceCollectedTracksTask>(
            m_networkAccessManager,
            m_baseUrl,
            collectionUid,
            tracks);
}

PurgeTracksTask* Gateway::purgeTracks(
        const QString& collectionUid,
        const QStringList& tracklocations) {
    return newWritingJsonWebTask<PurgeTracksTask>(
            m_networkAccessManager,
            m_baseUrl,
            collectionUid,
            tracklocations);
}

PurgeTracksTask* Gateway::purgeAllTracks(
        const QString& collectionUid,
        const QDir& rootDir) {
    return newWritingJsonWebTask<PurgeTracksTask>(
            m_networkAccessManager,
            m_baseUrl,
            collectionUid,
            rootDir);
}

RelocateCollectedTracksTask* Gateway::relocateTracks(
        const QString& collectionUid,
        const QList<QPair<QString, QString>>& trackRelocations) {
    return newWritingJsonWebTask<RelocateCollectedTracksTask>(
            m_networkAccessManager,
            m_baseUrl,
            collectionUid,
            trackRelocations);
}

RelocateCollectedTracksTask* Gateway::relocateAllTracks(
        const QString& collectionUid,
        const QDir& oldRootDir,
        const QDir& newRootDir) {
    return newWritingJsonWebTask<RelocateCollectedTracksTask>(
            m_networkAccessManager,
            m_baseUrl,
            collectionUid,
            oldRootDir,
            newRootDir);
}

ResolveCollectedTracksTask* Gateway::resolveTracksByUrl(
        const QString& collectionUid,
        QList<QUrl> trackUrls) {
    return newReadingNetworkTask<ResolveCollectedTracksTask>(
            m_networkAccessManager,
            m_baseUrl,
            collectionUid,
            std::move(trackUrls));
}

ListCollectedPlaylistsTask* Gateway::listCollectedPlaylists(
        const QString& collectionUid,
        const QString& kind) {
    return newReadingNetworkTask<ListCollectedPlaylistsTask>(
            m_networkAccessManager,
            m_baseUrl,
            collectionUid,
            kind);
}

ListTagsTask* Gateway::listTags(
        const QString& collectionUid,
        const std::optional<QStringList>& facets,
        const Pagination& pagination) {
    return newReadingNetworkTask<ListTagsTask>(
            m_networkAccessManager,
            m_baseUrl,
            collectionUid,
            facets,
            pagination);
}

ListTagsFacetsTask* Gateway::listTagsFacets(
        const QString& collectionUid,
        const std::optional<QStringList>& facets,
        const Pagination& pagination) {
    return newReadingNetworkTask<ListTagsFacetsTask>(
            m_networkAccessManager,
            m_baseUrl,
            collectionUid,
            facets,
            pagination);
}

CreateCollectedPlaylistTask* Gateway::createPlaylist(
        const QString& collectionUid,
        const json::Playlist& playlist) {
    return newWritingJsonWebTask<CreateCollectedPlaylistTask>(
            m_networkAccessManager,
            m_baseUrl,
            collectionUid,
            playlist);
}

DeletePlaylistTask* Gateway::deletePlaylist(
        QString playlistUid) {
    return newWritingJsonWebTask<DeletePlaylistTask>(
            m_networkAccessManager,
            m_baseUrl,
            std::move(playlistUid));
}

PlaylistAppendTrackEntriesByUrlTask* Gateway::playlistAppendTrackEntriesByUrl(
        QString collectionUid,
        json::EntityHeader playlistEntityHeader,
        QList<QUrl> trackUrls) {
    auto* const pNetworkTask = newWritingNetworkTask<PlaylistAppendTrackEntriesByUrlTask>(
            m_networkAccessManager,
            m_baseUrl,
            std::move(collectionUid),
            std::move(playlistEntityHeader),
            std::move(trackUrls));
    // The composite task PlaylistAppendTrackEntriesByUrlTask IS NOT
    // a mixxx::network::JsonWebTask and sends custom failed() and
    // networkError() signals (though with identical signature) that
    // need to be connected separately. Otherwise the gateway won't
    // notice that the task was finished.
    // TODO: Find a better way to propagate JsonWebTask signals
    // from component tasks to composite tasks without requiring to
    // inherit from JsonWebTask.
    connect(pNetworkTask,
            &PlaylistAppendTrackEntriesByUrlTask::failed,
            this,
            [this](const mixxx::network::JsonWebResponse& response) {
                auto* const pNetworkTask =
                        qobject_cast<PlaylistAppendTrackEntriesByUrlTask*>(sender());
                VERIFY_OR_DEBUG_ASSERT(pNetworkTask) {
                    return;
                }
                kLogger.warning()
                        << pNetworkTask
                        << "Failed to append playlist track entries by URL"
                        << response;
                pNetworkTask->deleteLater();
            });
    connect(pNetworkTask,
            &PlaylistAppendTrackEntriesByUrlTask::networkError,
            this,
            [this](QNetworkReply::NetworkError errorCode,
                    const QString& errorString,
                    const mixxx::network::WebResponseWithContent& responseWithContent) {
                auto* const pNetworkTask =
                        qobject_cast<PlaylistAppendTrackEntriesByUrlTask*>(sender());
                VERIFY_OR_DEBUG_ASSERT(pNetworkTask) {
                    return;
                }
                kLogger.warning()
                        << pNetworkTask
                        << "Failed to append playlist track entries by URL"
                        << errorCode
                        << errorString
                        << responseWithContent;
                pNetworkTask->deleteLater();
            });
    return pNetworkTask;
}

} // namespace aoide
