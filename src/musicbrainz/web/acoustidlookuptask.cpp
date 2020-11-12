#include "musicbrainz/web/acoustidlookuptask.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QMetaMethod>

#include "musicbrainz/gzip.h"
#include "util/assert.h"
#include "util/logger.h"

namespace mixxx {

namespace {

const Logger kLogger("AcoustIdLookupTask");

// see API-KEY site here http://acoustid.org/application/496
// I registered the KEY for version 1.12 -- kain88 (may 2013)
// See also: https://acoustid.org/webservice
const QString kClientApiKey = QStringLiteral("czKxnkyO");

const QUrl kBaseUrl = QStringLiteral("https://api.acoustid.org/");

const QString kRequestPath = QStringLiteral("/v2/lookup");

const QLatin1String kContentTypeHeaderValue("application/x-www-form-urlencoded");

const QByteArray kContentEncodingRawHeaderKey = "Content-Encoding";
const QByteArray kContentEncodingRawHeaderValue = "gzip";

QUrlQuery lookupUrlQuery(
        const QString& fingerprint,
        int duration) {
    DEBUG_ASSERT(!fingerprint.isEmpty());
    DEBUG_ASSERT(duration >= 0);

    QUrlQuery urlQuery;
    urlQuery.addQueryItem(
            QStringLiteral("format"),
            QStringLiteral("json"));
    urlQuery.addQueryItem(
            QStringLiteral("client"),
            kClientApiKey);
    urlQuery.addQueryItem(
            QStringLiteral("meta"),
            QStringLiteral("recordingids"));
    urlQuery.addQueryItem(
            QStringLiteral("fingerprint"),
            fingerprint);
    urlQuery.addQueryItem(
            QStringLiteral("duration"),
            QString::number(duration));
    return urlQuery;
}

network::JsonWebRequest lookupRequest() {
    return network::JsonWebRequest{
            network::HttpRequestMethod::Post,
            kRequestPath,
            QUrlQuery(),     // custom query
            QJsonDocument(), // custom body
    };
}

} // anonymous namespace

AcoustIdLookupTask::AcoustIdLookupTask(
        QNetworkAccessManager* networkAccessManager,
        const QString& fingerprint,
        int duration,
        QObject* parent)
        : network::JsonWebTask(
                  networkAccessManager,
                  kBaseUrl,
                  lookupRequest(),
                  parent),
          m_urlQuery(lookupUrlQuery(fingerprint, duration)) {
}

QNetworkReply* AcoustIdLookupTask::sendNetworkRequest(
        QNetworkAccessManager* networkAccessManager,
        network::HttpRequestMethod method,
        const QUrl& url,
        const QJsonDocument& content) {
    Q_UNUSED(method);
    DEBUG_ASSERT(method == network::HttpRequestMethod::Post);
    Q_UNUSED(content);
    DEBUG_ASSERT(content.isEmpty());

    DEBUG_ASSERT(url.isValid());
    QNetworkRequest req(url);
    req.setHeader(
            QNetworkRequest::ContentTypeHeader,
            kContentTypeHeaderValue);
    req.setRawHeader(
            kContentEncodingRawHeaderKey,
            kContentEncodingRawHeaderValue);

    // application/x-www-form-urlencoded request bodies must be percent encoded.
    DEBUG_ASSERT(!m_urlQuery.isEmpty());
    QByteArray body = gzipCompress(
            m_urlQuery.query(QUrl::FullyEncoded).toLatin1());

    if (kLogger.traceEnabled()) {
        kLogger.trace()
                << "POST"
                << url
                << body;
    }
    DEBUG_ASSERT(networkAccessManager);
    return networkAccessManager->post(req, body);
}

void AcoustIdLookupTask::onFinished(
        network::JsonWebResponse&& response) {
    if (!response.isStatusCodeSuccess()) {
        kLogger.warning()
                << "Request failed with HTTP status code"
                << response.statusCode;
        emitFailed(std::move(response));
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(response.statusCode == network::kHttpStatusCodeOk) {
        kLogger.warning()
                << "Unexpected HTTP status code"
                << response.statusCode;
        emitFailed(std::move(response));
        return;
    }

    VERIFY_OR_DEBUG_ASSERT(response.content.isObject()) {
        kLogger.warning()
                << "Invalid JSON content"
                << response.content;
        emitFailed(std::move(response));
        return;
    }
    const auto jsonObject = response.content.object();

    const auto statusText = jsonObject.value(QStringLiteral("status")).toString();
    if (statusText != QStringLiteral("ok")) {
        kLogger.warning()
                << "Unexpected response status"
                << statusText;
        emitFailed(std::move(response));
        return;
    }

    QList<QUuid> recordingIds;
    DEBUG_ASSERT(jsonObject.value(QLatin1String("results")).isArray());
    const QJsonArray results = jsonObject.value(QLatin1String("results")).toArray();
    double maxScore = -1.0; // uninitialized (< 0)
    // Results are expected to be ordered by score (descending)
    for (const auto& result : results) {
        DEBUG_ASSERT(result.isObject());
        const auto resultObject = result.toObject();
        const auto resultId =
                resultObject.value(QLatin1String("id")).toString();
        DEBUG_ASSERT(!resultId.isEmpty());
        // The default score is 1.0 if missing
        const double score =
                resultObject.value(QLatin1String("score")).toDouble(1.0);
        DEBUG_ASSERT(score >= 0.0);
        DEBUG_ASSERT(score <= 1.0);
        if (maxScore < 0.0) {
            // Initialize the maximum score
            maxScore = score;
        }
        DEBUG_ASSERT(score <= maxScore);
        if (score < maxScore && !recordingIds.isEmpty()) {
            // Ignore all remaining results with lower values
            // than the maximum score
            break;
        }
        const auto recordings = result.toObject().value(QLatin1String("recordings"));
        if (recordings.isUndefined()) {
            if (kLogger.debugEnabled()) {
                kLogger.debug()
                        << "No recording(s) available for result"
                        << resultId
                        << "with score"
                        << score;
            }
            continue;
        } else {
            DEBUG_ASSERT(recordings.isArray());
            const QJsonArray recordingsArray = recordings.toArray();
            if (kLogger.debugEnabled()) {
                kLogger.debug()
                        << "Found"
                        << recordingsArray.size()
                        << "recording(s) for result"
                        << resultId
                        << "with score"
                        << score;
            }
            for (const auto& recording : recordingsArray) {
                DEBUG_ASSERT(recording.isObject());
                const auto recordingObject = recording.toObject();
                const auto recordingId =
                        QUuid(recordingObject.value(QLatin1String("id")).toString());
                VERIFY_OR_DEBUG_ASSERT(!recordingId.isNull()) {
                    continue;
                }
                recordingIds.append(recordingId);
            }
        }
    }
    emitSucceeded(std::move(recordingIds));
}

void AcoustIdLookupTask::emitSucceeded(
        QList<QUuid>&& recordingIds) {
    VERIFY_OR_DEBUG_ASSERT(
            isSignalFuncConnected(&AcoustIdLookupTask::succeeded)) {
        kLogger.warning()
                << "Unhandled succeeded signal";
        deleteLater();
        return;
    }
    emit succeeded(
            std::move(recordingIds));
}

} // namespace mixxx
