#include "library/dao/directorydao.h"

#include <QDir>

#include "library/queryutil.h"
#include "util/db/fwdsqlquery.h"
#include "util/logger.h"

namespace {

const mixxx::Logger kLogger("DirectoryDAO");

const QString kTable = QStringLiteral("directories");
const QString kLocationColumn = QStringLiteral("directory");

} // anonymous namespace

QList<mixxx::FileInfo> DirectoryDAO::loadAllDirectories(
        bool skipInvalidOrMissing) const {
    DEBUG_ASSERT(m_database.isOpen());
    const auto statement =
            QStringLiteral("SELECT %1 FROM %2")
                    .arg(
                            kLocationColumn,
                            kTable);
    FwdSqlQuery query(
            m_database,
            statement);
    VERIFY_OR_DEBUG_ASSERT(query.execPrepared()) {
        return {};
    }

    QList<mixxx::FileInfo> allDirs;
    const auto locationIndex = query.fieldIndex(kLocationColumn);
    while (query.next()) {
        const auto locationValue =
                query.fieldValue(locationIndex).toString();
        auto fileInfo = mixxx::FileInfo(locationValue);
        if (skipInvalidOrMissing) {
            if (!fileInfo.exists() || !fileInfo.isDir()) {
                kLogger.debug()
                        << "Skipping to load invalid or missing directory"
                        << fileInfo;
                continue;
            }
        }
        allDirs.append(std::move(fileInfo));
    }
    return allDirs;
}

DirectoryDAO::AddResult DirectoryDAO::addDirectory(
        const mixxx::FileInfo& newDir) const {
    DEBUG_ASSERT(m_database.isOpen());
    if (!newDir.exists() || !newDir.isDir()) {
        kLogger.warning()
                << "Failed to add"
                << newDir
                << ": Directory does not exist or is inaccessible";
        return AddResult::InvalidOrMissingDirectory;
    }
    const auto newCanonicalLocation = newDir.canonicalLocation();
    DEBUG_ASSERT(!newCanonicalLocation.isEmpty());
    QList<mixxx::FileInfo> obsoleteChildDirs;
    for (auto&& oldDir : loadAllDirectories()) {
        if (!oldDir.exists() || !oldDir.isDir()) {
            // Abort to prevent inconsistencies in the database
            kLogger.warning()
                    << "Aborting to add"
                    << newDir
                    << ": Loaded directory"
                    << oldDir
                    << "does not exist or is inaccessible";
            return AddResult::InvalidOrMissingDirectory;
        }
        const auto oldCanonicalLocation = oldDir.canonicalLocation();
        DEBUG_ASSERT(!oldCanonicalLocation.isEmpty());
        if (mixxx::FileInfo::isRootSubCanonicalLocation(
                    oldCanonicalLocation,
                    newCanonicalLocation)) {
            return AddResult::AlreadyWatching;
        }
        if (mixxx::FileInfo::isRootSubCanonicalLocation(
                    newCanonicalLocation,
                    oldCanonicalLocation)) {
            obsoleteChildDirs.append(std::move(oldDir));
        }
    }

    const auto statement =
            QStringLiteral("INSERT INTO %1 (%2) VALUES (:location)")
                    .arg(
                            kTable,
                            kLocationColumn);
    FwdSqlQuery query(
            m_database,
            statement);
    query.bindValue(
            QStringLiteral(":location"),
            newDir.location());
    VERIFY_OR_DEBUG_ASSERT(query.execPrepared()) {
        return AddResult::SqlError;
    }

    for (const auto& oldDir : obsoleteChildDirs) {
        if (RemoveResult::Ok != removeDirectory(oldDir)) {
            kLogger.warning()
                    << "Failed to remove obsolete child directory"
                    << oldDir;
            DEBUG_ASSERT(!"removeDirectory failed");
            continue;
        }
    }

    return AddResult::Ok;
}

