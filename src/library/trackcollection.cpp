#include "library/trackcollection.h"

#include <QApplication>

#include "library/basetrackcache.h"
#include "moc_trackcollection.cpp"
#include "track/globaltrackcache.h"
#include "util/assert.h"
#include "util/db/sqltransaction.h"
#include "util/dnd.h"
#include "util/logger.h"

namespace {

mixxx::Logger kLogger("TrackCollection");

} // anonymous namespace

TrackCollection::TrackCollection(
        QObject* parent,
        const UserSettingsPointer& pConfig)
        : QObject(parent),
          m_analysisDao(pConfig),
          m_trackDao(m_cueDao, m_playlistDao,
                     m_analysisDao, m_libraryHashDao, pConfig) {
    // Forward signals from TrackDAO
    connect(&m_trackDao,
            &TrackDAO::trackClean,
            this,
            &TrackCollection::trackClean,
            /*signal-to-signal*/ Qt::DirectConnection);
    connect(&m_trackDao,
            &TrackDAO::trackDirty,
            this,
            &TrackCollection::trackDirty,
            /*signal-to-signal*/ Qt::DirectConnection);
    connect(&m_trackDao,
            &TrackDAO::tracksAdded,
            this,
            &TrackCollection::tracksAdded,
            /*signal-to-signal*/ Qt::DirectConnection);
    connect(&m_trackDao,
            &TrackDAO::tracksChanged,
            this,
            &TrackCollection::tracksChanged,
            /*signal-to-signal*/ Qt::DirectConnection);
    connect(&m_trackDao,
            &TrackDAO::tracksRemoved,
            this,
            &TrackCollection::tracksRemoved,
            /*signal-to-signal*/ Qt::DirectConnection);
    connect(&m_trackDao,
            &TrackDAO::forceModelUpdate,
            this,
            &TrackCollection::multipleTracksChanged,
            /*signal-to-signal*/ Qt::DirectConnection);
}

TrackCollection::~TrackCollection() {
    if (kLogger.debugEnabled()) {
        kLogger.debug() << "~TrackCollection()";
    }
    // The database should have been detached earlier
    DEBUG_ASSERT(!m_database.isOpen());
}

void TrackCollection::repairDatabase(const QSqlDatabase& database) {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);

    kLogger.info() << "Repairing database";
    m_crates.repairDatabase(database);
}

void TrackCollection::connectDatabase(const QSqlDatabase& database) {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);

    kLogger.info() << "Connecting database";
    m_database = database;
    m_trackDao.initialize(database);
    m_playlistDao.initialize(database);
    m_cueDao.initialize(database);
    m_directoryDao.initialize(database);
    m_analysisDao.initialize(database);
    m_libraryHashDao.initialize(database);
    m_crates.connectDatabase(database);
}

void TrackCollection::disconnectDatabase() {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);

    kLogger.info() << "Disconnecting database";
    m_database = QSqlDatabase();
    m_trackDao.finish();
    m_crates.disconnectDatabase();
}

void TrackCollection::connectTrackSource(QSharedPointer<BaseTrackCache> pTrackSource) {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);

    VERIFY_OR_DEBUG_ASSERT(m_pTrackSource.isNull()) {
        kLogger.warning() << "Track source has already been connected";
        return;
    }
    kLogger.info() << "Connecting track source";
    m_pTrackSource = pTrackSource;
    connect(this,
            &TrackCollection::scanTrackAdded,
            m_pTrackSource.data(),
            &BaseTrackCache::slotScanTrackAdded);
    connect(&m_trackDao,
            &TrackDAO::trackDirty,
            m_pTrackSource.data(),
            &BaseTrackCache::slotTrackDirty);
    connect(&m_trackDao,
            &TrackDAO::trackClean,
            m_pTrackSource.data(),
            &BaseTrackCache::slotTrackClean);
    connect(&m_trackDao,
            &TrackDAO::tracksAdded,
            m_pTrackSource.data(),
            &BaseTrackCache::slotTracksAddedOrChanged);
    connect(&m_trackDao,
            &TrackDAO::tracksChanged,
            m_pTrackSource.data(),
            &BaseTrackCache::slotTracksAddedOrChanged);
    connect(&m_trackDao,
            &TrackDAO::tracksRemoved,
            m_pTrackSource.data(),
            &BaseTrackCache::slotTracksRemoved);
}

