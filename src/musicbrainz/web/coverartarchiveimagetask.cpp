#include "musicbrainz/web/coverartarchiveimagetask.h"

#include <QMetaMethod>

#include "defs_urls.h"
#include "network/httpstatuscode.h"
#include "util/assert.h"
#include "util/logger.h"
#include "util/thread_affinity.h"
#include "util/versionstore.h"

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
        QObject* pParent)
        : network::WebTask(
                  pNetworkAccessManager,
                  pParent),
          m_coverArtUrl(coverArtLink) {
}

QNetworkReply* CoverArtArchiveImageTask::doStartNetworkRequest(
        QNetworkAccessManager* pNetworkAccessManager,
        int parentTimeoutMillis) {
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
        return;
    }

    emit succeeded(resultImageBytes);
}

} // namespace mixxx
