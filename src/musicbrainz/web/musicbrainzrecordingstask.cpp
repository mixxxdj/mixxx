#include "musicbrainz/web/musicbrainzrecordingstask.h"

#include <QMetaMethod>
#include <QXmlStreamReader>

#include "defs_urls.h"
#include "moc_musicbrainzrecordingstask.cpp"
#include "musicbrainz/gzip.h"
#include "musicbrainz/musicbrainzxml.h"
#include "network/httpstatuscode.h"
#include "util/assert.h"
#include "util/compatibility.h"
#include "util/logger.h"
#include "util/thread_affinity.h"
#include "util/version.h"

namespace mixxx {

namespace {

const Logger kLogger("MusicBrainzRecordingsTask");

const QUrl kBaseUrl = QStringLiteral("https://musicbrainz.org/");

const QString kRequestPath = QStringLiteral("/ws/2/recording/");

const QByteArray kUserAgentRawHeaderKey = "User-Agent";

QString userAgentRawHeaderValue() {
    return Version::applicationName() +
            QStringLiteral("/") +
            Version::version() +
            QStringLiteral(" ( ") +
            QStringLiteral(MIXXX_WEBSITE_URL) +
            QStringLiteral(" )");
}

QUrlQuery createUrlQuery() {
    typedef QPair<QString, QString> Param;
    QList<Param> params;
    params << Param("inc", "artists+artist-credits+releases+release-groups+media");

    QUrlQuery query;
    query.setQueryItems(params);
    return query;
}

QNetworkRequest createNetworkRequest(
        const QUuid& recordingId) {
    DEBUG_ASSERT(kBaseUrl.isValid());
    DEBUG_ASSERT(!recordingId.isNull());
    QUrl url = kBaseUrl;
    url.setPath(kRequestPath + uuidToStringWithoutBraces(recordingId));
    url.setQuery(createUrlQuery());
    DEBUG_ASSERT(url.isValid());
    QNetworkRequest networkRequest(url);
    // https://musicbrainz.org/doc/XML_Web_Service/Rate_Limiting#Provide_meaningful_User-Agent_strings
    // HTTP request headers must be latin1.
    networkRequest.setRawHeader(
            kUserAgentRawHeaderKey,
            userAgentRawHeaderValue().toLatin1());
    return networkRequest;
}

} // anonymous namespace

MusicBrainzRecordingsTask::MusicBrainzRecordingsTask(
        QNetworkAccessManager* networkAccessManager,
        QList<QUuid>&& recordingIds,
        QObject* parent)
        : network::WebTask(
                  networkAccessManager,
                  parent),
          m_queuedRecordingIds(std::move(recordingIds)),
          m_parentTimeoutMillis(0) {
    musicbrainz::registerMetaTypesOnce();
}

QNetworkReply* MusicBrainzRecordingsTask::doStartNetworkRequest(
        QNetworkAccessManager* networkAccessManager,
        int parentTimeoutMillis) {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    DEBUG_ASSERT(networkAccessManager);

    m_parentTimeoutMillis = parentTimeoutMillis;

    VERIFY_OR_DEBUG_ASSERT(!m_queuedRecordingIds.isEmpty()) {
        kLogger.warning()
                << "Nothing to do";
        return nullptr;
    }
    const auto recordingId = m_queuedRecordingIds.takeFirst();
    DEBUG_ASSERT(!recordingId.isNull());

    const QNetworkRequest networkRequest =
            createNetworkRequest(recordingId);

    if (kLogger.traceEnabled()) {
        kLogger.trace()
                << "GET"
                << networkRequest.url();
    }
    return networkAccessManager->get(networkRequest);
}

void MusicBrainzRecordingsTask::doNetworkReplyFinished(
        QNetworkReply* finishedNetworkReply,
        network::HttpStatusCode statusCode) {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);

    const QByteArray body = finishedNetworkReply->readAll();
    QXmlStreamReader reader(body);

    // HTTP status of successful results:
    // 200: Found
    // 301: Found, but UUID moved permanently in database
    // 404: Not found in database, i.e. empty result
    if (statusCode != 200 &&
            statusCode != 301 &&
            statusCode != 404) {
        kLogger.info()
                << "GET reply"
                << "statusCode:" << statusCode
                << "body:" << body;
        auto error = musicbrainz::Error(reader);
        emitFailed(
                network::WebResponse(
                        finishedNetworkReply->url(),
                        statusCode),
                error.code,
                std::move(error.message));
        return;
    }

    auto recordingsResult = musicbrainz::parseRecordings(reader);
    for (auto&& trackRelease : recordingsResult.first) {
        // In case of a response with status 301 (Moved Permanently)
        // the actual recording id might differ from the requested id.
        // To avoid requesting recording ids twice we need to remember
        // all recording ids.
        m_finishedRecordingIds.insert(trackRelease.recordingId);
        m_trackReleases.insert(trackRelease.trackReleaseId, trackRelease);
    }
    if (!recordingsResult.second) {
        kLogger.warning()
                << "Failed to parse XML response";
        emitFailed(
                network::WebResponse(
                        finishedNetworkReply->url(),
                        statusCode),
                -1,
                "Failed to parse XML response");
        return;
    }

    if (m_queuedRecordingIds.isEmpty()) {
        // Finished all recording ids
        m_finishedRecordingIds.clear();
        auto trackReleases = m_trackReleases.values();
        m_trackReleases.clear();
        emitSucceeded(std::move(trackReleases));
        return;
    }

    // Continue with next recording id
    DEBUG_ASSERT(!m_queuedRecordingIds.isEmpty());
    slotStart(m_parentTimeoutMillis);
}

void MusicBrainzRecordingsTask::emitSucceeded(
        QList<musicbrainz::TrackRelease>&& trackReleases) {
    VERIFY_OR_DEBUG_ASSERT(
            isSignalFuncConnected(&MusicBrainzRecordingsTask::succeeded)) {
        kLogger.warning()
                << "Unhandled succeeded signal";
        deleteLater();
        return;
    }
    emit succeeded(
            std::move(trackReleases));
}

void MusicBrainzRecordingsTask::emitFailed(
        network::WebResponse&& response,
        int errorCode,
        QString&& errorMessage) {
    VERIFY_OR_DEBUG_ASSERT(
            isSignalFuncConnected(&MusicBrainzRecordingsTask::failed)) {
        kLogger.warning()
                << "Unhandled failed signal"
                << response
                << errorCode
                << errorMessage;
        deleteLater();
        return;
    }
    emit failed(
            std::move(response),
            errorCode,
            std::move(errorMessage));
}

} // namespace mixxx
