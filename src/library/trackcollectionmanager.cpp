#include "database/mixxxdb.h"

#include "library/trackcollectionmanager.h"

#include "library/trackcollection.h"
#include "library/externaltrackcollection.h"

#include "sources/soundsourceproxy.h"
#include "util/db/dbconnectionpooled.h"
#include "util/logger.h"
#include "util/assert.h"

namespace {

const mixxx::Logger kLogger("TrackCollectionManager");

const QString kConfigGroup("[TrackCollection]");

const ConfigKey kConfigKeyRepairDatabaseOnNextRestart(kConfigGroup, "RepairDatabaseOnNextRestart");

} // anonymous namespace

TrackCollectionManager::TrackCollectionManager(
        QObject* parent,
        UserSettingsPointer pConfig,
        mixxx::DbConnectionPoolPtr pDbConnectionPool,
        deleteTrackFn_t /*only-needed-for-testing*/ deleteTrackFn)
    : QObject(parent),
      m_pConfig(pConfig),
      m_pInternalCollection(make_parented<TrackCollection>(this, pConfig)),
      m_scanner(pDbConnectionPool, m_pInternalCollection, pConfig) {

    const QSqlDatabase dbConnection = mixxx::DbConnectionPooled(std::move(pDbConnectionPool));

    // TODO(XXX): Add a checkbox in the library preferences for checking
    // and repairing the database on the next restart of the application.
    if (pConfig->getValue(kConfigKeyRepairDatabaseOnNextRestart, false)) {
        kLogger.info() << "Checking and repairing database (if necessary)";
        m_pInternalCollection->repairDatabase(dbConnection);
        // Reset config value
        pConfig->setValue(kConfigKeyRepairDatabaseOnNextRestart, false);
    }

    kLogger.info() << "Connecting database";
    m_pInternalCollection->connectDatabase(dbConnection);

    if (deleteTrackFn) {
        kLogger.info() << "External collections are not available in test mode";
    } else {
        // TODO: Add external collections
    }

    kLogger.info() << "Connecting external collections";
    for (const auto& externalCollection : m_externalCollections) {
        kLogger.info() << "Connecting" << externalCollection->name();
        externalCollection->establishConnection();
    }

    // Forward signals
    connect(&m_scanner,
            &LibraryScanner::scanStarted,
            this,
            &TrackCollectionManager::libraryScanStarted);
    connect(&m_scanner,
            &LibraryScanner::scanFinished,
            this,
            &TrackCollectionManager::libraryScanFinished);

    // Handle signals
    connect(&m_scanner,
            &LibraryScanner::trackAdded,
            this,
            &TrackCollectionManager::slotScanTrackAdded);
    connect(&m_scanner,
            &LibraryScanner::tracksChanged,
            this,
            &TrackCollectionManager::slotScanTracksUpdated);
    connect(&m_scanner,
            &LibraryScanner::tracksReplaced,
            this,
            &TrackCollectionManager::slotScanTracksReplaced);

    GlobalTrackCache::createInstance(this, deleteTrackFn);
}

TrackCollectionManager::~TrackCollectionManager() {
    const auto pWeakTrackSource = m_pInternalCollection->disconnectTrackSource();
    VERIFY_OR_DEBUG_ASSERT(pWeakTrackSource.isNull()) {
        kLogger.warning() << "BaseTrackCache is still in use";
    }

    // Evict all remaining tracks from the cache to trigger
    // updating of modified tracks. We assume that no other
    // components are accessing those files at this point.
    kLogger.info() << "Deactivating GlobalTrackCache";
    GlobalTrackCacheLocker().deactivateCache();

    if (!m_externalCollections.isEmpty()) {
        kLogger.info() << "Disconnecting from external track collections";
        for (const auto& externalCollection : m_externalCollections) {
            kLogger.info() << "Disconnecting from" << externalCollection->name();
            externalCollection->finishPendingTasksAndDisconnect();
        }
    }

    kLogger.info() << "Disconnecting internal track collection from database";
    m_pInternalCollection->disconnectDatabase();

    GlobalTrackCache::destroyInstance();
}

