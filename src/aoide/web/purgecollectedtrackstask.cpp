#include "aoide/web/purgecollectedtrackstask.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QMetaMethod>

#include "util/encodedurl.h"
#include "util/fileinfo.h"
#include "util/logger.h"

namespace {

const mixxx::Logger kLogger("aoide PurgeTracksTask");

} // anonymous namespace

namespace aoide {

namespace {

mixxx::network::JsonWebRequest buildRequest(
        const QString& collectionUid,
        const QJsonArray& jsonContent) {
    QUrlQuery query;
    return mixxx::network::JsonWebRequest{
            mixxx::network::HttpRequestMethod::Post,
            QStringLiteral("/c/") +
                    collectionUid +
                    QStringLiteral("/t/purge"),
            query,
            QJsonDocument(jsonContent)};
}

mixxx::network::JsonWebRequest buildRequest(
        const QString& collectionUid,
        const QStringList& trackLocations) {
    QJsonArray content;
    for (const auto& trackLocation : trackLocations) {
        const auto trackFile = mixxx::FileInfo(QFileInfo(trackLocation));
        const auto trackUri = mixxx::EncodedUrl::fromQUrl(trackFile.toQUrl());
        content +=
                QJsonObject{
                        {
                                QLatin1String("equals"),
                                trackUri.toQString(),
                        }};
    }
    return buildRequest(
            collectionUid,
            content);
}

mixxx::network::JsonWebRequest buildRequest(
        const QString& collectionUid,
        const QDir& rootDir) {
    const auto rootUri = mixxx::EncodedUrl::fromQUrlWithTrailingSlash(
            mixxx::FileInfo(rootDir).toQUrl());
    kLogger.warning() << "TODO: Fix invalid API call";
    const auto content = QJsonArray{
            QJsonObject{
                    {
                            QLatin1String("startsWith"),
                            QJsonValue(rootUri.toQString()),
                    }}};
    return buildRequest(
            collectionUid,
            content);
}

} // anonymous namespace

PurgeTracksTask::PurgeTracksTask(
        QNetworkAccessManager* networkAccessManager,
        QUrl baseUrl,
        const QString& collectionUid,
        const QStringList& trackLocations)
        : mixxx::network::JsonWebTask(
                  networkAccessManager,
                  std::move(baseUrl),
                  buildRequest(collectionUid, trackLocations)) {
}

PurgeTracksTask::PurgeTracksTask(
        QNetworkAccessManager* networkAccessManager,
        QUrl baseUrl,
        const QString& collectionUid,
        const QDir& rootDir)
        : mixxx::network::JsonWebTask(
                  networkAccessManager,
                  std::move(baseUrl),
                  buildRequest(collectionUid, rootDir)) {
}

void PurgeTracksTask::onFinished(
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

void PurgeTracksTask::emitSucceeded() {
    const auto signal = QMetaMethod::fromSignal(
            &PurgeTracksTask::succeeded);
    DEBUG_ASSERT(receivers(signal.name()) <= 1); // unique connection
    if (isSignalConnected(signal)) {
        emit succeeded();
    } else {
        deleteLater();
    }
}

} // namespace aoide
