#include "library/trackmetadataexportthread.h"

#include <QDateTime>
#include <QSqlDatabase>

#include "library/queryutil.h"
#include "moc_trackmetadataexportthread.cpp"
#include "sources/soundsourceproxy.h"
#include "track/track.h"
#include "util/db/dbconnectionpooled.h"
#include "util/db/dbconnectionpooler.h"
#include "util/db/fwdsqlquery.h"
#include "util/logger.h"

namespace {

const mixxx::Logger kLogger("TrackMetadataExportThread");

// Creates a temporary, uncached copy of the given track that contains
// all properties needed for exporting the metadata into the source
// file. The file location and the track id are shared with the
// original track.
TrackPointer createTrackForMetadataExport(const Track& track) {
    auto pTrack =
            std::make_shared<Track>(track.getFileAccess(), track.getId());
    pTrack->replaceRecord(track.getRecord(), track.getBeats());
    pTrack->setCuePoints(track.getCuePoints());
    if (track.isMarkedForMetadataExport()) {
        pTrack->markForMetadataExport();
    }
    return pTrack;
}

// Writes the new synchronization timestamp into the database after the
// export has finished. The evicted track object has already been
// deleted at this point, so it cannot be saved by the track collection
// anymore.
void updateSourceSynchronizedAt(
        const QSqlDatabase& dbConnection,
        const TrackId& trackId,
        const QDateTime& sourceSynchronizedAt) {
    DEBUG_ASSERT(!sourceSynchronizedAt.isValid() ||
            sourceSynchronizedAt.timeSpec() == Qt::UTC);
    FwdSqlQuery query(dbConnection,
            QStringLiteral(
                    "UPDATE library SET source_synchronized_ms=:source_synchronized_ms "
                    "WHERE id=:id"));
    query.bindValue(QStringLiteral(":source_synchronized_ms"),
            sourceSynchronizedAt.isValid()
                    ? QVariant(sourceSynchronizedAt.toMSecsSinceEpoch())
                    : QVariant());
    query.bindValue(QStringLiteral(":id"), trackId.toVariant());
    VERIFY_OR_DEBUG_ASSERT(query.execPrepared()) {
        LOG_FAILED_QUERY(query)
                << "Failed to update source synchronization timestamp of"
                << trackId;
    }
}

} // anonymous namespace

TrackMetadataExportThread::TrackMetadataExportThread(
        mixxx::DbConnectionPoolPtr pDbConnectionPool)
        : WorkerThread(QStringLiteral("TrackMetadataExport")),
          m_pDbConnectionPool(std::move(pDbConnectionPool)) {
    qRegisterMetaType<TrackPointer>();
    qRegisterMetaType<ExportTrackMetadataResult>();
}

void TrackMetadataExportThread::enqueueExport(
        const Track& track,
        const SyncTrackMetadataParams& syncParams) {
    {
        const std::lock_guard lock(m_queueMutex);
        m_queue.push_back(
                {createTrackForMetadataExport(track), syncParams});
    }
    wake();
}

void TrackMetadataExportThread::doRun() {
    mixxx::DbConnectionPooler dbConnectionPooler(m_pDbConnectionPool);
    if (m_pDbConnectionPool && !dbConnectionPooler.isPooling()) {
        kLogger.warning()
                << "Failed to obtain database connection for exporting track metadata";
    }

    while (awaitWorkItemsFetched()) {
        processJob(std::move(*m_currentJob));
        m_currentJob.reset();
    }
    // Write all pending jobs before exiting, e.g. after the global
    // track cache has been deactivated during shutdown.
    int pending = pendingJobs();
    if (pending > 0) {
        kLogger.info()
                << "Writing" << pending
                << "pending track metadata exports before exiting";
    }
    int drained = 0;
    while (tryFetchWorkItems() == TryFetchWorkItemsResult::Ready) {
        processJob(std::move(*m_currentJob));
        m_currentJob.reset();
        ++drained;
        pending = pendingJobs();
        if (drained % 10 == 0 || pending == 0) {
            kLogger.info()
                    << "Track metadata exports written during shutdown:" << drained
                    << "- remaining:" << pending;
        }
    }
}

WorkerThread::TryFetchWorkItemsResult TrackMetadataExportThread::tryFetchWorkItems() {
    DEBUG_ASSERT(!m_currentJob.has_value());
    const std::lock_guard lock(m_queueMutex);
    if (m_queue.empty()) {
        return TryFetchWorkItemsResult::Idle;
    }
    m_currentJob = std::move(m_queue.front());
    m_queue.pop_front();
    return TryFetchWorkItemsResult::Ready;
}

int TrackMetadataExportThread::pendingJobs() {
    const std::lock_guard lock(m_queueMutex);
    return static_cast<int>(m_queue.size());
}

void TrackMetadataExportThread::processJob(Job&& job) {
    kLogger.debug()
            << "Exporting track metadata into file:"
            << job.pTrack->getLocation();
    const auto result = SoundSourceProxy::exportTrackMetadataBeforeSaving(
            job.pTrack.get(), job.syncParams);
    if (result == ExportTrackMetadataResult::Failed) {
        // Reset the synchronization timestamp so that the metadata
        // is re-imported from the file on next access, like
        // TrackCollectionManager::saveTrack() does after a failed
        // synchronous export.
        job.pTrack->resetSourceSynchronizedAt();
    }
    const QDateTime sourceSynchronizedAt = job.pTrack->getSourceSynchronizedAt();
    // An invalid timestamp on failure resets the column in the
    // database and causes the file tags to be re-imported on next
    // access.
    if (job.pTrack->getId().isValid() &&
            result != ExportTrackMetadataResult::Skipped) {
        const QSqlDatabase dbConnection =
                mixxx::DbConnectionPooled(m_pDbConnectionPool);
        if (dbConnection.isValid()) {
            updateSourceSynchronizedAt(
                    dbConnection, job.pTrack->getId(), sourceSynchronizedAt);
        }
    }
    emit trackExported(std::move(job.pTrack), result);
}
