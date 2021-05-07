#include "aoide/web/createcollectedplaylisttask.h"

#include <QMetaMethod>

#include "util/logger.h"

namespace {

const mixxx::Logger kLogger("aoide CreateCollectedPlaylistTask");

} // anonymous namespace

namespace aoide {

namespace {

mixxx::network::JsonWebRequest buildRequest(
        const QString& collectionUid,
        const json::Playlist& playlist) {
    return mixxx::network::JsonWebRequest{
            mixxx::network::HttpRequestMethod::Post,
            QStringLiteral("/c/") +
                    collectionUid +
                    QStringLiteral("/p"),
            QUrlQuery(),
            QJsonDocument(playlist)};
}

} // anonymous namespace

CreateCollectedPlaylistTask::CreateCollectedPlaylistTask(
        QNetworkAccessManager* networkAccessManager,
        QUrl baseUrl,
        const QString& collectionUid,
        const json::Playlist& playlist)
        : mixxx::network::JsonWebTask(
                  networkAccessManager,
                  std::move(baseUrl),
                  buildRequest(collectionUid, playlist)) {
}

void CreateCollectedPlaylistTask::onFinished(
        const mixxx::network::JsonWebResponse& jsonResponse) {
    if (!jsonResponse.isStatusCodeSuccess()) {
        kLogger.warning()
                << "Request failed with HTTP status code"
                << jsonResponse.statusCode();
        emitFailed(jsonResponse);
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(jsonResponse.statusCode() == mixxx::network::kHttpStatusCodeCreated) {
        kLogger.warning()
                << "Unexpected HTTP status code"
                << jsonResponse.statusCode();
        emitFailed(jsonResponse);
        return;
    }

    VERIFY_OR_DEBUG_ASSERT(jsonResponse.content().isArray()) {
        kLogger.warning()
                << "Invalid JSON content"
                << jsonResponse.content();
        emitFailed(jsonResponse);
        return;
    }
    emitSucceeded(
            json::PlaylistEntity(
                    jsonResponse.content().array()));
}

void CreateCollectedPlaylistTask::emitSucceeded(
        const json::PlaylistEntity& playlistEntity) {
    VERIFY_OR_DEBUG_ASSERT(
            isSignalFuncConnected(&CreateCollectedPlaylistTask::succeeded)) {
        kLogger.warning()
                << "Unhandled succeeded signal";
        deleteLater();
        return;
    }
    emit succeeded(playlistEntity);
}

} // namespace aoide
