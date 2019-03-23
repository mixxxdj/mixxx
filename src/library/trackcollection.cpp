#include <QStringBuilder>
#include <QThread>
#include <QApplication>

#include "library/trackcollection.h"

#include "sources/soundsourceproxy.h"
#include "track/globaltrackcache.h"
#include "util/logger.h"
#include "util/db/sqltransaction.h"

#include "util/assert.h"


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

    QSet<TrackId> movedIds(
            m_directoryDao.relocateDirectory(oldDir, newDir));

    m_trackDao.databaseTracksMoved(std::move(movedIds), QSet<TrackId>());

    GlobalTrackCacheLocker().relocateCachedTracks(&m_trackDao);
}

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
    VERIFY_OR_DEBUG_ASSERT(m_trackDao.onHidingTracks(trackIds)) {
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

    // Transactional
    SqlTransaction transaction(m_database);
    VERIFY_OR_DEBUG_ASSERT(transaction) {
        return false;
    }
    VERIFY_OR_DEBUG_ASSERT(m_trackDao.onUnhidingTracks(trackIds)) {
        return false;
    }
    VERIFY_OR_DEBUG_ASSERT(transaction.commit()) {
        return false;
    }

    // Post-processing
    // TODO(XXX): Move signals from TrackDAO to TrackCollection
    m_trackDao.afterUnhidingTracks(trackIds);
    // TODO(XXX): Move signals from TrackDAO to TrackCollection

    // Emit signal(s)
    // TODO(XXX): Emit signals here instead of from DAOs
    QSet<CrateId> modifiedCrateSummaries(
            m_crates.collectCrateIdsOfTracks(trackIds));
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

bool TrackCollection::purgeTracks(
        const QDir& dir) {
    QList<TrackId> trackIds(m_trackDao.getTrackIds(dir));
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
