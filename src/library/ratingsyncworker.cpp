#include "library/ratingsyncworker.h"

#include "library/library_prefs.h"
#include "library/trackcollectionmanager.h"
#include "moc_ratingsyncworker.cpp"
#include "sources/metadatasourcetaglib.h"
#include "track/track.h"
#include "util/logger.h"

namespace mixxx {

namespace {
const Logger kLogger("RatingSyncWorker");
} // anonymous namespace

RatingSyncWorker::RatingSyncWorker(
        TrackCollectionManager* pTrackCollectionManager,
        UserSettingsPointer pConfig,
        QObject* parent)
        : QThread(parent),
          m_pTrackCollectionManager(pTrackCollectionManager),
          m_pConfig(pConfig),
          m_stop(false) {
}

RatingSyncWorker::~RatingSyncWorker() {
    stop();
    wait();
}

void RatingSyncWorker::stop() {
    QMutexLocker lock(&m_mutex);
    m_stop = true;
    m_waitCondition.wakeAll();
}

void RatingSyncWorker::syncRatingsForTracks(const QList<TrackId>& trackIds) {
    QMutexLocker lock(&m_mutex);
    // Replace pending tracks (user clicked a different playlist/crate)
    m_pendingTrackIds = trackIds;
    m_waitCondition.wakeOne();
}

void RatingSyncWorker::run() {
    kLogger.debug() << "Rating sync worker started";

    while (true) {
        QList<TrackId> trackIdsToProcess;

        {
            QMutexLocker lock(&m_mutex);
            while (m_pendingTrackIds.isEmpty() && !m_stop) {
                m_waitCondition.wait(&m_mutex);
            }
            if (m_stop) {
                kLogger.debug() << "Rating sync worker stopping";
                return;
            }
            trackIdsToProcess = m_pendingTrackIds;
            m_pendingTrackIds.clear();
        }

        int tracksUpdated = 0;
        int totalTracks = trackIdsToProcess.size();

        kLogger.debug()
                << "Starting rating sync for"
                << totalTracks
                << "tracks";

        for (const TrackId& trackId : trackIdsToProcess) {
            // Check for stop request between tracks
            {
                QMutexLocker lock(&m_mutex);
                if (m_stop) {
                    return;
                }
                // If new tracks were queued, abort current batch
                // and process the new ones instead
                if (!m_pendingTrackIds.isEmpty()) {
                    kLogger.debug()
                            << "Aborting current sync, new playlist/crate selected";
                    break;
                }
            }

            // Load track from cache/database
            TrackPointer pTrack = m_pTrackCollectionManager->getTrackById(trackId);
            if (!pTrack) {
                continue;
            }

            // Get file location and type for metadata source
            const QString location = pTrack->getLocation();
            const QString type = pTrack->getType();

            // Create metadata source to read rating from file
            MetadataSourceTagLib metadataSource(location, type);
            std::optional<int> fileRating = metadataSource.importRating();

            if (fileRating.has_value()) {
                int currentRating = pTrack->getRating();
                int newRating = fileRating.value();

                // File tags are source of truth - always update if different
                if (newRating != currentRating) {
                    pTrack->setRating(newRating);
                    ++tracksUpdated;
                    kLogger.debug()
                            << "Updated rating for"
                            << location
                            << "from" << currentRating
                            << "to" << newRating;
                }
            }
        }

        kLogger.debug()
                << "Rating sync complete:"
                << tracksUpdated
                << "of"
                << totalTracks
                << "tracks updated";

        emit syncComplete(tracksUpdated, totalTracks);
    }
}

} // namespace mixxx
