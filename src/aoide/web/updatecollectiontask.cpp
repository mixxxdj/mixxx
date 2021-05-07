#include "aoide/web/updatecollectiontask.h"

#include <QMetaMethod>

#include "util/logger.h"

namespace {

const mixxx::Logger kLogger("aoide UpdateCollectionTask");

} // anonymous namespace

namespace aoide {

namespace {

mixxx::network::JsonWebRequest buildRequest(
        const json::CollectionEntity& collectionEntity) {
    return mixxx::network::JsonWebRequest{
            mixxx::network::HttpRequestMethod::Put,
            QStringLiteral("/c/") + collectionEntity.header().uid().value(),
            QUrlQuery(),
            QJsonDocument(collectionEntity)};
}

} // anonymous namespace

UpdateCollectionTask::UpdateCollectionTask(
        QNetworkAccessManager* networkAccessManager,
        QUrl baseUrl,
        const json::CollectionEntity& collectionEntity)
        : mixxx::network::JsonWebTask(
                  networkAccessManager,
                  std::move(baseUrl),
                  buildRequest(collectionEntity)) {
}

void UpdateCollectionTask::onFinished(
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
    emitSucceeded(
            json::CollectionEntity(jsonResponse.content().array()));
}

void UpdateCollectionTask::emitSucceeded(
        const json::CollectionEntity& collectionEntity) {
    const auto signal = QMetaMethod::fromSignal(
            &UpdateCollectionTask::succeeded);
    DEBUG_ASSERT(receivers(signal.name()) <= 1); // unique connection
    if (isSignalConnected(signal)) {
        emit succeeded(
                std::move(collectionEntity));
    } else {
        deleteLater();
    }
}

} // namespace aoide
