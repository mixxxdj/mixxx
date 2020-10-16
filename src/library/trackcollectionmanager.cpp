#include "library/trackcollectionmanager.h"

#include "library/externaltrackcollection.h"
#include "library/scanner/libraryscanner.h"
#include "library/trackcollection.h"
#include "sources/soundsourceproxy.h"
#include "track/track.h"
#include "util/assert.h"
#include "util/db/dbconnectionpooled.h"
#include "util/logger.h"

namespace {

const mixxx::Logger kLogger("TrackCollectionManager");

const QString kConfigGroup = QStringLiteral("[TrackCollection]");

const ConfigKey kConfigKeyRepairDatabaseOnNextRestart(kConfigGroup, "RepairDatabaseOnNextRestart");

inline
parented_ptr<TrackCollection> createInternalTrackCollection(
        TrackCollectionManager* parent,
        const UserSettingsPointer& pConfig,
        deleteTrackFn_t deleteTrackFn) {
    // Ensure that GlobalTrackCache is ready before creating
    // the internal TrackCollection.
    GlobalTrackCache::createInstance(parent, deleteTrackFn);
    return make_parented<TrackCollection>(parent, pConfig);
}

} // anonymous namespace

TrackCollectionManager::TrackCollectionManager(
        QObject* parent,
        UserSettingsPointer pConfig,
        mixxx::DbConnectionPoolPtr pDbConnectionPool,
        deleteTrackFn_t /*only-needed-for-testing*/ deleteTrackForTestingFn)
    : QObject(parent),
      m_pConfig(pConfig),
      m_pInternalCollection(createInternalTrackCollection(this, pConfig, deleteTrackForTestingFn)) {
    const QSqlDatabase dbConnection = mixxx::DbConnectionPooled(pDbConnectionPool);

    // TODO(XXX): Add a checkbox in the library preferences for checking
    // and repairing the database on the next restart of the application.
    if (pConfig->getValue(kConfigKeyRepairDatabaseOnNextRestart, false)) {
        m_pInternalCollection->repairDatabase(dbConnection);
        // Reset config value
        pConfig->setValue(kConfigKeyRepairDatabaseOnNextRestart, false);
    }

    m_pInternalCollection->connectDatabase(dbConnection);

    if (deleteTrackForTestingFn) {
        kLogger.info() << "External collections are disabled in test mode";
    } else {
        // TODO: Add external collections
    }
    for (const auto& externalCollection : m_externalCollections) {
        kLogger.info()
                << "Connecting to"
                << externalCollection->name();
        externalCollection->establishConnection();
    }

    // TODO: Extract and decouple LibraryScanner from TrackCollectionManager
    if (deleteTrackForTestingFn) {
        // Exclude the library scanner from tests
        kLogger.info() << "Library scanner is disabled in test mode";
    } else {
        m_pScanner = std::make_unique<LibraryScanner>(pDbConnectionPool, pConfig);

        // Forward signals
        connect(m_pScanner.get(),
                &LibraryScanner::scanStarted,
                this,
                &TrackCollectionManager::libraryScanStarted,
                /*signal-to-signal*/ Qt::DirectConnection);
        connect(m_pScanner.get(),
                &LibraryScanner::scanFinished,
                this,
                &TrackCollectionManager::libraryScanFinished,
                /*signal-to-signal*/ Qt::DirectConnection);

        // Handle signals
        connect(m_pScanner.get(),
                &LibraryScanner::trackAdded,
                this,
                &TrackCollectionManager::slotScanTrackAdded);
        connect(m_pScanner.get(),
                &LibraryScanner::tracksChanged,
                this,
                &TrackCollectionManager::slotScanTracksUpdated);
        connect(m_pScanner.get(),
                &LibraryScanner::tracksRelocated,
                this,
                &TrackCollectionManager::slotScanTracksRelocated);

        // Force the GUI thread's Track cache to be cleared when a library
        // scan is finished, because we might have modified the database directly
        // when we detected moved files, and the track objects and table entries
        // corresponding to the moved files would then have the wrong track location.
        TrackDAO* pTrackDAO = &(m_pInternalCollection->getTrackDAO());
        connect(m_pScanner.get(),
                &LibraryScanner::tracksChanged,
                pTrackDAO,
                &TrackDAO::slotDatabaseTracksChanged);
        connect(m_pScanner.get(),
                &LibraryScanner::tracksRelocated,
                pTrackDAO,
                &TrackDAO::slotDatabaseTracksRelocated);

        kLogger.info() << "Starting library scanner thread";
        m_pScanner->start();
    }
}