DirectoryDAO::RemoveResult DirectoryDAO::removeDirectory(
        const mixxx::FileInfo& oldDir) const {
    DEBUG_ASSERT(m_database.isOpen());
    const auto statement =
            QStringLiteral("DELETE FROM %1 WHERE %2=:location")
                    .arg(
                            kTable,
                            kLocationColumn);
    FwdSqlQuery query(
            m_database,
            statement);
    query.bindValue(
            QStringLiteral(":location"),
            oldDir.location());
    VERIFY_OR_DEBUG_ASSERT(query.execPrepared()) {
        return RemoveResult::SqlError;
    }

    if (query.numRowsAffected() < 1) {
        return RemoveResult::NotFound;
    }
    DEBUG_ASSERT(query.numRowsAffected() == 1);

    return RemoveResult::Ok;
}

QList<RelocatedTrack> DirectoryDAO::relocateDirectory(
        const QString& oldDirectory,
        const QString& newDirectory) const {
    DEBUG_ASSERT(oldDirectory == mixxx::FileInfo(oldDirectory).location());
    DEBUG_ASSERT(newDirectory == mixxx::FileInfo(newDirectory).location());
    // TODO(rryan): This method could use error reporting. It can fail in
    // mysterious ways for example if a track in the oldDirectory also has a zombie
    // track location in newDirectory then the replace query will fail because the
    // location column becomes non-unique.
    QSqlQuery query(m_database);
    query.prepare("UPDATE " % kTable % " SET " % kLocationColumn %
            "=:newDirectory WHERE " % kLocationColumn % "=:oldDirectory");
    query.bindValue(":newDirectory", newDirectory);
    query.bindValue(":oldDirectory", oldDirectory);
    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "could not relocate directory"
                                << oldDirectory << "to" << newDirectory;
        return {};
    }

    // Appending '/' is required to disambiguate files from parent
    // directories, e.g. "a/b.mp3" and "a/b/c.mp3" where "a/b" would
    // match both instead of only files in the parent directory "a/b/".
    DEBUG_ASSERT(!oldDirectory.endsWith('/'));
    const QString oldDirectoryPrefix = oldDirectory + '/';
    query.prepare(QStringLiteral(
            "SELECT library.id,track_locations.id,track_locations.location "
            "FROM library INNER JOIN track_locations ON "
            "track_locations.id=library.location WHERE "
            "INSTR(track_locations.location,:oldDirectoryPrefix)=1"));
    query.bindValue(":oldDirectoryPrefix", oldDirectoryPrefix);
    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "could not relocate path of tracks";
        return {};
    }

    QList<DbId> loc_ids;
    QList<RelocatedTrack> relocatedTracks;
    while (query.next()) {
        const auto oldLocation = query.value(2).toString();
        const int oldSuffixLen = oldLocation.size() - oldDirectory.size();
        QString newLocation = newDirectory + oldLocation.right(oldSuffixLen);
        DEBUG_ASSERT(oldLocation.startsWith(oldDirectoryPrefix));
        loc_ids.append(DbId(query.value(1)));
        const auto trackId = TrackId(query.value(0));
        auto missingTrackRef = TrackRef::fromFilePath(
                oldLocation,
                std::move(trackId));
        auto addedTrackRef = TrackRef::fromFilePath(
                newLocation); // without TrackId, because no new track will be added!
        relocatedTracks.append(RelocatedTrack(
                std::move(missingTrackRef),
                std::move(addedTrackRef)));
    }

    // Also update information in the track_locations table. This is where mixxx
    // gets the location information for a track.
    const QString replacement = "UPDATE track_locations SET location=:newloc WHERE id=:id";
    query.prepare(replacement);
    for (int i = 0; i < loc_ids.size(); ++i) {
        query.bindValue("newloc", relocatedTracks.at(i).updatedTrackRef().getLocation());
        query.bindValue("id", loc_ids.at(i).toVariant());
        if (!query.exec()) {
            LOG_FAILED_QUERY(query) << "could not relocate path of tracks";
            return {};
        }
    }

    qDebug() << "Relocated tracks:" << relocatedTracks.size();
    return relocatedTracks;
}
