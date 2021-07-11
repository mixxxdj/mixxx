#include "aoide/web/resolvecollectedtrackstask.h"

#include <QJsonArray>
#include <QMetaMethod>

#include "util/encodedurl.h"
#include "util/logger.h"

namespace {

const mixxx::Logger kLogger("aoide ResolveCollectedTracksTask");

} // anonymous namespace

namespace aoide {

namespace {

mixxx::network::JsonWebRequest buildRequest(
        const QString& collectionUid,
        const QList<QUrl> trackUrls) {
    QUrlQuery query;
    QJsonArray encodedTrackUrls;
    for (const auto& trackUrl : trackUrls) {
        encodedTrackUrls.append(
                mixxx::EncodedUrl::fromQUrl(trackUrl).toQString());
    }
    return mixxx::network::JsonWebRequest{
            mixxx::network::HttpRequestMethod::Post,
            QStringLiteral("/c/") +
                    collectionUid +
                    QStringLiteral("/t/resolve"),
            query,
            QJsonDocument(encodedTrackUrls)};
}

} // anonymous namespace

ResolveCollectedTracksTask::ResolveCollectedTracksTask(
        QNetworkAccessManager* networkAccessManager,
        QUrl baseUrl,
        const QString& collectionUid,
        QList<QUrl> trackUrls)
        : mixxx::network::JsonWebTask(
                  networkAccessManager,
                  std::move(baseUrl),
                  buildRequest(collectionUid, trackUrls)),
          m_unresolvedTrackUrls(std::move(trackUrls)) {
}

void ResolveCollectedTracksTask::onFinished(
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

    QMap<QUrl, json::EntityUid> resolvedTrackUrls;
    for (const auto& elem : jsonArray) {
        VERIFY_OR_DEBUG_ASSERT(elem.isArray()) {
            kLogger.warning()
                    << "Invalid JSON content"
                    << jsonResponse.content();
            emitFailed(jsonResponse);
            return;
        }
        auto pair = elem.toArray();
        VERIFY_OR_DEBUG_ASSERT(pair.size() == 2) {
            kLogger.warning()
                    << "Invalid JSON content"
                    << jsonResponse.content();
            emitFailed(jsonResponse);
            return;
        }
        auto url = mixxx::EncodedUrl::fromEncodedQByteArray(
                pair[0].toString().toUtf8())
                           .toQUrl();
        VERIFY_OR_DEBUG_ASSERT(url.isValid()) {
            kLogger.warning()
                    << "Invalid encoded URL"
                    << pair[0].toString();
            emitFailed(jsonResponse);
            return;
        }
        const auto entityHeader = json::EntityHeader(pair[1].toArray());
        DEBUG_ASSERT(!resolvedTrackUrls.contains(url));
        DEBUG_ASSERT(entityHeader.uid().isValid());
        resolvedTrackUrls.insert(url, entityHeader.uid());
    }

    QList<QUrl> unresolvedTrackUrls;
    DEBUG_ASSERT(resolvedTrackUrls.size() <= m_unresolvedTrackUrls.size());
    unresolvedTrackUrls.reserve(
            m_unresolvedTrackUrls.size() -
            resolvedTrackUrls.size());
    for (const auto& url : m_unresolvedTrackUrls) {
        if (!resolvedTrackUrls.contains(url)) {
            unresolvedTrackUrls.append(url);
        }
    }

    emitSucceeded(
            resolvedTrackUrls,
            unresolvedTrackUrls);
}

void ResolveCollectedTracksTask::emitSucceeded(
        const QMap<QUrl, json::EntityUid>& resolvedTrackUrls,
        const QList<QUrl>& unresolvedTrackUrls) {
    const auto signal = QMetaMethod::fromSignal(
            &ResolveCollectedTracksTask::succeeded);
    DEBUG_ASSERT(receivers(signal.name()) <= 1); // unique connection
    if (isSignalConnected(signal)) {
        emit succeeded(
                resolvedTrackUrls,
                unresolvedTrackUrls);
    } else {
        deleteLater();
    }
}

} // namespace aoide
