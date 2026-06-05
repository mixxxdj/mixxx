#include "musicbrainz/acoustidworker.h"

#include <chromaprint.h>

#include <QEventLoop>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QThread>
#include <QTimer>
#include <QUrl>
#include <QUrlQuery>
#include <cstring>

#include "library/library_prefs.h"
#include "moc_acoustidworker.cpp"
#include "musicbrainz/gzip.h"
#include "util/assert.h"
#include "util/db/dbconnectionpooled.h"
#include "util/db/dbconnectionpooler.h"
#include "util/logger.h"

namespace mixxx {

namespace {

const Logger kLogger("AcoustIdWorker");

// Mixxx's built-in client key for lookups — same as AcoustIdLookupTask.
const QString kClientApiKey = QStringLiteral("czKxnkyO");

const QUrl kAcoustIdBaseUrl = QStringLiteral("https://api.acoustid.org/");
const QString kLookupPath = QStringLiteral("/v2/lookup");

// AcoustID allows max 3 requests/second.
constexpr int kRateLimitMs = 333;

// Timeout for connectivity check and per-lookup request.
constexpr int kConnectivityTimeoutMs = 5000;
constexpr int kRequestTimeoutMs = 15000;

// Exponential backoff: wait 4^attempt seconds before each retry.
// attempt=1 → 4s, attempt=2 → 16s (max_attempts=3 stops here).
constexpr int kBackoffBaseSeconds = 4;

// Maximum jobs fetched per poll cycle.
constexpr int kJobBatchSize = 10;

} // anonymous namespace

AcoustIdWorker::AcoustIdWorker(
        UserSettingsPointer pConfig,
        mixxx::DbConnectionPoolPtr dbConnectionPool)
        : WorkerThread(QStringLiteral("AcoustIdWorker"), QThread::LowPriority),
          m_pConfig(std::move(pConfig)),
          m_dbConnectionPool(std::move(dbConnectionPool)) {
    DEBUG_ASSERT(m_pConfig);
    DEBUG_ASSERT(m_dbConnectionPool);
}

void AcoustIdWorker::slotWakeUp() {
    wake();
}

void AcoustIdWorker::doRun() {
    // Keep a thread-local database connection alive for the lifetime of
    // this run — identical pattern to AnalyzerThread + AnalysisDao.
    mixxx::DbConnectionPooler dbConnectionPooler(m_dbConnectionPool);
    if (!dbConnectionPooler.isPooling()) {
        kLogger.warning() << "Failed to obtain database connection for AcoustID worker";
        return;
    }
    QSqlDatabase dbConnection = mixxx::DbConnectionPooled(m_dbConnectionPool);

    m_pFingerprintDao = std::make_unique<TrackFingerprintDao>(m_pConfig);
    m_pFingerprintDao->initialize(dbConnection);

    // QNetworkAccessManager must be created in the thread that uses it.
    // Local QEventLoops in doLookup() and checkConnectivity() provide the
    // event dispatching it requires.
    m_pNetwork = std::make_unique<QNetworkAccessManager>();

    while (!isStopping()) {
        if (!awaitWorkItemsFetched()) {
            break;
        }
        if (isStopping()) {
            break;
        }

        if (!checkConnectivity()) {
            kLogger.info() << "api.acoustid.org unreachable — sleeping 60 s";
            sleepMs(60000);
            m_pendingJobs.clear();
            continue;
        }

        while (!m_pendingJobs.isEmpty() && !isStopping()) {
            const AcoustIdJob job = m_pendingJobs.takeFirst();

            // Exponential backoff for previously failed jobs: 4^(attempts-1) seconds.
            // First attempt (attempts==0) proceeds immediately.
            // Sequence: 1 s → 4 s → 16 s (max_attempts=3).
            if (job.attempts > 0) {
                int backoffMs = 1000;
                for (int i = 1; i < job.attempts; ++i) {
                    backoffMs *= kBackoffBaseSeconds;
                }
                kLogger.debug() << "Backoff" << backoffMs << "ms for trackId"
                                << job.trackId << "(attempt" << job.attempts << ")";
                sleepMs(backoffMs);
                if (isStopping()) {
                    break;
                }
            }

            if (!processJob(job)) {
                kLogger.debug()
                        << "Transient failure for trackId"
                        << job.trackId
                        << "— will retry on next poll cycle";
            }

            // Enforce rate limit between consecutive API calls.
            if (!m_pendingJobs.isEmpty() && !isStopping()) {
                sleepMs(kRateLimitMs);
            }
        }

        if (m_pendingJobs.isEmpty()) {
            emit queueDrained();
        }
    }

    m_pFingerprintDao.reset();
    m_pNetwork.reset();
}

WorkerThread::TryFetchWorkItemsResult AcoustIdWorker::tryFetchWorkItems() {
    VERIFY_OR_DEBUG_ASSERT(m_pFingerprintDao) {
        return TryFetchWorkItemsResult::Idle;
    }
    m_pendingJobs = m_pFingerprintDao->getPendingJobs(kJobBatchSize);
    if (m_pendingJobs.isEmpty()) {
        return TryFetchWorkItemsResult::Idle;
    }
    kLogger.debug() << "Fetched" << m_pendingJobs.size() << "pending AcoustID jobs";
    return TryFetchWorkItemsResult::Ready;
}

bool AcoustIdWorker::checkConnectivity() {
    VERIFY_OR_DEBUG_ASSERT(m_pNetwork) {
        return false;
    }
    QNetworkRequest request(kAcoustIdBaseUrl);
    request.setAttribute(
            QNetworkRequest::RedirectPolicyAttribute,
            QNetworkRequest::NoLessSafeRedirectPolicy);

    QNetworkReply* pReply = m_pNetwork->head(request);
    VERIFY_OR_DEBUG_ASSERT(pReply) {
        return false;
    }

    QEventLoop loop;
    QTimer timeoutTimer;
    timeoutTimer.setSingleShot(true);
    connect(pReply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    connect(&timeoutTimer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timeoutTimer.start(kConnectivityTimeoutMs);
    loop.exec();
    timeoutTimer.stop();

    if (!pReply->isFinished()) {
        // Timeout fired before the reply arrived — abort the request.
        pReply->abort();
        pReply->deleteLater();
        kLogger.debug() << "Connectivity check timed out";
        return false;
    }

    // Any HTTP response (even 4xx) means the host is reachable.
    const bool reachable =
            pReply->error() == QNetworkReply::NoError ||
            pReply->error() == QNetworkReply::ContentNotFoundError ||
            pReply->error() == QNetworkReply::AuthenticationRequiredError;

    const QNetworkReply::NetworkError err = pReply->error();
    pReply->deleteLater();

    kLogger.debug() << "Connectivity:" << (reachable ? "ok" : "unreachable")
                    << "networkError:" << err;
    return reachable;
}

// static
QString AcoustIdWorker::encodeChromaFingerprint(const QByteArray& rawChromaData) {
    if (rawChromaData.isEmpty()) {
        return {};
    }
    // The .chroma file is a flat array of uint32_t values written by
    // AnalyzerChromaprint via chromaprint_get_raw_fingerprint().
    // chromaprint_encode_fingerprint() compresses and Base64-encodes it
    // back to the format the AcoustID API expects.
    const int numValues =
            rawChromaData.size() / static_cast<int>(sizeof(uint32_t));
    if (numValues == 0) {
        return {};
    }

    // QByteArray only guarantees char alignment. Copy into an aligned
    // buffer to avoid undefined behaviour on strict-alignment platforms
    // (ARM, UBSan builds).
    std::vector<uint32_t> aligned(numValues);
    std::memcpy(aligned.data(), rawChromaData.constData(), numValues * sizeof(uint32_t));
    const uint32_t* pRaw = aligned.data();

    char* pEncoded = nullptr;
    int encodedSize = 0;
    // Last argument 1 = request Base64 output.
    const int rc = chromaprint_encode_fingerprint(
            pRaw,
            numValues,
            CHROMAPRINT_ALGORITHM_DEFAULT,
            &pEncoded,
            &encodedSize,
            1);

    if (rc != 1 || pEncoded == nullptr) {
        kLogger.warning() << "chromaprint_encode_fingerprint failed";
        return {};
    }

    QString result = QString::fromLatin1(pEncoded, encodedSize);
    chromaprint_dealloc(pEncoded);
    return result;
}

bool AcoustIdWorker::processJob(const AcoustIdJob& job) {
    kLogger.debug() << "Processing AcoustID job for trackId" << job.trackId
                    << "queueId" << job.queueId
                    << "attempts" << job.attempts;

    VERIFY_OR_DEBUG_ASSERT(m_pFingerprintDao) {
        return false;
    }

    // Step 1 — fetch the stored fingerprint metadata for this track.
    const auto pMetadata =
            m_pFingerprintDao->getFingerprintMetadata(job.trackId);
    if (!pMetadata || !pMetadata->fingerprintValid) {
        kLogger.warning() << "No valid fingerprint_metadata for trackId"
                          << job.trackId << "— removing from queue";
        m_pFingerprintDao->deleteQueueEntry(job.trackId);
        return true;
    }

    // Step 2 — check the local acoustid_cache before hitting the network.
    // Two different tracks that share the exact same audio (same SHA-256)
    // will both hit this cache after the first one is looked up.
    const auto pCached =
            m_pFingerprintDao->lookupAcoustIdCache(pMetadata->chromaSha256);
    if (pCached) {
        kLogger.debug() << "Cache hit sha256" << pMetadata->chromaSha256
                        << "acoustidId" << pCached->acoustidId;
        emit acoustidResultReady(
                job.trackId,
                pCached->acoustidId,
                QStringLiteral("completed"),
                pCached->musicbrainzRecordingId,
                pCached->musicbrainzReleaseId,
                QString(),
                QString());
        m_pFingerprintDao->deleteQueueEntry(job.trackId);
        return true;
    }

    // Step 3 — load .chroma file and encode for AcoustID.
    const QByteArray rawChroma =
            m_pFingerprintDao->loadChromaFile(job.trackId);
    if (rawChroma.isEmpty()) {
        kLogger.warning() << "Missing .chroma file for trackId" << job.trackId
                          << "— marking needs regen";
        m_pFingerprintDao->markFingerprintNeedsRegen(job.trackId);
        m_pFingerprintDao->deleteQueueEntry(job.trackId);
        return true;
    }

    const QString encodedFingerprint = encodeChromaFingerprint(rawChroma);
    if (encodedFingerprint.isEmpty()) {
        kLogger.warning() << "Fingerprint encoding failed for trackId" << job.trackId;
        m_pFingerprintDao->updateQueueStatus(
                job.queueId,
                QStringLiteral("failed"),
                QStringLiteral("fingerprint encoding failed"));
        return false;
    }

    const int durationSeconds =
            static_cast<int>(pMetadata->fingerprintDuration);

    // Step 4 — send the network lookup request.
    const auto optResult = doLookup(encodedFingerprint, durationSeconds);
    if (!optResult) {
        // Transient network failure — increment attempts, worker will retry
        // with exponential backoff on the next poll cycle.
        m_pFingerprintDao->updateQueueStatus(
                job.queueId,
                QStringLiteral("failed"),
                QStringLiteral("network error"));
        return false;
    }

    const LookupResult& result = *optResult;

    if (!result.acoustidId.isEmpty()) {
        // Step 5a — match found.
        kLogger.info() << "AcoustID match for trackId" << job.trackId
                       << "acoustidId" << result.acoustidId
                       << "score" << result.score;

        const QString mbRecordingId = result.recordingIds.isEmpty()
                ? QString()
                : result.recordingIds.first().toString(QUuid::WithoutBraces);

        // Persist to cache so other tracks with the same audio skip the API.
        AcoustIdCacheEntry cacheEntry;
        cacheEntry.chromaSha256 = pMetadata->chromaSha256;
        cacheEntry.acoustidId = result.acoustidId;
        cacheEntry.confidence = result.score;
        cacheEntry.musicbrainzRecordingId = mbRecordingId;
        cacheEntry.lookupTimestamp = QDateTime::currentDateTimeUtc();
        m_pFingerprintDao->cacheAcoustIdResult(cacheEntry);

        // Write results to library via signal — TrackDAO's m_database belongs
        // to the main thread and must not be accessed from the worker thread.
        emit acoustidResultReady(
                job.trackId,
                result.acoustidId,
                QStringLiteral("completed"),
                mbRecordingId,
                QString(), // release ID — requires a separate MusicBrainz call
                QString(),
                QString());
        m_pFingerprintDao->deleteQueueEntry(job.trackId);
        return true;
    }

    // Step 5b — no match found.
    kLogger.info() << "No AcoustID match for trackId" << job.trackId;

    const bool autoSubmit = m_pConfig->getValue(
            mixxx::library::prefs::kAcoustIdAutoSubmitConfigKey, false);
    const QString userApiKey = m_pConfig->getValue(
            mixxx::library::prefs::kAcoustIdUserApiKeyConfigKey, QString());

    if (autoSubmit && !userApiKey.isEmpty()) {
        // Submission deferred to CMRT 19 (manual submission dialog PR).
        // Mark as unmatched so the MusicBrainzQueue view surfaces it.
        kLogger.debug() << "Auto-submit queued for trackId" << job.trackId
                        << "— deferred to CMRT 19";
    }

    // "unmatched" distinguishes a successful-but-empty lookup from a
    // network or encoding failure. The MBQueue view (CMRT 18) uses
    // this to show "no match" vs "lookup failed" separately.
    emit acoustidResultReady(
            job.trackId,
            QString(),
            QStringLiteral("unmatched"),
            QString(),
            QString(),
            QString(),
            QString());
    m_pFingerprintDao->updateQueueStatus(
            job.queueId,
            QStringLiteral("unmatched"),
            QStringLiteral("no match found"));
    return true;
}

std::optional<AcoustIdWorker::LookupResult> AcoustIdWorker::doLookup(
        const QString& fingerprint,
        int durationSeconds) {
    VERIFY_OR_DEBUG_ASSERT(m_pNetwork) {
        return std::nullopt;
    }

    QUrl url(kAcoustIdBaseUrl);
    url.setPath(kLookupPath);

    QUrlQuery params;
    params.addQueryItem(QStringLiteral("format"), QStringLiteral("json"));
    params.addQueryItem(QStringLiteral("client"), kClientApiKey);
    // recordingids returns the MusicBrainz UUID for each match — sufficient
    // for our cache and library write. A separate MusicBrainz call is needed
    // for release IDs (deferred to a later PR).
    params.addQueryItem(QStringLiteral("meta"), QStringLiteral("recordingids"));
    params.addQueryItem(QStringLiteral("fingerprint"), fingerprint);
    params.addQueryItem(QStringLiteral("duration"), QString::number(durationSeconds));

    // AcoustID requires a gzip-compressed, URL-encoded POST body —
    // same as AcoustIdLookupTask.
    const QByteArray body =
            gzipCompress(params.query(QUrl::FullyEncoded).toLatin1());

    QNetworkRequest request(url);
    request.setHeader(
            QNetworkRequest::ContentTypeHeader,
            QStringLiteral("application/x-www-form-urlencoded"));
    request.setRawHeader("Content-Encoding", "gzip");
    request.setAttribute(
            QNetworkRequest::RedirectPolicyAttribute,
            QNetworkRequest::NoLessSafeRedirectPolicy);

    QNetworkReply* pReply = m_pNetwork->post(request, body);
    VERIFY_OR_DEBUG_ASSERT(pReply) {
        return std::nullopt;
    }

    // Block the worker thread using a local QEventLoop until the reply
    // arrives or the timeout fires. This is the correct pattern for
    // network I/O inside a WorkerThread (no persistent event loop).
    QEventLoop loop;
    QTimer timeoutTimer;
    timeoutTimer.setSingleShot(true);
    connect(pReply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    connect(&timeoutTimer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timeoutTimer.start(kRequestTimeoutMs);
    loop.exec();
    timeoutTimer.stop();

    if (!pReply->isFinished()) {
        // Timeout fired before the reply arrived — abort the request.
        pReply->abort();
        pReply->deleteLater();
        kLogger.warning() << "AcoustID lookup timed out";
        return std::nullopt;
    }

    if (pReply->error() != QNetworkReply::NoError) {
        kLogger.warning() << "AcoustID lookup network error:"
                          << pReply->errorString();
        pReply->deleteLater();
        return std::nullopt;
    }

    const QByteArray responseData = pReply->readAll();
    pReply->deleteLater();

    QJsonParseError parseError;
    const QJsonDocument doc =
            QJsonDocument::fromJson(responseData, &parseError);
    if (doc.isNull()) {
        kLogger.warning() << "Failed to parse AcoustID JSON:"
                          << parseError.errorString();
        return std::nullopt;
    }
    VERIFY_OR_DEBUG_ASSERT(doc.isObject()) {
        return std::nullopt;
    }

    const QJsonObject root = doc.object();
    if (root.value(QStringLiteral("status")).toString() !=
            QStringLiteral("ok")) {
        kLogger.warning() << "AcoustID response status not ok:"
                          << root.value(QStringLiteral("status")).toString();
        return std::nullopt;
    }

    const QJsonArray results =
            root.value(QStringLiteral("results")).toArray();
    if (results.isEmpty()) {
        // Valid API response, but no match in the AcoustID database.
        return LookupResult{};
    }

    // Results are ordered by score descending — take the first (best) one.
    const QJsonObject topResult = results.first().toObject();
    LookupResult result;
    result.acoustidId =
            topResult.value(QStringLiteral("id")).toString();
    result.score =
            topResult.value(QStringLiteral("score")).toDouble(0.0);

    const QJsonArray recordings =
            topResult.value(QStringLiteral("recordings")).toArray();
    for (const auto& rec : recordings) {
        const QUuid id(rec.toObject()
                        .value(QStringLiteral("id"))
                        .toString());
        if (!id.isNull()) {
            result.recordingIds.append(id);
        }
    }

    return result;
}

void AcoustIdWorker::sleepMs(int ms) {
    // Sleep in 100 ms slices so the thread checks isStopping() regularly
    // and exits promptly when stop() is called.
    constexpr int kSliceMs = 100;
    int remaining = ms;
    while (remaining > 0 && !isStopping()) {
        msleep(static_cast<unsigned long>(std::min(remaining, kSliceMs)));
        remaining -= kSliceMs;
    }
}

} // namespace mixxx
