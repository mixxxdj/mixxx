#include "aoide/trackreplacementscheduler.h"

#include "aoide/gateway.h"
#include "aoide/trackexport.h"
#include "library/trackloader.h"
#include "util/logger.h"
#include "util/thread_affinity.h"

namespace {

const mixxx::Logger kLogger("aoide TrackReplacementScheduler");

const int kMaxLoading = 8;

// The JSON representation of a track has a size of 2 to 5 kB depending
// on the amount of metadata. Batching requests help to reduce network
// traffic with a small chance that a whole batch might fail. Within
// a batch individual tracks might be rejected or skipped by the server
// without failing the whole batch.
// TODO:
//  - Define an initial batch size, e.g. 8 items
//  - Define a max. batch size, e.g. 64 items (~ 5kB serialized JSON data per track)
//  - Define a max. latency, e.g. 500 ms
//  - Measure actual mean latency as a moving average
//  - Dynamically either halve or double the batch size
//    after measuring a sufficient number of latencies with
//    the current batch size if the actual mean latency
//    is too high or low respectively.
const int kBatchSize = 64;

// 2 pending batches + some slots for loading tracks
const int kMaxPending = 2 * (kMaxLoading + kBatchSize);

} // anonymous namespace

namespace aoide {

TrackReplacementScheduler::TrackReplacementScheduler(
        Gateway* gateway,
        mixxx::TrackLoader* trackLoader,
        QObject* parent)
        : QObject(parent),
          m_gateway(gateway),
          m_trackLoader(trackLoader),
          m_pendingCounter(0),
          m_succeededCounter(0),
          m_failedCounter(0) {
    m_loadingTrackRefs.reserve(kMaxLoading);
    DEBUG_ASSERT(kMaxLoading <= kMaxPending);
    DEBUG_ASSERT(kBatchSize <= kMaxPending);
    // We explicitly use a queued connection, because the trackLoaded
    // signal must be received from the event loop to avoid infinitely
    // nested signal/slot cascades!
    connect(m_trackLoader,
            &mixxx::TrackLoader::trackLoaded,
            this,
            &TrackReplacementScheduler::onTrackLoaded,
            Qt::QueuedConnection);
}

bool TrackReplacementScheduler::isLoading(const TrackRef& trackRef) const {
    int count = m_loadingTrackRefs.count(trackRef);
    DEBUG_ASSERT(count <= 1);
    return count > 0;
}

bool TrackReplacementScheduler::enterLoading(const TrackRef& trackRef) {
    if (isLoading(trackRef)) {
        return false;
    } else {
        m_loadingTrackRefs.append(trackRef);
        return true;
    }
}

bool TrackReplacementScheduler::leaveLoading(const TrackRef& trackRef) {
    int removed = m_loadingTrackRefs.removeAll(trackRef);
    DEBUG_ASSERT(removed <= 1);
    return removed > 0;
}

void TrackReplacementScheduler::scheduleReplaceTracks(
        QString collectionUid,
        json::MediaSourceConfig collectionMediaSourceConfig,
        QList<TrackRef> trackRefs) {
    QMetaObject::invokeMethod(
            this,
            "slotReplaceTracks",
            Qt::QueuedConnection, // async
            Q_ARG(QString, std::move(collectionUid)),
            Q_ARG(QJsonValue, std::move(collectionMediaSourceConfig.intoQJsonValue())),
            Q_ARG(QList<TrackRef>, std::move(trackRefs)));
}

void TrackReplacementScheduler::slotReplaceTracks(
        QString collectionUid,
        const QJsonValue& collectionMediaSourceConfigJson,
        const QList<TrackRef>& trackRefs) {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    VERIFY_OR_DEBUG_ASSERT(!collectionUid.isEmpty()) {
        kLogger.warning()
                << "Cannot replace"
                << trackRefs.size()
                << "tracks without a collection UID";
    }
    DEBUG_ASSERT(collectionMediaSourceConfigJson.isObject());
    if (m_collectionUid.isEmpty() || (m_collectionUid == collectionUid)) {
        auto collectionMediaSourceConfig =
                json::MediaSourceConfig(collectionMediaSourceConfigJson.toObject());
        // The configuration must not change while exporting tracks
        DEBUG_ASSERT(m_collectionUid != collectionUid ||
                m_collectionMediaSourceConfig == collectionMediaSourceConfig);
        m_collectionUid = std::move(collectionUid);
        m_collectionMediaSourceConfig = std::move(collectionMediaSourceConfig);
        m_queuedTrackRefs.append(trackRefs);
        makeProgress();
    } else {
        kLogger.debug()
                << "Deferring replacement of"
                << trackRefs.size()
                << "tracks in different collection"
                << collectionUid;
        m_deferredRequests.append(std::make_pair(std::move(collectionUid), std::move(trackRefs)));
    }
}

void TrackReplacementScheduler::invokeCancel() {
    QMetaObject::invokeMethod(
            this,
            "slotCancel",
            Qt::QueuedConnection); // async
}

void TrackReplacementScheduler::slotCancel() {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    m_deferredRequests.clear();
    m_collectionUid.clear();
    m_queuedTrackRefs.clear();
    makeProgress();
}

void TrackReplacementScheduler::onTrackLoaded(TrackRef trackRef, TrackPointer trackPtr) {
    if (!leaveLoading(trackRef)) {
        if (kLogger.debugEnabled()) {
            kLogger.debug() << "Ignoring loaded track" << trackRef;
        }
        return;
    }

    if (!trackPtr) {
        kLogger.warning() << "Failed to load track" << trackRef;
        DEBUG_ASSERT(m_pendingCounter > 0);
        --m_pendingCounter;
        ++m_failedCounter;
        emitProgress();
        return;
    }
    const auto fileInfo = trackPtr->getFileInfo();
    if (!fileInfo.exists()) {
        kLogger.warning() << "Track file is not accessible" << fileInfo;
        DEBUG_ASSERT(m_pendingCounter > 0);
        --m_pendingCounter;
        ++m_failedCounter;
        emitProgress();
        return;
    }
    if (!m_gateway || m_collectionUid.isEmpty()) {
        kLogger.warning() << "Not connected or no active collection - skipping track" << trackRef;
        DEBUG_ASSERT(m_pendingCounter > 0);
        --m_pendingCounter;
        ++m_failedCounter;
        emitProgress();
        return;
    }

    m_bufferedRequests += exportTrack(
            m_collectionMediaSourceConfig,
            fileInfo,
            trackPtr->getRecord(),
            trackPtr->getCuePoints());
    DEBUG_ASSERT(m_bufferedRequests.size() <= m_pendingCounter);
    if (m_bufferedRequests.size() >= kBatchSize ||
            (m_queuedTrackRefs.isEmpty() && m_loadingTrackRefs.isEmpty())) {
        // Collect the buffered requests into a batch request
        // and and start the task
        auto* task =
                m_gateway->replaceTracks(
                        m_collectionUid,
                        m_bufferedRequests);
        m_bufferedRequests.clear();
        connect(task,
                &ReplaceCollectedTracksTask::networkError,
                this,
                &TrackReplacementScheduler::slotReplaceTracksNetworkError,
                Qt::UniqueConnection);
        connect(task,
                &ReplaceCollectedTracksTask::aborted,
                this,
                &TrackReplacementScheduler::slotReplaceTracksAborted,
                Qt::UniqueConnection);
        connect(task,
                &ReplaceCollectedTracksTask::failed,
                this,
                &TrackReplacementScheduler::slotReplaceTracksFailed,
                Qt::UniqueConnection);
        connect(task,
                &ReplaceCollectedTracksTask::succeeded,
                this,
                &TrackReplacementScheduler::slotReplaceTracksSucceeded,
                Qt::UniqueConnection);
        task->invokeStart();
    }

    makeProgress();
}

void TrackReplacementScheduler::slotReplaceTracksSucceeded(
        QJsonObject result) {
    auto* task = qobject_cast<ReplaceCollectedTracksTask*>(sender());
    VERIFY_OR_DEBUG_ASSERT(task) {
        return;
    }
    task->deleteLater();
    int created = 0;
    if (result.contains("created")) {
        DEBUG_ASSERT(result.value("created").isArray());
        created = result.value("created").toArray().size();
        DEBUG_ASSERT(created >= 0);
    }
    int updated = 0;
    if (result.contains("updated")) {
        DEBUG_ASSERT(result.value("updated").isArray());
        updated = result.value("updated").toArray().size();
        DEBUG_ASSERT(updated >= 0);
    }
    int unchanged = 0;
    if (result.contains("unchanged")) {
        DEBUG_ASSERT(result.value("unchanged").isArray());
        unchanged = result.value("unchanged").toArray().size();
        DEBUG_ASSERT(unchanged >= 0);
    }
    const int succeeded = created + updated + unchanged;
    DEBUG_ASSERT(succeeded == task->size());
    m_succeededCounter += succeeded;
    if (result.contains("notCreated")) {
        DEBUG_ASSERT(result.value("notCreated").isArray());
        const int notCreated = result.value("notCreated").toArray().size();
        Q_UNUSED(notCreated)
        DEBUG_ASSERT(notCreated == 0); // none expected
    }
    if (result.contains("notUpdated")) {
        DEBUG_ASSERT(result.value("notUpdated").isArray());
        const int notUpdated = result.value("notUpdated").toArray().size();
        Q_UNUSED(notUpdated)
        DEBUG_ASSERT(notUpdated == 0); // none expected
    }
    DEBUG_ASSERT(m_pendingCounter >= task->size());
    m_pendingCounter -= task->size();
    if (kLogger.debugEnabled()) {
        kLogger.debug()
                << "Replaced"
                << (created + updated)
                << "of"
                << task->size()
                << "track(s):"
                << created << "created +"
                << updated << "updated +"
                << unchanged << "unchanged";
    }
    emitProgress();
    makeProgress();
}

void TrackReplacementScheduler::slotReplaceTracksNetworkError(
        QNetworkReply::NetworkError errorCode,
        const QString& errorString,
        const mixxx::network::WebResponseWithContent& responseWithContent) {
    Q_UNUSED(responseWithContent);
    auto* task = qobject_cast<ReplaceCollectedTracksTask*>(sender());
    VERIFY_OR_DEBUG_ASSERT(task) {
        return;
    }
    task->deleteLater();
    kLogger.warning()
            << "Failed to replace"
            << task->size()
            << "tracks:"
            << errorCode
            << errorString;
    DEBUG_ASSERT(m_pendingCounter >= task->size());
    m_pendingCounter -= task->size();
    m_failedCounter += task->size();
    emitProgress();
    makeProgress();
}

void TrackReplacementScheduler::slotReplaceTracksAborted() {
    auto* task = qobject_cast<ReplaceCollectedTracksTask*>(sender());
    VERIFY_OR_DEBUG_ASSERT(task) {
        return;
    }
    task->deleteLater();
    kLogger.warning()
            << "Aborted request to replace"
            << task->size()
            << "tracks";
    DEBUG_ASSERT(m_pendingCounter >= task->size());
    m_pendingCounter -= task->size();
    m_failedCounter += task->size();
    emitProgress();
    makeProgress();
}

void TrackReplacementScheduler::slotReplaceTracksFailed(
        mixxx::network::JsonWebResponse response) {
    auto* task = qobject_cast<ReplaceCollectedTracksTask*>(sender());
    VERIFY_OR_DEBUG_ASSERT(task) {
        return;
    }
    task->deleteLater();
    kLogger.warning()
            << "Failed to replace"
            << task->size()
            << "tracks"
            << response;
    DEBUG_ASSERT(m_pendingCounter >= task->size());
    m_pendingCounter -= task->size();
    m_failedCounter += task->size();
    emitProgress();
    makeProgress();
}

void TrackReplacementScheduler::makeProgress() {
    while (!m_collectionUid.isEmpty()) {
        while (!m_queuedTrackRefs.isEmpty() &&
                m_loadingTrackRefs.size() < kMaxLoading &&
                m_pendingCounter < kMaxPending) {
            TrackRef trackRef = m_queuedTrackRefs.dequeue();
            if (enterLoading(trackRef)) {
                VERIFY_OR_DEBUG_ASSERT(m_trackLoader) {
                    kLogger.warning() << "Cannot load track";
                    dumpObjectInfo();
                    ++m_failedCounter;
                    break;
                }
                ++m_pendingCounter;
                m_trackLoader->invokeSlotLoadTrack(std::move(trackRef));
            } else {
                // We can safely skip this dequeued track
                if (kLogger.debugEnabled()) {
                    kLogger.debug()
                            << "Track is already loading"
                            << trackRef;
                }
            }
        }
        DEBUG_ASSERT(m_pendingCounter >= 0);
        DEBUG_ASSERT(m_succeededCounter >= 0);
        DEBUG_ASSERT(m_failedCounter >= 0);
        if (m_queuedTrackRefs.isEmpty() && m_pendingCounter == 0) {
            // Idle -> reset
            m_collectionUid.clear();
            m_succeededCounter = 0;
            m_failedCounter = 0;
        }
        emitProgress();
        if (m_collectionUid.isEmpty() && !m_deferredRequests.isEmpty()) {
            // Idle -> next batch
            DEBUG_ASSERT(m_queuedTrackRefs.isEmpty());
            auto nextBatch = m_deferredRequests.dequeue();
            m_collectionUid = std::move(nextBatch.first);
            m_queuedTrackRefs.append(std::move(nextBatch.second));
        } else {
            // Continue with the event loop
            return;
        }
    }
}

void TrackReplacementScheduler::emitProgress() {
    const int queued = m_queuedTrackRefs.size();
    if (kLogger.debugEnabled()) {
        kLogger.debug() << "Emitting progress"
                        << ": queued" << queued << "/ pending" << m_pendingCounter << "/ succeeded"
                        << m_succeededCounter << "/ failed" << m_failedCounter;
    }
    emit progress(queued, m_pendingCounter, m_succeededCounter, m_failedCounter);
}

} // namespace aoide