void TrackCollectionManager::startLibraryScan() {
    m_scanner.scan();
}

void TrackCollectionManager::stopLibraryScan() {
    m_scanner.slotCancel();
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
// and external libaries.
void TrackCollectionManager::saveEvictedTrack(Track* pTrack) noexcept {
    saveTrack(pTrack, TrackMetadataExportMode::Immediate);
}

void TrackCollectionManager::saveTrack(
        Track* pTrack,
        TrackMetadataExportMode mode) {
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
    return m_pInternalCollection->addDirectory(dir);
}

bool TrackCollectionManager::removeDirectory(const QString& dir) {
    return m_pInternalCollection->removeDirectory(dir);
}

void TrackCollectionManager::relocateDirectory(QString oldDir, QString newDir) {
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
    return m_pInternalCollection->hideTracks(trackIds);
}

bool TrackCollectionManager::unhideTracks(const QList<TrackId>& trackIds) {
    return m_pInternalCollection->unhideTracks(trackIds);
}

void TrackCollectionManager::hideAllTracks(const QDir& rootDir) {
    m_pInternalCollection->hideAllTracks(rootDir);
}

void TrackCollectionManager::purgeTracks(const QList<TrackId>& trackIds) {
    if (trackIds.isEmpty()) {
        return;
    }
    // Collect the corresponding track locations BEFORE purging the
    // tracks from the internal collection!
    QList<QString> trackLocations;
    if (!m_externalCollections.isEmpty()) {
        trackLocations =
                m_pInternalCollection->getTrackDAO().getTrackLocations(trackIds);
    }
    DEBUG_ASSERT(trackLocations.size() <= trackIds.size());
    kLogger.debug()
            << "Purging"
            << trackIds.size()
            << "tracks from internal collection";
    if (!m_pInternalCollection->purgeTracks(trackIds)) {
        kLogger.warning()
                << "Failed to purge tracks from internal collection";
        return;
    }
    if (m_externalCollections.isEmpty()) {
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(trackLocations.size() == trackIds.size()) {
        kLogger.warning()
                << "Purging only"
                << trackLocations.size()
                << "of"
                << trackIds.size()
                << "tracks from"
                << m_externalCollections.size()
                << "external collection(s)";
    } else {
        kLogger.debug()
                << "Purging"
                << trackLocations.size()
                << "tracks from"
                << m_externalCollections.size()
                << "external collection(s)";
    }
    for (const auto& externalTrackCollection : m_externalCollections) {
        externalTrackCollection->purgeTracks(trackLocations);
    }
}

void TrackCollectionManager::purgeAllTracks(const QDir& rootDir) {
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

void TrackCollectionManager::slotScanTracksReplaced(QList<QPair<TrackRef, TrackRef>> replacedTracks) {
    // Already replaced in m_pInternalCollection
    if (m_externalCollections.isEmpty()) {
        return;
    }
    QList<ExternalTrackCollection::DuplicateTrack> duplicateTracks;
    duplicateTracks.reserve(replacedTracks.size());
    for (const auto& replacedTrack : replacedTracks) {
        ExternalTrackCollection::DuplicateTrack duplicateTrack;
        duplicateTrack.removed = replacedTrack.first;
        duplicateTrack.replacedBy = replacedTrack.second;
        duplicateTracks.append(duplicateTrack);
    }
    kLogger.debug()
            << "Deduplicating"
            << duplicateTracks.size()
            << "replaced track(s) in"
            << m_externalCollections.size()
            << "external collection(s)";
    for (const auto& externalTrackCollection : m_externalCollections) {
        externalTrackCollection->deduplicateTracks(duplicateTracks);
    }
}