TrackCollectionManager::~TrackCollectionManager() {
    if (m_pScanner) {
        while (m_pScanner->isRunning()) {
            kLogger.info() << "Stopping library scanner thread";
            m_pScanner->quit();
            if (m_pScanner->wait()) {
                kLogger.info() << "Stopped library scanner thread";
            }
        }
        DEBUG_ASSERT(m_pScanner->isFinished());
        m_pScanner.reset();
    }

    const auto pWeakTrackSource = m_pInternalCollection->disconnectTrackSource();
    VERIFY_OR_DEBUG_ASSERT(pWeakTrackSource.isNull()) {
        kLogger.warning() << "BaseTrackCache is still in use";
    }

    // Evict all remaining tracks from the cache to trigger
    // updating of modified tracks. We assume that no other
    // components are accessing those files at this point.
    GlobalTrackCacheLocker().deactivateCache();

    for (const auto& externalCollection : m_externalCollections) {
        kLogger.info()
                << "Disconnecting from"
                << externalCollection->name();
        // TODO: Disconnecting from external track collections
        // should be done asynchrously. The manager should poll
        // the track collections until all have been disconnected.
        externalCollection->finishPendingTasksAndDisconnect(); // synchronous
    }

    m_pInternalCollection->disconnectDatabase();

    GlobalTrackCache::destroyInstance();
}

void TrackCollectionManager::startLibraryScan() {
    DEBUG_ASSERT(m_pScanner);
    m_pScanner->scan();
}

void TrackCollectionManager::stopLibraryScan() {
    DEBUG_ASSERT(m_pScanner);
    m_pScanner->slotCancel();
}

bool TrackCollectionManager::saveTrack(const TrackPointer& pTrack) {
    VERIFY_OR_DEBUG_ASSERT(pTrack) {
        return false;
    }
    if (!pTrack->isDirty()) {
        return false;
    }
    saveTrack(pTrack.get(), TrackMetadataExportMode::Deferred);
    DEBUG_ASSERT(!pTrack->isDirty());
    return true;
}

// Export metadata and save the track in both the internal database
// and external libraries.
void TrackCollectionManager::saveEvictedTrack(Track* pTrack) noexcept {
    saveTrack(pTrack, TrackMetadataExportMode::Immediate);
}

void TrackCollectionManager::saveTrack(
        Track* pTrack,
        TrackMetadataExportMode mode) {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    DEBUG_ASSERT(pTrack);
    DEBUG_ASSERT(pTrack->getDateAdded().isValid());

    // The metadata must be exported while the cache is locked to
    // ensure that we have exclusive (write) access on the file
    // and not reader or writer is accessing the same file
    // concurrently.
    exportTrackMetadata(pTrack, mode);

    // The dirty flag is reset while saving the track in the internal
    // collection!
    const bool trackDirty = pTrack->isDirty();

    // This operation must be executed synchronously while the cache is
    // locked to prevent that a new track is created from outdated
    // metadata in the database before saving finished.
    kLogger.debug()
            << "Saving track"
            << pTrack->getLocation()
            << "in internal collection";
    m_pInternalCollection->saveTrack(pTrack);

    if (m_externalCollections.isEmpty()) {
        return;
    }
    if (pTrack->getId().isValid()) {
        // Track still exists in the internal collection/database
        if (trackDirty) {
            kLogger.debug()
                    << "Saving modified track"
                    << pTrack->getLocation()
                    << "in"
                    << m_externalCollections.size()
                    << "external collection(s)";
            for (const auto& externalTrackCollection : m_externalCollections) {
                externalTrackCollection->saveTrack(
                        *pTrack,
                        ExternalTrackCollection::ChangeHint::Modified);
            }
        }
    } else {
        // Track has been deleted from the internal collection/database
        // while it was cached in-memory
        kLogger.debug()
                << "Purging deleted track"
                << pTrack->getLocation()
                << "from"
                << m_externalCollections.size()
                << "external collection(s)";
        for (const auto& externalTrackCollection : m_externalCollections) {
            externalTrackCollection->purgeTracks(
                    QStringList{pTrack->getLocation()});
        }
    }
}