QWeakPointer<BaseTrackCache> TrackCollection::disconnectTrackSource() {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);

    auto pWeakPtr = m_pTrackSource.toWeakRef();
    if (m_pTrackSource) {
        kLogger.info() << "Disconnecting track source";
        m_trackDao.disconnect(m_pTrackSource.data());
        m_pTrackSource.reset();
    }
    return pWeakPtr;
}

bool TrackCollection::addDirectory(const QString& dir) {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);

    SqlTransaction transaction(m_database);
    switch (m_directoryDao.addDirectory(dir)) {
    case SQL_ERROR:
        return false;
    case ALREADY_WATCHING:
        return true;
    case ALL_FINE:
        transaction.commit();
        return true;
    default:
        DEBUG_ASSERT("unreachable");
    }
    return false;
}

bool TrackCollection::removeDirectory(const QString& dir) {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);

    SqlTransaction transaction(m_database);
    switch (m_directoryDao.removeDirectory(dir)) {
    case SQL_ERROR:
        return false;
    case ALL_FINE:
        transaction.commit();
        return true;
    default:
        DEBUG_ASSERT("unreachable");
    }
    return false;
}

void TrackCollection::relocateDirectory(const QString& oldDir, const QString& newDir) {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);

    // We only call this method if the user has picked a relocated directory via
    // a file dialog. This means the system sandboxer (if we are sandboxed) has
    // granted us permission to this folder. Create a security bookmark while we
    // have permission so that we can access the folder on future runs. We need
    // to canonicalize the path so we first wrap the directory string with a
    // QDir.
    Sandbox::createSecurityToken(QDir(newDir));

    SqlTransaction transaction(m_database);
    QList<RelocatedTrack> relocatedTracks =
            m_directoryDao.relocateDirectory(oldDir, newDir);
    transaction.commit();

    if (relocatedTracks.isEmpty()) {
        // No tracks moved
        return;
    }

    // Inform the TrackDAO about the changes
    m_trackDao.slotDatabaseTracksRelocated(std::move(relocatedTracks));

    GlobalTrackCacheLocker().relocateCachedTracks(&m_trackDao);
}

QList<TrackId> TrackCollection::resolveTrackIds(
        const QList<TrackFile>& trackFiles,
        TrackDAO::ResolveTrackIdFlags flags) {
    QList<TrackId> trackIds = m_trackDao.resolveTrackIds(trackFiles, flags);
    if (flags & TrackDAO::ResolveTrackIdFlag::UnhideHidden) {
        unhideTracks(trackIds);
    }
    return trackIds;
}

QList<TrackId> TrackCollection::resolveTrackIdsFromUrls(
        const QList<QUrl>& urls, bool addMissing) {
    QList<TrackFile> files = DragAndDropHelper::supportedTracksFromUrls(urls, false, true);
    if (files.isEmpty()) {
        return QList<TrackId>();
    }

    TrackDAO::ResolveTrackIdFlags flags =
            TrackDAO::ResolveTrackIdFlag::UnhideHidden;
    if (addMissing) {
        flags |= TrackDAO::ResolveTrackIdFlag::AddMissing;
    }
    return resolveTrackIds(files, flags);
}

QList<TrackId> TrackCollection::resolveTrackIdsFromLocations(
        const QList<QString>& locations) {
    QList<TrackFile> trackFiles;
    trackFiles.reserve(locations.size());
    for (const QString& location : locations) {
        trackFiles.append(TrackFile(location));
    }
    return resolveTrackIds(trackFiles,
            TrackDAO::ResolveTrackIdFlag::UnhideHidden
                    | TrackDAO::ResolveTrackIdFlag::AddMissing);
}

