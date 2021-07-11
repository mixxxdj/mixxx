#include "aoide/web/listtagsfacetstask.h"

#include <QMetaMethod>

#include "util/logger.h"

namespace {

const mixxx::Logger kLogger("aoide ListTagsFacetsTask");

} // anonymous namespace

namespace aoide {

namespace {

mixxx::network::JsonWebRequest buildRequest(
        const QString& collectionUid,
        const std::optional<QStringList>& facets,
        const Pagination& pagination) {
    QUrlQuery query;
    if (!collectionUid.isEmpty()) {
        query.addQueryItem(QStringLiteral("collectionUid"), collectionUid);
    }
    if (facets) {
        query.addQueryItem(QStringLiteral("facets"), facets->join(','));
    }
    pagination.addToQuery(&query);
    return mixxx::network::JsonWebRequest{
            mixxx::network::HttpRequestMethod::Get,
            QStringLiteral("/tags/facets"),
            std::move(query),
            QJsonDocument()};
}

} // anonymous namespace

ListTagsFacetsTask::ListTagsFacetsTask(
        QNetworkAccessManager* networkAccessManager,
        QUrl baseUrl,
        const QString& collectionUid,
        const std::optional<QStringList>& facets,
        const Pagination& pagination)
        : mixxx::network::JsonWebTask(
                  networkAccessManager,
                  std::move(baseUrl),
                  buildRequest(collectionUid, facets, pagination)) {
}

void ListTagsFacetsTask::onFinished(
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

    QVector<json::TagFacetCount> result;
    result.reserve(jsonArray.size());
    for (const auto& jsonValue : jsonArray) {
        DEBUG_ASSERT(jsonValue.isArray());
        result.append(
                json::TagFacetCount(
                        jsonValue.toArray()));
    }

    emitSucceeded(result);
}

void ListTagsFacetsTask::emitSucceeded(
        const QVector<aoide::json::TagFacetCount>& result) {
    const auto signal = QMetaMethod::fromSignal(
            &ListTagsFacetsTask::succeeded);
    DEBUG_ASSERT(receivers(signal.name()) <= 1); // unique connection
    if (isSignalConnected(signal)) {
        emit succeeded(
                std::move(result));
    } else {
        deleteLater();
    }
}

} // namespace aoide
