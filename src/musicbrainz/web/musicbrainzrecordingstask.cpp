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
#include "util/duration.h"
#include "util/logger.h"
#include "util/thread_affinity.h"
#include "util/versionstore.h"

namespace mixxx {

namespace {

const Logger kLogger("MusicBrainzRecordingsTask");

const QUrl kBaseUrl = QStringLiteral("https://musicbrainz.org/");

const QString kRequestPath = QStringLiteral("/ws/2/recording/");

const QByteArray kUserAgentRawHeaderKey = "User-Agent";

static constexpr int kMinTimeBetweenMbRequests = 1000;

QString userAgentRawHeaderValue() {
    return VersionStore::applicationName() +
            QStringLiteral("/") +
            VersionStore::version() +
            QStringLiteral(" ( ") +
            // QStringLiteral(MIXXX_WEBSITE_URL) fails to compile on Fedora 36 with GCC 12.0.x
            MIXXX_WEBSITE_URL +
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
          m_queuedRecordingIds(recordingIds),
          m_measurementTimer(0),
          m_parentTimeoutMillis(0) {
    musicbrainz::registerMetaTypesOnce();

    // According to the MusicBrainz API Doc: https://musicbrainz.org/doc/MusicBrainz_API/Rate_Limiting
    // The rate limit should be one query in a second.
    // Related Bug: https://github.com/mixxxdj/mixxx/issues/10795
    // In order to not hit the rate limits and respect their rate limiting rule.
    // Every request was delayed by one second.
    connect(&m_requestTimer,
            &QTimer::timeout,
            this,
            &MusicBrainzRecordingsTask::triggerSlotStart);
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

    m_measurementTimer.start();

    return networkAccessManager->get(networkRequest);
}

bool MusicBrainzRecordingsTask::doNetworkReplyFinished(
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
                        finishedNetworkReply->request().url(),
                        statusCode),
                error.code,
                error.message);
        return true;
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
                        finishedNetworkReply->request().url(),
                        statusCode),
                -1,
                QStringLiteral("Failed to parse XML response"));
        return m_queuedRecordingIds.isEmpty() ? true : false;
    }

    if (m_queuedRecordingIds.isEmpty()) {
        // Finished all recording ids
        m_finishedRecordingIds.clear();
        auto trackReleases = m_trackReleases.values();
        m_trackReleases.clear();
        emitSucceeded(trackReleases);
        return true;
    }

    // Continue with next recording id
    DEBUG_ASSERT(!m_queuedRecordingIds.isEmpty());
    auto timerSinceLastRequest = m_measurementTimer.elapsed(true);
    int timerSinceLastRequestMillis = timerSinceLastRequest.toIntegerMillis();
    qDebug() << "Task took:" << timerSinceLastRequestMillis;
    m_requestTimer.setSingleShot(true);
    if (timerSinceLastRequestMillis >= kMinTimeBetweenMbRequests) {
        qDebug() << "Task took more than a second, slot is calling now.";
        m_requestTimer.start(1);
    } else {
        auto sleepDuration = (kMinTimeBetweenMbRequests -
                timerSinceLastRequestMillis);
        qDebug() << "Task took less than a second, slot is going to be called in:" << sleepDuration;
        m_requestTimer.start(sleepDuration);
    }
    m_measurementTimer.restart(true);
    return false;
}

void MusicBrainzRecordingsTask::triggerSlotStart() {
    slotStart(m_parentTimeoutMillis);
}

void MusicBrainzRecordingsTask::doWaitingTaskAborted() {
    kLogger.info()
            << "Aborted task was waiting for next slot to be called."
            << "Is QTimer active? (true) || (false):"
            << m_requestTimer.isActive();
    if (m_requestTimer.isActive()) {
        m_requestTimer.stop();
        kLogger.info()
                << "QTimer is stopped.";
    }
}

void MusicBrainzRecordingsTask::emitSucceeded(
        const QList<musicbrainz::TrackRelease>& trackReleases) {
    VERIFY_OR_DEBUG_ASSERT(
            isSignalFuncConnected(&MusicBrainzRecordingsTask::succeeded)) {
        kLogger.warning()
                << "Unhandled succeeded signal";
        deleteLater();
        return;
    }
    m_requestTimer.stop();
    emit succeeded(trackReleases);
}

void MusicBrainzRecordingsTask::emitFailed(
        const network::WebResponse& response,
        int errorCode,
        const QString& errorMessage) {
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
            response,
            errorCode,
            errorMessage);
}

} // namespace mixxx
