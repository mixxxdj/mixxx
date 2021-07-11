#include "aoide/web/deleteplaylisttask.h"

#include <QMetaMethod>

#include "util/logger.h"

namespace {

const mixxx::Logger kLogger("aoide DeletePlaylistTask");

} // anonymous namespace

namespace aoide {

namespace {

mixxx::network::JsonWebRequest buildRequest(
        const QString& playlistUid) {
    return mixxx::network::JsonWebRequest{
            mixxx::network::HttpRequestMethod::Delete,
            QStringLiteral("/p/") + playlistUid,
            QUrlQuery(),
            QJsonDocument()};
}

} // anonymous namespace

DeletePlaylistTask::DeletePlaylistTask(
        QNetworkAccessManager* networkAccessManager,
        QUrl baseUrl,
        QString playlistUid)
        : mixxx::network::JsonWebTask(
                  networkAccessManager,
                  std::move(baseUrl),
                  buildRequest(playlistUid)),
          m_playlistUid(std::move(playlistUid)) {
}

void DeletePlaylistTask::onFinished(
        const mixxx::network::JsonWebResponse& jsonResponse) {
    if (!jsonResponse.isStatusCodeSuccess()) {
        kLogger.warning()
                << "Request failed with HTTP status code"
                << jsonResponse.statusCode();
        emitFailed(jsonResponse);
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(jsonResponse.statusCode() == mixxx::network::kHttpStatusCodeNoContent) {
        kLogger.warning()
                << "Unexpected HTTP status code"
                << jsonResponse.statusCode();
        emitFailed(jsonResponse);
        return;
    }

    VERIFY_OR_DEBUG_ASSERT(jsonResponse.content().isEmpty()) {
        kLogger.warning()
                << "Invalid JSON content"
                << jsonResponse.content();
        emitFailed(jsonResponse);
        return;
    }
    emitSucceeded();
}

void DeletePlaylistTask::emitSucceeded() {
    VERIFY_OR_DEBUG_ASSERT(
            isSignalFuncConnected(&DeletePlaylistTask::succeeded)) {
        kLogger.warning()
                << "Unhandled succeeded signal";
        deleteLater();
        return;
    }
    emit succeeded(m_playlistUid);
}

} // namespace aoide
