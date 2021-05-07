#include "aoide/web/deletecollectiontask.h"

#include <QMetaMethod>

#include "util/logger.h"

namespace {

const mixxx::Logger kLogger("aoide DeleteCollectionTask");

} // anonymous namespace

namespace aoide {

namespace {

mixxx::network::JsonWebRequest buildRequest(
        const QString& collectionUid) {
    return mixxx::network::JsonWebRequest{
            mixxx::network::HttpRequestMethod::Delete,
            QStringLiteral("/c/") + collectionUid,
            QUrlQuery(),
            QJsonDocument()};
}

} // anonymous namespace

DeleteCollectionTask::DeleteCollectionTask(
        QNetworkAccessManager* networkAccessManager,
        QUrl baseUrl,
        QString collectionUid)
        : mixxx::network::JsonWebTask(
                  networkAccessManager,
                  std::move(baseUrl),
                  buildRequest(collectionUid)),
          m_collectionUid(std::move(collectionUid)) {
}

void DeleteCollectionTask::onFinished(
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

void DeleteCollectionTask::emitSucceeded() {
    VERIFY_OR_DEBUG_ASSERT(
            isSignalFuncConnected(&DeleteCollectionTask::succeeded)) {
        kLogger.warning()
                << "Unhandled succeeded signal";
        deleteLater();
        return;
    }
    emit succeeded(m_collectionUid);
}

} // namespace aoide
