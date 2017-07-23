#include <QStringBuilder>

#include "library/trackcollection.h"

#include "track/track.h"
#include "util/logger.h"
#include "util/db/sqltransaction.h"

#include "util/assert.h"


namespace {
    mixxx::Logger kLogger("TrackCollection");
} // anonymous namespace

TrackCollection::TrackCollection(
        const UserSettingsPointer& pConfig)
        : m_analysisDao(pConfig),
          m_trackDao(m_cueDao, m_playlistDao,
                     m_analysisDao, m_libraryHashDao, pConfig) {
}

TrackCollection::~TrackCollection() {
    kLogger.debug() << "~TrackCollection()";
    // The database should have been detached earlier
    DEBUG_ASSERT(!m_database.isOpen());
}

void TrackCollection::repairDatabase(QSqlDatabase database) {
    m_crates.repairDatabase(database);
}

void TrackCollection::connectDatabase(QSqlDatabase database) {
    m_database = database;
    m_trackDao.initialize(database);
    m_playlistDao.initialize(database);
    m_cueDao.initialize(database);
    m_directoryDao.initialize(database);
    m_analysisDao.initialize(database);
    m_libraryHashDao.initialize(database);
    m_savedDao.initialize(database);
    m_crates.connectDatabase(database);
}

void TrackCollection::disconnectDatabase() {
    m_database = QSqlDatabase();
    m_trackDao.finish();
    m_crates.disconnectDatabase();
}

void TrackCollection::setTrackSource(QSharedPointer<BaseTrackCache> pTrackSource) {
    VERIFY_OR_DEBUG_ASSERT(m_pTrackSource.isNull()) {
        return;
    }
    m_pTrackSource = pTrackSource;
}

void TrackCollection::relocateDirectory(QString oldDir, QString newDir) {
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

    // Clear cache to that all TIO with the old dir information get updated
    m_trackDao.clearCache();
    m_trackDao.databaseTracksMoved(std::move(movedIds), QSet<TrackId>());
}

bool TrackCollection::hideTracks(const QList<TrackId>& trackIds) {
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
      m_crates.tracks().collectCrateIdsOfTracks(trackIds));

    // Emit signal(s)
    // TODO(XXX): Emit signals here instead of from DAOs
    //emit(m_crates.crateSummaryChanged(modifiedCrateSummaries));

    return true;
}

bool TrackCollection::unhideTracks(const QList<TrackId>& trackIds) {
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
      m_crates.tracks().collectCrateIdsOfTracks(trackIds));
    //emit(m_crates.crateSummaryChanged(modifiedCrateSummaries));

    return true;
}

bool TrackCollection::purgeTracks(
        const QList<TrackId>& trackIds) {
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
      m_crates.tracks().collectCrateIdsOfTracks(trackIds));
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
    //emit(m_crates.crateSummaryChanged(modifiedCrateSummaries));

    return true;
}

bool TrackCollection::purgeTracks(
        const QDir& dir) {
    QList<TrackId> trackIds(m_trackDao.getTrackIds(dir));
    return purgeTracks(trackIds);
}