bool TrackCollection::hideTracks(const QList<TrackId>& trackIds) {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);

    // Warn if tracks have a playlist membership
    QSet<int> allPlaylistIds;
    for (const auto& trackId: trackIds) {
        QSet<int> playlistIds;
        m_playlistDao.getPlaylistsTrackIsIn(trackId, &playlistIds);
        for (const auto& playlistId : qAsConst(playlistIds)) {
            if (m_playlistDao.getHiddenType(playlistId) != PlaylistDAO::PLHT_SET_LOG) {
                allPlaylistIds.insert(playlistId);
            }
        }
    }

    if (!allPlaylistIds.isEmpty()) {
         QStringList playlistNames;
         playlistNames.reserve(allPlaylistIds.count());
         for (const auto& playlistId: allPlaylistIds) {
             playlistNames.append(m_playlistDao.getPlaylistName(playlistId));
         }

         QString playlistNamesSection =
                 "\n\n\"" %
                 playlistNames.join("\"\n\"") %
                 "\"\n\n";

         if (QMessageBox::question(
                 nullptr,
                 tr("Hiding tracks"),
                 tr("The selected tracks are in the following playlists:"
                     "%1"
                     "Hiding them will remove them from these playlists. Continue?")
                         .arg(playlistNamesSection),
                 QMessageBox::Ok | QMessageBox::Cancel) != QMessageBox::Ok) {
             return false;
         }
     }

    // Transactional
    SqlTransaction transaction(m_database);
    VERIFY_OR_DEBUG_ASSERT(transaction) {
        return false;
    }
    VERIFY_OR_DEBUG_ASSERT(m_trackDao.hideTracks(trackIds)) {
        return false;
    }
    VERIFY_OR_DEBUG_ASSERT(transaction.commit()) {
        return false;
    }

    m_playlistDao.removeTracksFromPlaylists(trackIds);

    // Post-processing
    // TODO(XXX): Move signals from TrackDAO to TrackCollection
    m_trackDao.afterHidingTracks(trackIds);
    QSet<CrateId> modifiedCrateSummaries(
            m_crates.collectCrateIdsOfTracks(trackIds));

    // Emit signal(s)
    // TODO(XXX): Emit signals here instead of from DAOs
    emit crateSummaryChanged(modifiedCrateSummaries);

    return true;
}

void TrackCollection::hideAllTracks(const QDir& rootDir) {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);

    m_trackDao.hideAllTracks(rootDir);
}

bool TrackCollection::unhideTracks(const QList<TrackId>& trackIds) {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);

    VERIFY_OR_DEBUG_ASSERT(m_trackDao.unhideTracks(trackIds)) {
        return false;
    }

    // Post-processing
    // TODO(XXX): Move signals from TrackDAO to TrackCollection
    // to update BaseTrackCache
    m_trackDao.afterUnhidingTracks(trackIds);

    // Emit signal(s)
    // TODO(XXX): Emit signals here instead of from DAOs
    // To update labels of CrateFeature, because unhiding might make a
    // crate track visible again.
    QSet<CrateId> modifiedCrateSummaries =
            m_crates.collectCrateIdsOfTracks(trackIds);
    emit crateSummaryChanged(modifiedCrateSummaries);

    return true;
}

bool TrackCollection::purgeTracks(
        const QList<TrackId>& trackIds) {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);

    // Transactional
    SqlTransaction transaction(m_database);
    VERIFY_OR_DEBUG_ASSERT(transaction) {
        return false;
    }
    VERIFY_OR_DEBUG_ASSERT(m_trackDao.onPurgingTracks(trackIds)) {
        return false;
    }
    // Collect crates of tracks that will be purged before actually purging
    // them within the same transactions. Those tracks will be removed from
    // all crates on purging.
    QSet<CrateId> modifiedCrateSummaries(
            m_crates.collectCrateIdsOfTracks(trackIds));
    VERIFY_OR_DEBUG_ASSERT(m_crates.onPurgingTracks(trackIds)) {
        return false;
    }
    VERIFY_OR_DEBUG_ASSERT(transaction.commit()) {
        return false;
    }
    // TODO(XXX): Move reversible actions inside transaction
    m_cueDao.deleteCuesForTracks(trackIds);
    m_playlistDao.removeTracksFromPlaylists(trackIds);
    m_analysisDao.deleteAnalyses(trackIds);

    // Post-processing
    // TODO(XXX): Move signals from TrackDAO to TrackCollection
    m_trackDao.afterPurgingTracks(trackIds);

    // Emit signal(s)
    // TODO(XXX): Emit signals here instead of from DAOs
    emit crateSummaryChanged(modifiedCrateSummaries);

    return true;
}

bool TrackCollection::purgeAllTracks(
        const QDir& rootDir) {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);

    QList<TrackRef> trackRefs = m_trackDao.getAllTrackRefs(rootDir);
    QList<TrackId> trackIds;
    trackIds.reserve(trackRefs.size());
    for (const auto& trackRef : trackRefs) {
        DEBUG_ASSERT(trackRef.hasId());
        trackIds.append(trackRef.getId());
    }
    return purgeTracks(trackIds);
}

