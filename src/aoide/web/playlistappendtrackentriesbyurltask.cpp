#include "aoide/web/playlistappendtrackentriesbyurltask.h"

#include <QMetaMethod>

#include "util/logger.h"
#include "util/thread_affinity.h"

namespace {

const mixxx::Logger kLogger("aoide PlaylistAppendTrackEntriesByUrlTask");

} // anonymous namespace

namespace aoide {

PlaylistAppendTrackEntriesByUrlTask::PlaylistAppendTrackEntriesByUrlTask(
        QNetworkAccessManager* networkAccessManager,
        QUrl baseUrl,
        QString collectionUid,
        json::EntityHeader playlistEntityHeader,
        QList<QUrl> trackUrls)
        : mixxx::network::NetworkTask(networkAccessManager),
          m_baseUrl(std::move(baseUrl)),
          m_collectionUid(std::move(collectionUid)),
          m_timeoutMillis(kNoTimeout),
          m_playlistEntityHeader(std::move(playlistEntityHeader)),
          m_unresolvedTrackUrls(std::move(trackUrls)) {
    DEBUG_ASSERT(m_baseUrl.isValid());
}

void PlaylistAppendTrackEntriesByUrlTask::startResolveCollectedTracksTask() {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    DEBUG_ASSERT(!m_resolveTracksByUrlTask);
    m_resolveTracksByUrlTask =
            new ResolveCollectedTracksTask(
                    m_networkAccessManagerWeakPtr.data(),
                    m_baseUrl,
                    m_collectionUid,
                    m_unresolvedTrackUrls);
    connect(m_resolveTracksByUrlTask,
            &mixxx::network::NetworkTask::aborted,
            this,
            &PlaylistAppendTrackEntriesByUrlTask::slotSubtaskAborted);
    connect(m_resolveTracksByUrlTask,
            &mixxx::network::WebTask::networkError,
            this,
            &PlaylistAppendTrackEntriesByUrlTask::slotSubtaskNetworkError);
    connect(m_resolveTracksByUrlTask,
            &ResolveCollectedTracksTask::failed,
            this,
            &PlaylistAppendTrackEntriesByUrlTask::failed,
            /*signal-to-signal*/ Qt::DirectConnection);
    connect(m_resolveTracksByUrlTask,
            &ResolveCollectedTracksTask::succeeded,
            this,
            &PlaylistAppendTrackEntriesByUrlTask::slotResolveTracksByUrlSucceeded);

    m_resolveTracksByUrlTask->slotStart(m_timeoutMillis);
}

void PlaylistAppendTrackEntriesByUrlTask::startPlaylistAppendTrackEntriesTask(
        const QList<json::EntityUid>& trackUids) {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    DEBUG_ASSERT(!m_playlistAppendTrackEntriesTask);
    m_playlistAppendTrackEntriesTask =
            new PlaylistAppendTrackEntriesTask(
                    m_networkAccessManagerWeakPtr.data(),
                    m_baseUrl,
                    m_playlistEntityHeader,
                    trackUids);
    connect(m_playlistAppendTrackEntriesTask,
            &mixxx::network::NetworkTask::aborted,
            this,
            &PlaylistAppendTrackEntriesByUrlTask::slotSubtaskAborted);
    connect(m_playlistAppendTrackEntriesTask,
            &mixxx::network::WebTask::networkError,
            this,
            &PlaylistAppendTrackEntriesByUrlTask::slotSubtaskNetworkError);
    connect(m_playlistAppendTrackEntriesTask,
            &PlaylistAppendTrackEntriesTask::failed,
            this,
            &PlaylistAppendTrackEntriesByUrlTask::failed,
            /*signal-to-signal*/ Qt::DirectConnection);
    connect(m_playlistAppendTrackEntriesTask,
            &PlaylistAppendTrackEntriesTask::succeeded,
            this,
            &PlaylistAppendTrackEntriesByUrlTask::slotPlaylistAddTracksSucceeded);
    m_playlistAppendTrackEntriesTask->slotStart(m_timeoutMillis);
}

void PlaylistAppendTrackEntriesByUrlTask::slotStart(
        int timeoutMillis) {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);

    // Save invocation parameters for starting subtasks
    m_timeoutMillis = timeoutMillis;

    // Start 1st subtask
    startResolveCollectedTracksTask();
}

void PlaylistAppendTrackEntriesByUrlTask::slotAbort() {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);

    // Try to abort child tasks in reverse order
    if (m_playlistAppendTrackEntriesTask) {
        abortThis(m_playlistAppendTrackEntriesTask);
    }
    if (m_resolveTracksByUrlTask) {
        abortThis(m_resolveTracksByUrlTask);
    }
}

