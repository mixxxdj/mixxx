#include <QStringBuilder>
#include <QThread>
#include <QApplication>

#include "library/trackcollection.h"

#include "sources/soundsourceproxy.h"
#include "track/globaltrackcache.h"
#include "util/logger.h"
#include "util/db/sqltransaction.h"

#include "util/assert.h"
#include "util/dnd.h"

namespace {
    mixxx::Logger kLogger("TrackCollection");
} // anonymous namespace

TrackCollection::TrackCollection(
        const UserSettingsPointer& pConfig)
        : m_pConfig(pConfig),
          m_analysisDao(pConfig),
          m_trackDao(m_cueDao, m_playlistDao,
                     m_analysisDao, m_libraryHashDao, pConfig) {
}

TrackCollection::~TrackCollection() {
    if (kLogger.debugEnabled()) {
        kLogger.debug() << "~TrackCollection()";
    }
    // The database should have been detached earlier
    DEBUG_ASSERT(!m_database.isOpen());
}

void TrackCollection::repairDatabase(QSqlDatabase database) {
    DEBUG_ASSERT(QApplication::instance()->thread() == QThread::currentThread());

    m_crates.repairDatabase(database);
}

void TrackCollection::connectDatabase(QSqlDatabase database) {
    DEBUG_ASSERT(QApplication::instance()->thread() == QThread::currentThread());

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
    DEBUG_ASSERT(QApplication::instance()->thread() == QThread::currentThread());

    m_database = QSqlDatabase();
    m_trackDao.finish();
    m_crates.disconnectDatabase();
}

void TrackCollection::setTrackSource(QSharedPointer<BaseTrackCache> pTrackSource) {
    DEBUG_ASSERT(QApplication::instance()->thread() == QThread::currentThread());

    VERIFY_OR_DEBUG_ASSERT(m_pTrackSource.isNull()) {
        return;
    }
    m_pTrackSource = pTrackSource;
}

bool TrackCollection::addDirectory(const QString& dir) {
    SqlTransaction transaction(m_database);
    switch (m_directoryDao.addDirectory(dir)) {
    case SQL_ERROR:
        return false;
    case ALREADY_WATCHING:
        return true;
    case ALL_FINE:
        transaction.commit();
        return true;
    }
    DEBUG_ASSERT("unreachable");
    return false;
}

void TrackCollection::relocateDirectory(QString oldDir, QString newDir) {
    DEBUG_ASSERT(QApplication::instance()->thread() == QThread::currentThread());

    // We only call this method if the user has picked a relocated directory via
    // a file dialog. This means the system sandboxer (if we are sandboxed) has
    // granted us permission to this folder. Create a security bookmark while we
    // have permission so that we can access the folder on future runs. We need
    // to canonicalize the path so we first wrap the directory string with a
    // QDir.
    QDir directory(newDir);
    Sandbox::createSecurityToken(directory);

    SqlTransaction transaction(m_database);
    QList<TrackRef> movedTrackRefs =
            m_directoryDao.relocateDirectory(oldDir, newDir);
    transaction.commit();

    QList<QPair<TrackRef, TrackRef>> replacedTrackRefs;
    replacedTrackRefs.reserve(movedTrackRefs.size());
    for (const auto& movedTrackRef : movedTrackRefs) {
        auto removedTrackRef = movedTrackRef;
        // The actual new location is unknown, only the id remains the same
        auto changedTrackRef = TrackRef::fromFileInfo(TrackFile(), movedTrackRef.getId());
        replacedTrackRefs.append(qMakePair(removedTrackRef, changedTrackRef));
    }
    m_trackDao.databaseTracksReplaced(std::move(replacedTrackRefs));

    GlobalTrackCacheLocker().relocateCachedTracks(&m_trackDao);
}

QList<TrackId> TrackCollection::resolveTrackIds(
        const QList<QFileInfo>& files, TrackDAO::ResolveTrackIdFlags flags) {
    QList<TrackId> trackIds = m_trackDao.resolveTrackIds(files, flags);
    if (flags & TrackDAO::ResolveTrackIdFlag::UnhideHidden) {
        unhideTracks(trackIds);
    }
    return trackIds;
}

