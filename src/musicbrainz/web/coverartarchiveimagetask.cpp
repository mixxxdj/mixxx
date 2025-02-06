#include "musicbrainz/web/coverartarchiveimagetask.h"

#include "moc_coverartarchiveimagetask.cpp"
#include "network/httpstatuscode.h"
#include "util/assert.h"
#include "util/logger.h"
#include "util/thread_affinity.h"

namespace mixxx {

namespace {

const Logger kLogger("CoverArtArchiveImageTask");

QNetworkRequest createNetworkRequest(const QString& coverArtUrl) {
    QUrl url = coverArtUrl;
    DEBUG_ASSERT(url.isValid());
    QNetworkRequest networkRequest(url);
    return networkRequest;
}

} // anonymous namespace

CoverArtArchiveImageTask::CoverArtArchiveImageTask(
        QNetworkAccessManager* pNetworkAccessManager,
        const QString& coverArtLink,
        const QUuid& albumReleaseId,
        QObject* pParent)
        : network::WebTask(
                  pNetworkAccessManager,
                  pParent),
          m_coverArtUrl(coverArtLink),
          m_albumReleaseId(albumReleaseId) {
}

QNetworkReply* CoverArtArchiveImageTask::doStartNetworkRequest(
        QNetworkAccessManager* pNetworkAccessManager,
        int parentTimeoutMillis) {
    Q_UNUSED(parentTimeoutMillis);
    pNetworkAccessManager->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    DEBUG_ASSERT(pNetworkAccessManager);

    const QNetworkRequest networkRequest =
            createNetworkRequest(m_coverArtUrl);

    if (kLogger.traceEnabled()) {
        kLogger.trace()
                << "GET"
                << networkRequest.url();
    }
    return pNetworkAccessManager->get(networkRequest);
}

void CoverArtArchiveImageTask::doNetworkReplyFinished(
        QNetworkReply* pFinishedNetworkReply,
        network::HttpStatusCode statusCode) {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);

    const QByteArray resultImageBytes = pFinishedNetworkReply->readAll();

    if (statusCode != 200) {
        kLogger.info()
                << "GET reply"
                << "statusCode:" << statusCode;
        emitFailed(
                network::WebResponse(
                        pFinishedNetworkReply->url(),
                        pFinishedNetworkReply->request().url(),
                        statusCode),
                statusCode,
                QStringLiteral("Failed to get Image"));
        return;
    }

    emitSucceeded(resultImageBytes);
}

void CoverArtArchiveImageTask::emitSucceeded(
        const QByteArray& coverArtImageBytes) {
    VERIFY_OR_DEBUG_ASSERT(
            isSignalFuncConnected(&CoverArtArchiveImageTask::succeeded)) {
        kLogger.warning()
                << "Unhandled succeeded signal";
        deleteLater();
        return;
    }
    emit succeeded(m_albumReleaseId, coverArtImageBytes);
}

void CoverArtArchiveImageTask::emitFailed(
        const network::WebResponse& response,
        int errorCode,
        const QString& errorMessage) {
    VERIFY_OR_DEBUG_ASSERT(
            isSignalFuncConnected(&CoverArtArchiveImageTask::failed)) {
        kLogger.warning()
                << "Unhandled failed signal"
                << response
                << errorCode
                << errorMessage;
        deleteLater();
        return;
    }
    emit failed(
            response,
            errorCode,
            errorMessage);
}

} // namespace mixxx
