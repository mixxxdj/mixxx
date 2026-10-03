#pragma once

#include <deque>
#include <mutex>
#include <optional>

#include "track/track_decl.h"
#include "util/db/dbconnectionpool.h"
#include "util/workerthread.h"

/// Serializes the export of track metadata into source files on a
/// dedicated worker thread. Writing file tags may take a noticeable
/// amount of time, e.g. when the files are stored on slow or
/// network-mounted storage, and must not block the GUI thread.
///
/// Each job consists of a temporary, uncached Track object that holds
/// a snapshot of all metadata needed for the export, cloned from the
/// evicted track before it gets deleted. After the export has been
/// processed the updated source synchronization timestamp is written
/// into the database directly by this worker thread. The temporary
/// track is handed back with the trackExported() signal and gets
/// destroyed on the receiving thread.
class TrackMetadataExportThread : public WorkerThread {
    Q_OBJECT

  public:
    explicit TrackMetadataExportThread(
            mixxx::DbConnectionPoolPtr pDbConnectionPool);
    ~TrackMetadataExportThread() override = default;

    /// Clones all metadata required for the export from the given
    /// track into a temporary track object and enqueues it for
    /// deferred writing. May be invoked from any thread.
    void enqueueExport(
            const Track& track,
            const SyncTrackMetadataParams& syncParams);

  signals:
    /// Emitted from the worker thread after a job has been processed.
    /// Receivers are invoked with a queued connection on their own
    /// thread. The temporary track is passed back to ensure that it
    /// is finally destroyed on the receiving thread.
    void trackExported(
            TrackPointer pTrack,
            ExportTrackMetadataResult result);

  private:
    struct Job {
        TrackPointer pTrack;
        SyncTrackMetadataParams syncParams;
    };

    void doRun() override;
    TryFetchWorkItemsResult tryFetchWorkItems() override;
    void processJob(Job&& job);
    int pendingJobs();

    const mixxx::DbConnectionPoolPtr m_pDbConnectionPool;

    std::deque<Job> m_queue;
    std::mutex m_queueMutex;
    std::optional<Job> m_currentJob;
};