bool TrackCollection::insertCrate(
        const Crate& crate,
        CrateId* pCrateId) {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);

    // Transactional
    SqlTransaction transaction(m_database);
    VERIFY_OR_DEBUG_ASSERT(transaction) {
        return false;
    }
    CrateId crateId;
    VERIFY_OR_DEBUG_ASSERT(m_crates.onInsertingCrate(crate, &crateId)) {
        return false;
    }
    DEBUG_ASSERT(crateId.isValid());
    VERIFY_OR_DEBUG_ASSERT(transaction.commit()) {
        return false;
    }

    // Emit signals
    emit crateInserted(crateId);

    if (pCrateId != nullptr) {
        *pCrateId = crateId;
    }
    return true;
}

bool TrackCollection::updateCrate(
        const Crate& crate) {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);

    // Transactional
    SqlTransaction transaction(m_database);
    VERIFY_OR_DEBUG_ASSERT(transaction) {
        return false;
    }
    VERIFY_OR_DEBUG_ASSERT(m_crates.onUpdatingCrate(crate)) {
        return false;
    }
    VERIFY_OR_DEBUG_ASSERT(transaction.commit()) {
        return false;
    }

    // Emit signals
    emit crateUpdated(crate.getId());

    return true;
}

bool TrackCollection::deleteCrate(
        CrateId crateId) {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);

    // Transactional
    SqlTransaction transaction(m_database);
    VERIFY_OR_DEBUG_ASSERT(transaction) {
        return false;
    }
    VERIFY_OR_DEBUG_ASSERT(m_crates.onDeletingCrate(crateId)) {
        return false;
    }
    VERIFY_OR_DEBUG_ASSERT(transaction.commit()) {
        return false;
    }

    // Emit signals
    emit crateDeleted(crateId);

    return true;
}

bool TrackCollection::addCrateTracks(
        CrateId crateId,
        const QList<TrackId>& trackIds) {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);

    // Transactional
    SqlTransaction transaction(m_database);
    VERIFY_OR_DEBUG_ASSERT(transaction) {
        return false;
    }
    VERIFY_OR_DEBUG_ASSERT(m_crates.onAddingCrateTracks(crateId, trackIds)) {
        return false;
    }
    VERIFY_OR_DEBUG_ASSERT(transaction.commit()) {
        return false;
    }

    // Emit signals
    emit crateTracksChanged(crateId, trackIds, QList<TrackId>());

    return true;
}

bool TrackCollection::removeCrateTracks(
        CrateId crateId,
        const QList<TrackId>& trackIds) {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);

    // Transactional
    SqlTransaction transaction(m_database);
    VERIFY_OR_DEBUG_ASSERT(transaction) {
        return false;
    }
    VERIFY_OR_DEBUG_ASSERT(m_crates.onRemovingCrateTracks(crateId, trackIds)) {
        return false;
    }
    VERIFY_OR_DEBUG_ASSERT(transaction.commit()) {
        return false;
    }

    // Emit signals
    emit crateTracksChanged(crateId, QList<TrackId>(), trackIds);

    return true;
}

bool TrackCollection::updateAutoDjCrate(
        CrateId crateId,
        bool isAutoDjSource) {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);

    Crate crate;
    VERIFY_OR_DEBUG_ASSERT(crates().readCrateById(crateId, &crate)) {
        return false; // inexistent or failure
    }
    if (crate.isAutoDjSource() == isAutoDjSource) {
        return false; // nothing to do
    }
    crate.setAutoDjSource(isAutoDjSource);
    return updateCrate(crate);
}

void TrackCollection::saveTrack(Track* pTrack) {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);

    m_trackDao.saveTrack(pTrack);
}

TrackPointer TrackCollection::getTrackById(
        TrackId trackId) const {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);

    return m_trackDao.getTrackById(trackId);
}

TrackPointer TrackCollection::getTrackByRef(
        const TrackRef& trackRef) const {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);

    return m_trackDao.getTrackByRef(trackRef);
}

TrackId TrackCollection::getTrackIdByRef(
        const TrackRef& trackRef) const {
    return m_trackDao.getTrackIdByRef(trackRef);
}

TrackPointer TrackCollection::getOrAddTrack(
        const TrackRef& trackRef,
        bool* pAlreadyInLibrary) {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);

    return m_trackDao.getOrAddTrack(trackRef, pAlreadyInLibrary);
}

TrackId TrackCollection::addTrack(
        const TrackPointer& pTrack,
        bool unremove) {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);

    m_trackDao.addTracksPrepare();
    const auto trackId = m_trackDao.addTracksAddTrack(pTrack, unremove);
    m_trackDao.addTracksFinish(!trackId.isValid());
    return trackId;
}
