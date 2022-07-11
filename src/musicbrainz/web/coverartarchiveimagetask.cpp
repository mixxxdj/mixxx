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
        QNetworkAccessManager* networkAccessManager,
        const QString& coverArtLink,
        QObject* parent)
        : network::WebTask(
                  networkAccessManager,
                  parent),
          m_coverArtUrl(coverArtLink) {
}

QNetworkReply* CoverArtArchiveImageTask::doStartNetworkRequest(
        QNetworkAccessManager* networkAccessManager,
        int parentTimeoutMillis) {
    networkAccessManager->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    DEBUG_ASSERT(networkAccessManager);

    const QNetworkRequest networkRequest =
            createNetworkRequest(m_coverArtUrl);

    if (kLogger.traceEnabled()) {
        kLogger.trace()
                << "GET"
                << networkRequest.url();
    }
    return networkAccessManager->get(networkRequest);
}

void CoverArtArchiveImageTask::doNetworkReplyFinished(
        QNetworkReply* finishedNetworkReply,
        network::HttpStatusCode statusCode) {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);

    const QByteArray resultImageBytes = finishedNetworkReply->readAll();

    if (statusCode != 200) {
        kLogger.info()
                << "GET reply"
                << "statusCode:" << statusCode;
        return;
    }

    emit succeeded(resultImageBytes);
}

} // namespace mixxx
