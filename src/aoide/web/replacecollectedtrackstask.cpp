#include "aoide/web/replacecollectedtrackstask.h"

#include <QMetaMethod>

#include "util/logger.h"

namespace {

const mixxx::Logger kLogger("aoide ReplaceCollectedTracksTask");

} // anonymous namespace

namespace aoide {

namespace {

mixxx::network::JsonWebRequest buildRequest(
        const QString& collectionUid,
        const QList<json::Track>& tracks) {
    QUrlQuery query;
    query.addQueryItem(
            QStringLiteral("resolvePathFromUrl"),
            QStringLiteral("true"));
    QJsonArray jsonTracks;
    for (auto track : tracks) {
        jsonTracks += track.intoQJsonValue();
    }
    return mixxx::network::JsonWebRequest{
            mixxx::network::HttpRequestMethod::Post,
            QStringLiteral("/c/") +
                    collectionUid +
                    QStringLiteral("/t/replace"),
            query,
            QJsonDocument(jsonTracks)};
}

} // anonymous namespace

ReplaceCollectedTracksTask::ReplaceCollectedTracksTask(
        QNetworkAccessManager* networkAccessManager,
        QUrl baseUrl,
        const QString& collectionUid,
        const QList<json::Track>& tracks)
        : mixxx::network::JsonWebTask(
                  networkAccessManager,
                  std::move(baseUrl),
                  buildRequest(collectionUid, tracks)),
          m_size(tracks.size()) {
}

void ReplaceCollectedTracksTask::onFinished(
        const mixxx::network::JsonWebResponse& jsonResponse) {
    if (!jsonResponse.isStatusCodeSuccess()) {
        kLogger.warning()
                << "Request failed with HTTP status code"
                << jsonResponse.statusCode();
        emitFailed(jsonResponse);
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(jsonResponse.statusCode() == mixxx::network::kHttpStatusCodeOk) {
        kLogger.warning()
                << "Unexpected HTTP status code"
                << jsonResponse.statusCode();
        emitFailed(jsonResponse);
        return;
    }

    VERIFY_OR_DEBUG_ASSERT(jsonResponse.content().isObject()) {
        kLogger.warning()
                << "Invalid JSON content"
                << jsonResponse.content();
        emitFailed(jsonResponse);
        return;
    }
    emitSucceeded(jsonResponse.content().object());
}

void ReplaceCollectedTracksTask::emitSucceeded(
        const QJsonObject& result) {
    const auto signal = QMetaMethod::fromSignal(
            &ReplaceCollectedTracksTask::succeeded);
    DEBUG_ASSERT(receivers(signal.name()) <= 1); // unique connection
    if (isSignalConnected(signal)) {
        emit succeeded(
                std::move(result));
    } else {
        deleteLater();
    }
}

} // namespace aoide
