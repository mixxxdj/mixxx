#include "aoide/web/relocatecollectedtrackstask.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QMetaMethod>

#include "util/encodedurl.h"
#include "util/fileinfo.h"
#include "util/logger.h"

namespace {

const mixxx::Logger kLogger("aoide RelocateCollectedTracksTask");

} // anonymous namespace

namespace aoide {

namespace {

mixxx::network::JsonWebRequest buildRequest(
        const QString& collectionUid,
        const QJsonArray& content) {
    QUrlQuery query;
    if (!collectionUid.isEmpty()) {
        query.addQueryItem(
                QStringLiteral("collectionUid"),
                collectionUid);
    }
    return mixxx::network::JsonWebRequest{
            mixxx::network::HttpRequestMethod::Post,
            QStringLiteral("/t/relocate"),
            query,
            QJsonDocument(content)};
}

mixxx::network::JsonWebRequest buildRequest(
        const QString& collectionUid,
        const QList<QPair<QString, QString>>& trackRelocations) {
    QJsonArray content;
    for (const auto& trackRelocation : trackRelocations) {
        const auto oldTrackFile = mixxx::FileInfo(QDir(trackRelocation.first));
        const auto oldSourceUri = mixxx::EncodedUrl::fromQUrl(oldTrackFile.toQUrl());
        const auto newTrackFile = mixxx::FileInfo(QDir(trackRelocation.second));
        const auto newSourceUri = mixxx::EncodedUrl::fromQUrl(newTrackFile.toQUrl());
        kLogger.debug()
                << "Relocating track file:"
                << oldSourceUri
                << "->"
                << newSourceUri;
        content +=
                QJsonObject{
                        {
                                QLatin1String("predicate"),
                                QJsonObject{{QLatin1String("exact"), oldSourceUri.toQString()}},
                        },
                        {
                                QLatin1String("relocatement"),
                                newSourceUri.toQString(),
                        },
                };
    }
    return buildRequest(
            collectionUid,
            content);
}

mixxx::network::JsonWebRequest buildRequest(
        const QString& collectionUid,
        const QDir& oldRootDir,
        const QDir& newRootDir) {
    const auto oldTrackDir = mixxx::FileInfo(QDir(oldRootDir));
    const auto oldUriPrefix = mixxx::EncodedUrl::fromQUrlWithTrailingSlash(oldTrackDir.toQUrl());
    const auto newTrackDir = mixxx::FileInfo(QDir(newRootDir));
    const auto newUriPrefix = mixxx::EncodedUrl::fromQUrlWithTrailingSlash(newTrackDir.toQUrl());
    kLogger.debug()
            << "Relocating all tracks in directory:"
            << oldUriPrefix
            << "->"
            << newUriPrefix;
    const auto content = QJsonArray{QJsonObject{
            {QLatin1String("predicate"),
                    QJsonObject{
                            {QLatin1String("prefix"),
                                    QJsonValue(oldUriPrefix.toQString())},
                    }},
            {QLatin1String("replacement"),
                    QJsonValue(newUriPrefix.toQString())},
    }};
    return buildRequest(
            collectionUid,
            content);
}

} // anonymous namespace

RelocateCollectedTracksTask::RelocateCollectedTracksTask(
        QNetworkAccessManager* networkAccessManager,
        QUrl baseUrl,
        const QString& collectionUid,
        QList<QPair<QString, QString>> const& trackRelocations)
        : mixxx::network::JsonWebTask(
                  networkAccessManager,
                  std::move(baseUrl),
                  buildRequest(collectionUid, trackRelocations)) {
}

RelocateCollectedTracksTask::RelocateCollectedTracksTask(
        QNetworkAccessManager* networkAccessManager,
        QUrl baseUrl,
        const QString& collectionUid,
        const QDir& oldRootDir,
        const QDir& newRootDir)
        : mixxx::network::JsonWebTask(
                  networkAccessManager,
                  std::move(baseUrl),
                  buildRequest(collectionUid, oldRootDir, newRootDir)) {
}

void RelocateCollectedTracksTask::onFinished(
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

void RelocateCollectedTracksTask::emitSucceeded() {
    const auto signal = QMetaMethod::fromSignal(
            &RelocateCollectedTracksTask::succeeded);
    DEBUG_ASSERT(receivers(signal.name()) <= 1); // unique connection
    if (isSignalConnected(signal)) {
        emit succeeded();
    } else {
        deleteLater();
    }
}

} // namespace aoide
