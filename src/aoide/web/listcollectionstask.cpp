#include "aoide/web/listcollectionstask.h"

#include <QMetaMethod>

#include "util/logger.h"

namespace {

const mixxx::Logger kLogger("aoide ListCollectionsTask");

} // anonymous namespace

namespace aoide {

namespace {

mixxx::network::JsonWebRequest buildRequest(
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
            QStringLiteral("/c"),
            std::move(query),
            QJsonDocument()};
}

} // anonymous namespace

ListCollectionsTask::ListCollectionsTask(
        QNetworkAccessManager* networkAccessManager,
        QUrl baseUrl,
        const QString& kind,
        const Pagination& pagination)
        : mixxx::network::JsonWebTask(
                  networkAccessManager,
                  std::move(baseUrl),
                  buildRequest(kind, pagination)) {
}

void ListCollectionsTask::onFinished(
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

    QVector<json::CollectionEntity> result;
    result.reserve(jsonArray.size());
    for (const auto& jsonValue : jsonArray) {
        DEBUG_ASSERT(jsonValue.isArray());
        result.append(
                json::CollectionEntity(
                        jsonValue.toArray()));
    }

    emitSucceeded(result);
}

void ListCollectionsTask::emitSucceeded(
        const QVector<aoide::json::CollectionEntity>& result) {
    const auto signal = QMetaMethod::fromSignal(
            &ListCollectionsTask::succeeded);
    DEBUG_ASSERT(receivers(signal.name()) <= 1); // unique connection
    if (isSignalConnected(signal)) {
        emit succeeded(
                std::move(result));
    } else {
        deleteLater();
    }
}

} // namespace aoide
