#include "musicbrainz/web/coverartarchivelinkstask.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkRequest>

#include "moc_coverartarchivelinkstask.cpp"
#include "network/httpstatuscode.h"
#include "util/assert.h"
#include "util/logger.h"

namespace mixxx {

namespace {

const Logger kLogger("CoverArtArchiveLinksTask");

const QUrl kBaseUrl = QStringLiteral("https://coverartarchive.org/");

const QString kRequestPath = QStringLiteral("/release/");

network::JsonWebRequest lookupRequest() {
    return network::JsonWebRequest{
            network::HttpRequestMethod::Get,
            kRequestPath,
            QUrlQuery(),
            QJsonDocument()};
}

QNetworkRequest createNetworkRequest(
        const QUuid& releaseID) {
    DEBUG_ASSERT(kBaseUrl.isValid());
    DEBUG_ASSERT(!releaseID.isNull());
    QUrl url = kBaseUrl;
    url.setPath(kRequestPath + releaseID.toString(QUuid::WithoutBraces));
    DEBUG_ASSERT(url.isValid());
    QNetworkRequest networkRequest(url);
    return networkRequest;
}

} // anonymous namespace

CoverArtArchiveLinksTask::CoverArtArchiveLinksTask(
        QNetworkAccessManager* pNetworkAccessManager,
        const QUuid& albumReleaseId,
        QObject* pParent)
        : network::JsonWebTask(
                  pNetworkAccessManager,
                  kBaseUrl,
                  lookupRequest(),
                  pParent),
          m_albumReleaseId(albumReleaseId) {
}

QNetworkReply* CoverArtArchiveLinksTask::sendNetworkRequest(
        QNetworkAccessManager* pNetworkAccessManager,
        network::HttpRequestMethod method,
        const QUrl& url,
        const QJsonDocument& content) {
    DEBUG_ASSERT(pNetworkAccessManager);
    Q_UNUSED(method);
    Q_UNUSED(content);
    DEBUG_ASSERT(method == network::HttpRequestMethod::Get);
    pNetworkAccessManager->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);

    const QNetworkRequest networkRequest = createNetworkRequest(m_albumReleaseId);

    VERIFY_OR_DEBUG_ASSERT(url.isValid()) {
        kLogger.warning() << "Invalid URL" << url;
        return nullptr;
    }
    if (kLogger.traceEnabled()) {
        kLogger.trace()
                << "Get"
                << url;
    }
    return pNetworkAccessManager->get(networkRequest);
}

void CoverArtArchiveLinksTask::onFinished(
        const network::JsonWebResponse& response) {
    if (!response.isStatusCodeSuccess()) {
        kLogger.warning()
                << "Request failed with HTTP status code"
                << response.statusCode();
        emitFailed(response);
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(response.statusCode() == network::kHttpStatusCodeOk) {
        kLogger.warning()
                << "Unexpected HTTP status code"
                << response.statusCode();
        emitFailed(response);
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(response.content().isObject()) {
        kLogger.warning()
                << "Invalid JSON content"
                << response.content();
        emitFailed(response);
        return;
    }
    const auto jsonObject = response.content().object();

    if (jsonObject.isEmpty()) {
        kLogger.warning()
                << "Empty Json";
        emitFailed(response);
        return;
    }

    // An example JSON schema can be found for the release: 48cf6a4e-4c61-4214-afe3-ed4e11d902c9
    // JSON Schema: https://coverartarchive.org/release/48cf6a4e-4c61-4214-afe3-ed4e11d902c9
    // See also: https://wiki.musicbrainz.org/Cover_Art_Archive/API

    QList<QString> allUrls;
    DEBUG_ASSERT(jsonObject.value(QLatin1String("images")).isArray());
    const QJsonArray images = jsonObject.value(QLatin1String("images")).toArray();
    for (const auto& image : images) {
        DEBUG_ASSERT(image.isObject());
        const auto imageObject = image.toObject();

        const auto thumbnails = imageObject.value(QLatin1String("thumbnails")).toObject();
        DEBUG_ASSERT(!thumbnails.isEmpty());

        // Due to few exceptions encountered
        // For 250 PX and 500 PX cover arts, "small" and "large" keys are used.
        // See: https://coverartarchive.org/release/5240094f-9d79-44fd-9985-77c7287bcc16

        const auto smallThumbnailUrl = thumbnails.value(QLatin1String("small")).toString();
        DEBUG_ASSERT(!smallThumbnailUrl.isNull());
        allUrls.append(smallThumbnailUrl);

        const auto largeThumbnailUrl = thumbnails.value(QLatin1String("large")).toString();
        DEBUG_ASSERT(!largeThumbnailUrl.isNull());
        allUrls.append(largeThumbnailUrl);

        if (thumbnails.value(QLatin1String("1200")).toString() != nullptr) {
            const auto largestThumbnailUrl = thumbnails.value(QLatin1String("1200")).toString();
            allUrls.append(largestThumbnailUrl);
        }

        if (!(imageObject.value(QLatin1String("image")).isNull())) {
            const auto highestResolutionImageUrl =
                    imageObject.value(QLatin1String("image")).toString();
            allUrls.append(highestResolutionImageUrl);
        }

        break;
    }
    emitSucceeded(allUrls);
}

void CoverArtArchiveLinksTask::emitSucceeded(
        const QList<QString>& allUrls) {
    VERIFY_OR_DEBUG_ASSERT(
            isSignalFuncConnected(&CoverArtArchiveLinksTask::succeeded)) {
        kLogger.warning()
                << "Unhandled succeeded signal";
        deleteLater();
        return;
    }
    emit succeeded(m_albumReleaseId, allUrls);
}

} // namespace mixxx
