#include "aoide/web/listcollectedplayliststask.h"

#include <QMetaMethod>

#include "util/logger.h"

namespace {

const mixxx::Logger kLogger("aoide ListCollectedPlaylistsTask");

} // anonymous namespace

namespace aoide {

namespace {

mixxx::network::JsonWebRequest buildRequest(
        const QString& collectionUid,
        const QString& kind,
        const Pagination& pagination) {
    QUrlQuery query;
    if (!kind.isEmpty()) {
        query.addQueryItem(
                QStringLiteral("kind"),
                kind);
    }
    pagination.addToQuery(&query);
    return mixxx::network::JsonWebRequest{
            mixxx::network::HttpRequestMethod::Get,
            QStringLiteral("/c/") +
                    collectionUid +
                    QStringLiteral("/p"),
            query,
            QJsonDocument()};
}

} // anonymous namespace

ListCollectedPlaylistsTask::ListCollectedPlaylistsTask(
        QNetworkAccessManager* networkAccessManager,
        QUrl baseUrl,
        const QString& collectionUid,
        const QString& kind,
        const Pagination& pagination)
        : mixxx::network::JsonWebTask(
                  networkAccessManager,
                  std::move(baseUrl),
                  buildRequest(collectionUid, kind, pagination)) {
}

void ListCollectedPlaylistsTask::onFinished(
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

    VERIFY_OR_DEBUG_ASSERT(jsonResponse.content().isArray()) {
        kLogger.warning()
                << "Invalid JSON content"
                << jsonResponse.content();
        emitFailed(jsonResponse);
        return;
    }
    const auto jsonArray = jsonResponse.content().array();

    QVector<json::PlaylistWithEntriesSummaryEntity> result;
    result.reserve(jsonArray.size());
    for (const auto& jsonValue : jsonArray) {
        DEBUG_ASSERT(jsonValue.isArray());
        result.append(
                json::PlaylistWithEntriesSummaryEntity(
                        jsonValue.toArray()));
    }

    emitSucceeded(result);
}

void ListCollectedPlaylistsTask::emitSucceeded(
        const QVector<aoide::json::PlaylistWithEntriesSummaryEntity>& result) {
    const auto signal = QMetaMethod::fromSignal(
            &ListCollectedPlaylistsTask::succeeded);
    DEBUG_ASSERT(receivers(signal.name()) <= 1); // unique connection
    if (isSignalConnected(signal)) {
        emit succeeded(
                std::move(result));
    } else {
        deleteLater();
    }
}

} // namespace aoide
