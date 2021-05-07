#include "aoide/web/createcollectiontask.h"

#include <QMetaMethod>

#include "util/logger.h"

namespace {

const mixxx::Logger kLogger("aoide CreateCollectionTask");

} // anonymous namespace

namespace aoide {

namespace {

mixxx::network::JsonWebRequest buildRequest(
        const json::Collection& collection) {
    return mixxx::network::JsonWebRequest{
            mixxx::network::HttpRequestMethod::Post,
            QStringLiteral("/c"),
            QUrlQuery(),
            QJsonDocument(collection)};
}

} // anonymous namespace

CreateCollectionTask::CreateCollectionTask(
        QNetworkAccessManager* networkAccessManager,
        QUrl baseUrl,
        const json::Collection& collection)
        : mixxx::network::JsonWebTask(
                  networkAccessManager,
                  std::move(baseUrl),
                  buildRequest(collection)) {
}

void CreateCollectionTask::onFinished(
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
            json::CollectionEntity(jsonResponse.content().array()));
}

void CreateCollectionTask::emitSucceeded(
        const json::CollectionEntity& collectionEntity) {
    VERIFY_OR_DEBUG_ASSERT(
            isSignalFuncConnected(&CreateCollectionTask::succeeded)) {
        kLogger.warning()
                << "Unhandled succeeded signal";
        deleteLater();
        return;
    }
    emit succeeded(collectionEntity);
}

} // namespace aoide
