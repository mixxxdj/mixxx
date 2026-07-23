#pragma once

#include <QList>
#include <QNetworkAccessManager>
#include <QUuid>
#include <memory>
#include <optional>

#include "library/dao/trackfingerprintdao.h"
#include "musicbrainz/cmrtgroupingservice.h"
#include "preferences/usersettings.h"
#include "util/db/dbconnectionpool.h"
#include "util/workerthread.h"

namespace mixxx {

/// Background worker that processes the acoustid_queue, performing AcoustID
/// fingerprint lookups and optionally submitting unmatched fingerprints.
///
/// Network model: each HTTP request uses a local QEventLoop to block the
/// worker thread until the reply arrives. No persistent event loop is needed,
/// which is consistent with WorkerThread (a thread without an event loop).
///
/// Rate limit   : 333 ms between API requests (AcoustID allows 3/sec).
/// Retry backoff: exponential — 4^attempt seconds before re-attempting
///                a failed job (1 s → 4 s → 16 s, max_attempts = 3).
/// Connectivity : lightweight HEAD request to api.acoustid.org — not raw
///                ICMP ping, which requires elevated privileges on some OSes.
class AcoustIdWorker : public WorkerThread {
    Q_OBJECT

  public:
    AcoustIdWorker(
            UserSettingsPointer pConfig,
            mixxx::DbConnectionPoolPtr dbConnectionPool);
    ~AcoustIdWorker() override = default;

  public slots:
    /// Called by AnalyzerThread after a batch of tracks finishes analysis
    /// to wake the worker and process newly enqueued jobs.
    void slotWakeUp();

  signals:
    /// Emitted when all pending jobs in the current batch have been handled.
    void queueDrained();

    /// Emitted when a lookup result should be written back to the library
    /// table. Connected to TrackDAO::updateAcoustIdResult in the main
    /// thread via a queued connection — TrackDAO's m_database must not be
    /// accessed from the worker thread.
    void acoustidResultReady(
            TrackId trackId,
            QString acoustidId,
            QString acoustidLookupStatus,
            QString musicbrainzRecordingId,
            QString musicbrainzReleaseId,
            QString musicbrainzTrackId,
            QString musicbrainzArtistId);

  protected:
    void doRun() override;
    TryFetchWorkItemsResult tryFetchWorkItems() override;

  private:
    /// Full result from one AcoustID /v2/lookup call.
    struct LookupResult {
        QString acoustidId;        // AcoustID entry UUID (result.id field)
        double score{0.0};         // Confidence score 0.0–1.0
        QList<QUuid> recordingIds; // MusicBrainz recording UUIDs

        QString musicbrainzReleaseId; // recordings[0].releases[0].id
        QString musicbrainzArtistId;  // recordings[0].releases[0].artists[0].id
        QString musicbrainzTrackId;   // recordings[0].releases[0].mediums[0].tracks[0].id
    };

    /// Returns true if api.acoustid.org responds to a HEAD request within
    /// the connectivity timeout.
    bool checkConnectivity();

    /// Processes one job end-to-end. Returns true when the job is fully
    /// handled (matched, unmatched, or permanently failed). Returns false
    /// only on a transient network error that should be retried via backoff.
    bool processJob(const AcoustIdJob& job);

    /// Encodes the raw uint32[] stored in a .chroma file to the
    /// Chromaprint-compressed, Base64-encoded string the AcoustID API
    /// expects. Returns an empty string on failure.
    static QString encodeChromaFingerprint(const QByteArray& rawChromaData);

    /// Sends a POST to /v2/lookup and blocks the worker thread via a local
    /// QEventLoop until the reply arrives. Returns std::nullopt on network
    /// or parse failure. Returns an empty LookupResult (acoustidId.isEmpty())
    /// when the lookup succeeds but AcoustID found no match.
    std::optional<LookupResult> doLookup(
            const QString& fingerprint,
            int durationSeconds);

    /// Sleeps for the given number of milliseconds. Checks isStopping()
    /// every 100 ms so the thread remains promptly stoppable.
    void sleepMs(int ms);

    const UserSettingsPointer m_pConfig;
    const mixxx::DbConnectionPoolPtr m_dbConnectionPool;

    // Created in doRun() — must live in the worker thread context so Qt
    // routes network events through the local QEventLoops used here.
    std::unique_ptr<QNetworkAccessManager> m_pNetwork;

    // Created in doRun() with a thread-local DB connection from the pool —
    // same pattern as AnalyzerThread + AnalysisDao.
    std::unique_ptr<TrackFingerprintDao> m_pFingerprintDao;

    // Created in doRun() right after m_pFingerprintDao. Runs the CMRT
    // grouping pipeline after a successful or unmatched AcoustID lookup
    std::unique_ptr<CmrtGroupingService> m_pGroupingService;

    QList<AcoustIdJob> m_pendingJobs;
};

} // namespace mixxx
