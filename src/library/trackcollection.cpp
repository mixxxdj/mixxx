#include "library/trackcollection.h"

#include "database/schemamanager.h"
#include "track/track.h"
#include "util/logger.h"
#include "util/db/sqltransaction.h"

#include "util/assert.h"


// The schema XML is baked into the binary via Qt resources.
//static
const QString TrackCollection::kDefaultSchemaFile(":/schema.xml");

// static
const int TrackCollection::kRequiredSchemaVersion = 27;

namespace {
    mixxx::Logger kLogger("TrackCollection");
} // anonymous namespace

TrackCollection::TrackCollection(
        UserSettingsPointer pConfig)
        : m_dbConnection(pConfig->getSettingsPath()),
          m_analysisDao(pConfig),
          m_trackDao(m_cueDao, m_playlistDao,
                     m_analysisDao, m_libraryHashDao, pConfig) {
}

TrackCollection::~TrackCollection() {
    kLogger.debug() << "~TrackCollection()";
    m_trackDao.finish();
    m_crates.detachDatabase();
}

bool TrackCollection::initDatabaseSchema(
        const QString& schemaFile,
        int schemaVersion) {
    if (!database().isOpen()) {
        QMessageBox::critical(0, tr("Cannot open database"),
                            tr("Unable to establish a database connection.\n"
                                "Mixxx requires QT with SQLite support. Please read "
                                "the Qt SQL driver documentation for information on how "
                                "to build it.\n\n"
                                "Click OK to exit."), QMessageBox::Ok);
        return false; // abort
    }

    QString okToExit = tr("Click OK to exit.");
    QString upgradeFailed = tr("Cannot upgrade database schema");
    QString upgradeToVersionFailed =
            tr("Unable to upgrade your database schema to version %1")
            .arg(QString::number(schemaVersion));
    QString helpEmail = tr("For help with database issues contact:") + "\n" +
                           "mixxx-devel@lists.sourceforge.net";

    SchemaManager schemaManager(database());
    switch (schemaManager.upgradeToSchemaVersion(schemaFile, schemaVersion)) {
        case SchemaManager::Result::CurrentVersion:
        case SchemaManager::Result::UpgradeSucceeded:
        case SchemaManager::Result::NewerVersionBackwardsCompatible:
            m_trackDao.initialize(database());
            m_playlistDao.initialize(database());
            m_cueDao.initialize(database());
            m_directoryDao.initialize(database());
            m_libraryHashDao.initialize(database());
            m_crates.attachDatabase(database());
            return true; // done
        case SchemaManager::Result::UpgradeFailed:
            QMessageBox::warning(
                    0, upgradeFailed,
                    upgradeToVersionFailed + "\n" +
                    tr("Your mixxxdb.sqlite file may be corrupt.") + "\n" +
                    tr("Try renaming it and restarting Mixxx.") + "\n" +
                    helpEmail + "\n\n" + okToExit,
                    QMessageBox::Ok);
            return false; // abort
        case SchemaManager::Result::NewerVersionIncompatible:
            QMessageBox::warning(
                    0, upgradeFailed,
                    upgradeToVersionFailed + "\n" +
                    tr("Your mixxxdb.sqlite file was created by a newer "
                       "version of Mixxx and is incompatible.") +
                    "\n\n" + okToExit,
                    QMessageBox::Ok);
            return false; // abort
        case SchemaManager::Result::SchemaError:
            QMessageBox::warning(
                    0, upgradeFailed,
                    upgradeToVersionFailed + "\n" +
                    tr("The database schema file is invalid.") + "\n" +
                    helpEmail + "\n\n" + okToExit,
                    QMessageBox::Ok);
            return false; // abort
    }
    // Suppress compiler warning
    DEBUG_ASSERT(!"unhandled switch/case");
    return false;
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
    SqlTransaction transaction(database());
    VERIFY_OR_DEBUG_ASSERT(transaction) {
        return false;
    }
    VERIFY_OR_DEBUG_ASSERT(m_trackDao.onHidingTracks(transaction, trackIds)) {
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
    // Transactional
    SqlTransaction transaction(database());
    VERIFY_OR_DEBUG_ASSERT(transaction) {
        return false;
    }
    VERIFY_OR_DEBUG_ASSERT(m_trackDao.onUnhidingTracks(transaction, trackIds)) {
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
    // Transactional
    SqlTransaction transaction(database());
    VERIFY_OR_DEBUG_ASSERT(transaction) {
        return false;
    }
    VERIFY_OR_DEBUG_ASSERT(m_trackDao.onPurgingTracks(transaction, trackIds)) {
        return false;
    }
    // Collect crates of tracks that will be purged before actually purging
    // them within the same transactions. Those tracks will be removed from
    // all crates on purging.
    QSet<CrateId> modifiedCrateSummaries(
            m_crates.collectCrateIdsOfTracks(trackIds));
    VERIFY_OR_DEBUG_ASSERT(m_crates.onPurgingTracks(transaction, trackIds)) {
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
    // Transactional
    SqlTransaction transaction(database());
    VERIFY_OR_DEBUG_ASSERT(transaction) {
        return false;
    }
    CrateId crateId;
    VERIFY_OR_DEBUG_ASSERT(m_crates.onInsertingCrate(transaction, crate, &crateId)) {
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
    // Transactional
    SqlTransaction transaction(database());
    VERIFY_OR_DEBUG_ASSERT(transaction) {
        return false;
    }
    VERIFY_OR_DEBUG_ASSERT(m_crates.onUpdatingCrate(transaction, crate)) {
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
    // Transactional
    SqlTransaction transaction(database());
    VERIFY_OR_DEBUG_ASSERT(transaction) {
        return false;
    }
    VERIFY_OR_DEBUG_ASSERT(m_crates.onDeletingCrate(transaction, crateId)) {
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
    // Transactional
    SqlTransaction transaction(database());
    VERIFY_OR_DEBUG_ASSERT(transaction) {
        return false;
    }
    VERIFY_OR_DEBUG_ASSERT(m_crates.onAddingCrateTracks(transaction, crateId, trackIds)) {
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
    // Transactional
    SqlTransaction transaction(database());
    VERIFY_OR_DEBUG_ASSERT(transaction) {
        return false;
    }
    VERIFY_OR_DEBUG_ASSERT(m_crates.onRemovingCrateTracks(transaction, crateId, trackIds)) {
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