void TrackCollectionManager::exportTrackMetadata(
        Track* pTrack,
        TrackMetadataExportMode mode) const {
    DEBUG_ASSERT(pTrack);

    // Write audio meta data, if explicitly requested by the user
    // for individual tracks or enabled in the preferences for all
    // tracks.
    //
    // This must be done before updating the database, because
    // a timestamp is used to keep track of when metadata has been
    // last synchronized. Exporting metadata will update this time
    // stamp on the track object!
    if (pTrack->isMarkedForMetadataExport() ||
            (pTrack->isDirty() && m_pConfig && m_pConfig->getValueString(ConfigKey("[Library]","SyncTrackMetadataExport")).toInt() == 1)) {
        switch (mode) {
        case TrackMetadataExportMode::Immediate:
            // Export track metadata now by saving as file tags.
            SoundSourceProxy::exportTrackMetadataBeforeSaving(pTrack);
            break;
        case TrackMetadataExportMode::Deferred:
            // Export track metadata later when the track object goes out
            // of scope and we have exclusive file access. This is required
            // unconditionally, even if the dirty flag is not set again!
            // Use case: Keep all track collection up-to-date while tracks
            // are still loaded in memory to allow frequent synchronization
            // of external collections. Local caching is error prone and not
            // always feasible.
            pTrack->markForMetadataExport();
            break;
        }
    }
}

bool TrackCollectionManager::addDirectory(const QString& dir) {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);

    return m_pInternalCollection->addDirectory(dir);
}

bool TrackCollectionManager::removeDirectory(const QString& dir) {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);

    return m_pInternalCollection->removeDirectory(dir);
}

void TrackCollectionManager::relocateDirectory(QString oldDir, QString newDir) {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);

    kLogger.debug()
            << "Relocating directory in internal track collection:"
            << oldDir
            << "->"
            << newDir;
    // TODO(XXX): Add error handling in TrackCollection::relocateDirectory()
    m_pInternalCollection->relocateDirectory(oldDir, newDir);
    if (m_externalCollections.isEmpty()) {
        return;
    }
    kLogger.debug()
            << "Relocating directory in"
            << m_externalCollections.size()
            << "external track collection(s):"
            << oldDir
            << "->"
            << newDir;
    for (const auto& externalTrackCollection : m_externalCollections) {
        externalTrackCollection->relocateDirectory(oldDir, newDir);
    }
}

bool TrackCollectionManager::hideTracks(const QList<TrackId>& trackIds) {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);

    return m_pInternalCollection->hideTracks(trackIds);
}

bool TrackCollectionManager::unhideTracks(const QList<TrackId>& trackIds) {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);

    return m_pInternalCollection->unhideTracks(trackIds);
}

void TrackCollectionManager::hideAllTracks(const QDir& rootDir) {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);

    m_pInternalCollection->hideAllTracks(rootDir);
}

void TrackCollectionManager::purgeTracks(const QList<TrackRef>& trackRefs) {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);

    if (trackRefs.isEmpty()) {
        return;
    }
    kLogger.debug()
            << "trackRefs"
            << trackRefs.size()
            << "tracks from internal collection";
    {
        QList<TrackId> trackIds;
        trackIds.reserve(trackRefs.size());
        for (const auto& trackRef : trackRefs) {
            DEBUG_ASSERT(trackRef.hasId());
            trackIds.append(trackRef.getId());
        }
        if (!m_pInternalCollection->purgeTracks(trackIds)) {
            kLogger.warning()
                    << "Failed to purge tracks from internal collection";
            return;
        }
    }
    if (m_externalCollections.isEmpty()) {
        return;
    }
    QList<QString> trackLocations;
    trackLocations.reserve(trackLocations.size());
    for (const auto& trackRef : trackRefs) {
        DEBUG_ASSERT(trackRef.hasLocation());
        trackLocations.append(trackRef.getLocation());
    }
    kLogger.debug()
            << "Purging"
            << trackLocations.size()
            << "tracks from"
            << m_externalCollections.size()
            << "external collection(s)";
    for (const auto& externalTrackCollection : m_externalCollections) {
        externalTrackCollection->purgeTracks(trackLocations);
    }
}