void PlaylistAppendTrackEntriesByUrlTask::reset() {
    // Reset child tasks in reverse order
    if (m_playlistAppendTrackEntriesTask) {
        m_playlistAppendTrackEntriesTask->disconnect(this);
        m_playlistAppendTrackEntriesTask->deleteLater();
        m_playlistAppendTrackEntriesTask.clear();
    }
    if (m_resolveTracksByUrlTask) {
        // Prevent emitting duplicate signals
        m_resolveTracksByUrlTask->disconnect(this);
        m_resolveTracksByUrlTask->deleteLater();
        m_resolveTracksByUrlTask.clear();
    }
    m_unresolvedTrackUrls.clear();
}

void PlaylistAppendTrackEntriesByUrlTask::slotSubtaskFailed(
        const mixxx::network::JsonWebResponse& response) {
    reset();
    emitFailed(response);
}

void PlaylistAppendTrackEntriesByUrlTask::slotSubtaskAborted(
        const QUrl& requestUrl) {
    reset();
    emitAborted(requestUrl);
}

void PlaylistAppendTrackEntriesByUrlTask::slotSubtaskNetworkError(
        QNetworkReply::NetworkError errorCode,
        const QString& errorString,
        const mixxx::network::WebResponseWithContent& responseWithContent) {
    reset();
    emitNetworkError(
            errorCode,
            errorString,
            responseWithContent);
}

void PlaylistAppendTrackEntriesByUrlTask::slotResolveTracksByUrlSucceeded(
        const QMap<QUrl, json::EntityUid>& resolvedTrackUrls,
        const QList<QUrl>& unresolvedTrackUrls) {
    VERIFY_OR_DEBUG_ASSERT(m_resolveTracksByUrlTask) {
        return;
    }
    m_resolveTracksByUrlTask->deleteLater();
    m_resolveTracksByUrlTask.clear();

    // Save intermediate results for later
    m_unresolvedTrackUrls = std::move(unresolvedTrackUrls);

    // Start 2nd subtask
    startPlaylistAppendTrackEntriesTask(resolvedTrackUrls.values());
}

void PlaylistAppendTrackEntriesByUrlTask::slotPlaylistAddTracksSucceeded(
        const json::PlaylistWithEntriesSummaryEntity& playlistEntity) {
    VERIFY_OR_DEBUG_ASSERT(m_playlistAppendTrackEntriesTask) {
        return;
    }
    m_playlistAppendTrackEntriesTask->deleteLater();
    m_playlistAppendTrackEntriesTask.clear();

    reset();
    emitSucceeded(std::move(playlistEntity));
}

void PlaylistAppendTrackEntriesByUrlTask::emitFailed(
        const mixxx::network::JsonWebResponse& response) {
    VERIFY_OR_DEBUG_ASSERT(
            isSignalFuncConnected(&PlaylistAppendTrackEntriesByUrlTask::failed)) {
        kLogger.warning()
                << this
                << "Unhandled failed signal"
                << response;
        deleteLater();
        return;
    }
    emit failed(response);
}

void PlaylistAppendTrackEntriesByUrlTask::emitNetworkError(
        QNetworkReply::NetworkError errorCode,
        const QString& errorString,
        const mixxx::network::WebResponseWithContent& responseWithContent) {
    VERIFY_OR_DEBUG_ASSERT(
            isSignalFuncConnected(&PlaylistAppendTrackEntriesByUrlTask::failed)) {
        kLogger.warning()
                << this
                << "Unhandled network error signal"
                << errorCode
                << errorString
                << responseWithContent;
        deleteLater();
        return;
    }
    emit networkError(errorCode, errorString, responseWithContent);
}

void PlaylistAppendTrackEntriesByUrlTask::emitSucceeded(
        const json::PlaylistWithEntriesSummaryEntity& playlistEntity) {
    DEBUG_ASSERT(playlistEntity.header().uid() ==
            m_playlistEntityHeader.uid());
    const auto signal = QMetaMethod::fromSignal(
            &PlaylistAppendTrackEntriesByUrlTask::succeeded);
    DEBUG_ASSERT(receivers(signal.name()) <= 1); // unique connection
    auto unresolvedTrackUrls = std::move(m_unresolvedTrackUrls);
    if (isSignalConnected(signal)) {
        emit succeeded(
                std::move(playlistEntity),
                std::move(unresolvedTrackUrls));
    } else {
        deleteLater();
    }
}

} // namespace aoide