QList<TrackId> TrackCollection::resolveTrackIdsFromUrls(
        const QList<QUrl>& urls, bool addMissing) {
    QList<QFileInfo> files = DragAndDropHelper::supportedTracksFromUrls(urls, false, true);
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
    QList<QFileInfo> fileInfoList;
    foreach(QString fileLocation, locations) {
        QFileInfo fileInfo(fileLocation);
        fileInfoList.append(fileInfo);
    }
    return resolveTrackIds(fileInfoList,
            TrackDAO::ResolveTrackIdFlag::UnhideHidden
                    | TrackDAO::ResolveTrackIdFlag::AddMissing);
}

QList<TrackId> resolveTrackIdsFromUrls(const QList<QUrl>& urls,
        TrackDAO::ResolveTrackIdFlags flags);

bool TrackCollection::hideTracks(const QList<TrackId>& trackIds) {
    DEBUG_ASSERT(QApplication::instance()->thread() == QThread::currentThread());

    // Warn if tracks have a playlist membership
    QSet<int> allPlaylistIds;
    for (const auto& trackId: trackIds) {
        QSet<int> playlistIds;
        m_playlistDao.getPlaylistsTrackIsIn(trackId, &playlistIds);
        for (const auto& playlistId: playlistIds) {
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
    emit(crateSummaryChanged(modifiedCrateSummaries));

    return true;
}

bool TrackCollection::unhideTracks(const QList<TrackId>& trackIds) {
    DEBUG_ASSERT(QApplication::instance()->thread() == QThread::currentThread());

    VERIFY_OR_DEBUG_ASSERT(m_trackDao.unhideTracks(trackIds)) {
        return false;
    }

    // Post-processing
    // TODO(XXX): Move signals from TrackDAO to TrackCollection
    // To update BaseTrackCache
    m_trackDao.afterUnhidingTracks(trackIds);
    // TODO(XXX): Move signals from TrackDAO to TrackCollection

    // Emit signal(s)
    // TODO(XXX): Emit signals here instead of from DAOs
    // To update labels of CrateFeature, because unhiding might make a
    // crate track visible again.
    QSet<CrateId> modifiedCrateSummaries =
            m_crates.collectCrateIdsOfTracks(trackIds);
    emit(crateSummaryChanged(modifiedCrateSummaries));

    return true;
}

bool TrackCollection::purgeTracks(
        const QList<TrackId>& trackIds) {
    DEBUG_ASSERT(QApplication::instance()->thread() == QThread::currentThread());

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
    emit(crateSummaryChanged(modifiedCrateSummaries));

    return true;
}

bool TrackCollection::purgeAllTracks(
        const QDir& rootDir) {
    QList<TrackId> trackIds = m_trackDao.getAllTrackIds(rootDir);
    return purgeTracks(trackIds);
}

bool TrackCollection::insertCrate(
        const Crate& crate,
        CrateId* pCrateId) {
    DEBUG_ASSERT(QApplication::instance()->thread() == QThread::currentThread());

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
    emit(crateInserted(crateId));

    if (pCrateId != nullptr) {
        *pCrateId = crateId;
    }
    return true;
}

bool TrackCollection::updateCrate(
        const Crate& crate) {
    DEBUG_ASSERT(QApplication::instance()->thread() == QThread::currentThread());

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
    emit(crateUpdated(crate.getId()));

    return true;
}

bool TrackCollection::deleteCrate(
        CrateId crateId) {
    DEBUG_ASSERT(QApplication::instance()->thread() == QThread::currentThread());

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
    emit(crateDeleted(crateId));

    return true;
}

bool TrackCollection::addCrateTracks(
        CrateId crateId,
        const QList<TrackId>& trackIds) {
    DEBUG_ASSERT(QApplication::instance()->thread() == QThread::currentThread());

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
    emit(crateTracksChanged(crateId, trackIds, QList<TrackId>()));

    return true;
}

bool TrackCollection::removeCrateTracks(
        CrateId crateId,
        const QList<TrackId>& trackIds) {
    DEBUG_ASSERT(QApplication::instance()->thread() == QThread::currentThread());

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
    emit(crateTracksChanged(crateId, QList<TrackId>(), trackIds));

    return true;
}

bool TrackCollection::updateAutoDjCrate(
        CrateId crateId,
        bool isAutoDjSource) {
    DEBUG_ASSERT(QApplication::instance()->thread() == QThread::currentThread());

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

void TrackCollection::exportTrackMetadata(Track* pTrack) const {
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
        SoundSourceProxy::exportTrackMetadataBeforeSaving(pTrack);
    }
}

void TrackCollection::saveTrack(Track* pTrack) {
    DEBUG_ASSERT(QApplication::instance()->thread() == QThread::currentThread());

    m_trackDao.saveTrack(pTrack);
}