void TrackCollectionManager::purgeAllTracks(const QDir& rootDir) {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);

    kLogger.debug()
            << "Purging directory"
            << rootDir
            << "from internal track collection";
    if (!m_pInternalCollection->purgeAllTracks(rootDir)) {
        kLogger.warning()
                << "Failed to purge directory from internal collection";
        return;
    }
    if (m_externalCollections.isEmpty()) {
        return;
    }
    kLogger.debug()
            << "Purging directory"
            << rootDir
            << "from"
            << m_externalCollections.size()
            << "external track collection(s)";
    for (const auto& externalTrackCollection : m_externalCollections) {
        externalTrackCollection->purgeAllTracks(rootDir);
    }
}

TrackPointer TrackCollectionManager::getOrAddTrack(
        const TrackRef& trackRef,
        bool* pAlreadyInLibrary) {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);

    bool alreadyInLibrary;
    if (pAlreadyInLibrary) {
        alreadyInLibrary = *pAlreadyInLibrary;
    }
    // Forward call to internal collection
    auto pTrack = m_pInternalCollection->getOrAddTrack(trackRef, &alreadyInLibrary);
    if (pAlreadyInLibrary) {
        *pAlreadyInLibrary = alreadyInLibrary;
    }
    if (pTrack && !alreadyInLibrary) {
        // Add to external libraries
        slotScanTrackAdded(pTrack);
    }
    return pTrack;
}

void TrackCollectionManager::slotScanTrackAdded(TrackPointer pTrack) {
    DEBUG_ASSERT(pTrack);
    DEBUG_ASSERT(pTrack->getDateAdded().isValid());

    // Already added to m_pInternalCollection
    if (m_externalCollections.isEmpty()) {
        return;
    }
    kLogger.debug()
            << "Adding new track"
            << pTrack->getLocation()
            << "to"
            << m_externalCollections.size()
            << "external track collection(s)";
    for (const auto& externalTrackCollection : m_externalCollections) {
        externalTrackCollection->saveTrack(*pTrack, ExternalTrackCollection::ChangeHint::Added);
    }
}

void TrackCollectionManager::slotScanTracksUpdated(QSet<TrackId> updatedTrackIds) {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);

    // Already updated in m_pInternalCollection
    if (updatedTrackIds.isEmpty()) {
        return;
    }
    if (m_externalCollections.isEmpty()) {
        return;
    }
    QList<TrackRef> trackRefs;
    trackRefs.reserve(updatedTrackIds.size());
    for (const auto& trackId : updatedTrackIds) {
        auto trackLocation = m_pInternalCollection->getTrackDAO().getTrackLocation(trackId);
        if (!trackLocation.isEmpty()) {
            trackRefs.append(TrackRef::fromFileInfo(trackLocation, trackId));
        }
    }
    DEBUG_ASSERT(trackRefs.size() <= updatedTrackIds.size());
    VERIFY_OR_DEBUG_ASSERT(trackRefs.size() == updatedTrackIds.size()) {
        kLogger.warning()
                << "Updating only"
                << trackRefs.size()
                << "of"
                << updatedTrackIds.size()
                << "track(s) in"
                << m_externalCollections.size()
                << "external collection(s)";
    } else {
        kLogger.debug()
                << "Updating"
                << trackRefs.size()
                << "track(s) in"
                << m_externalCollections.size()
                << "external collection(s)";
    }
    for (const auto& externalTrackCollection : m_externalCollections) {
        externalTrackCollection->updateTracks(trackRefs);
    }
}

void TrackCollectionManager::slotScanTracksRelocated(
        QList<RelocatedTrack> relocatedTracks) {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);

    // Already replaced in m_pInternalCollection
    if (m_externalCollections.isEmpty()) {
        return;
    }
    kLogger.debug()
            << "Relocating"
            << relocatedTracks.size()
            << "track(s) in"
            << m_externalCollections.size()
            << "external collection(s)";
    for (const auto& externalTrackCollection : m_externalCollections) {
        externalTrackCollection->relocateTracks(relocatedTracks);
    }
}
