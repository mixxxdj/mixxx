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

QStringList DirectoryDAO::getRootDirStrings() const {
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

    QStringList allDirs;
    const auto locationIndex = query.fieldIndex(kLocationColumn);
    while (query.next()) {
        const auto locationValue =
                query.fieldValue(locationIndex).toString();
        allDirs.append(locationValue);
    }
    return allDirs;
}

DirectoryDAO::AddResult DirectoryDAO::addDirectory(
        const mixxx::FileInfo& newDir) const {
    DEBUG_ASSERT(m_database.isOpen());
    if (!newDir.exists() || !newDir.isDir()) {
        kLogger.warning()
                << "Failed to add"
                << newDir.location()
                << ": Directory does not exist or is inaccessible";
        return AddResult::InvalidOrMissingDirectory;
    }
    if (!newDir.isReadable()) {
        kLogger.warning()
                << "Aborting to to add"
                << newDir.location()
                << ": Directory can not be read";
        return AddResult::UnreadableDirectory;
    }
    const auto newCanonicalLocation = newDir.canonicalLocation();
    DEBUG_ASSERT(!newCanonicalLocation.isEmpty());
    QList<mixxx::FileInfo> obsoleteChildDirs;
    // Ignore invalid or missing directories in order to allow adding new dirs
    // while the list contains e.g. currently unmounted removable drives.
    // Worst that can happen is that we have orphan tracks in the database that
    // would be moved to missing after the rescan (which is required anyway after
    // having added a new dir).
    for (auto&& oldDir : loadAllDirectories(true /* ignore missing */)) {
        const auto oldCanonicalLocation = oldDir.canonicalLocation();
        DEBUG_ASSERT(!oldCanonicalLocation.isEmpty());
        if (mixxx::FileInfo::isRootSubCanonicalLocation(
                    oldCanonicalLocation,
                    newCanonicalLocation)) {
            // New dir is a child of an existing dir, return
            return AddResult::AlreadyWatching;
        }
        if (mixxx::FileInfo::isRootSubCanonicalLocation(
                    newCanonicalLocation,
                    oldCanonicalLocation)) {
            // New dir is the parent of an existing dir. Remove the child from
            // the dir list.
            obsoleteChildDirs.append(std::move(oldDir));
        }
    }

    const auto statement =
            QStringLiteral("INSERT INTO %1 (%2) VALUES (:location)")
                    .arg(
                            kTable,
                            kLocationColumn);
    FwdSqlQuery query(m_database, statement);
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
                    << oldDir.location();
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

std::pair<DirectoryDAO::RelocateResult, QList<RelocatedTrack>> DirectoryDAO::relocateDirectory(
        const QString& oldDirectory,
        const QString& newDirectory) const {
    // Don't verify the old directory with
    // DEBUG_ASSERT(oldDirectory == mixxx::FileInfo(oldDirectory).location());
    // The path may have been set on another OS and therefore it will not have a
    //  valid location.
    // Just work with the QString; in case of an invalid path track relocation
    // will simply fail if the database query yields no results.
    const mixxx::FileInfo newFileInfo(newDirectory);
    DEBUG_ASSERT(newDirectory == newFileInfo.location());

    if (!newFileInfo.exists() || !newFileInfo.isDir()) {
        kLogger.warning()
                << "Aborting to relocate"
                << oldDirectory
                << ": "
                << newDirectory
                << "does not exist or is inaccessible";
        return {RelocateResult::InvalidOrMissingDirectory, {}};
    }
    if (!newFileInfo.isReadable()) {
        kLogger.warning()
                << "Aborting to relocate"
                << oldDirectory
                << ": "
                << newDirectory
                << "can not be read";
        return {RelocateResult::UnreadableDirectory, {}};
    }

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
        return {RelocateResult::SqlError, {}};
    }

    // Appending '/' is required to disambiguate files from parent
    // directories, e.g. "a/b.mp3" and "a/b/c.mp3" where "a/b" would
    // match both instead of only files in the parent directory "a/b/".
    QString oldDirectoryPrefix = oldDirectory;
    if (!oldDirectory.endsWith('/')) {
        oldDirectoryPrefix += '/';
    }
    query.prepare(QStringLiteral(
            "SELECT library.id,track_locations.id,track_locations.location "
            "FROM library INNER JOIN track_locations ON "
            "track_locations.id=library.location WHERE "
            "INSTR(track_locations.location,:oldDirectoryPrefix)=1"));
    query.bindValue(":oldDirectoryPrefix", oldDirectoryPrefix);
    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "could not relocate path of tracks";
        return {RelocateResult::SqlError, {}};
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
    const QString replacement =
            "UPDATE track_locations SET location=:newloc, directory=:newdir WHERE id=:id";
    query.prepare(replacement);
    for (int i = 0; i < loc_ids.size(); ++i) {
        query.bindValue(QStringLiteral(":newloc"),
                relocatedTracks.at(i).updatedTrackRef().getLocation());
        query.bindValue(QStringLiteral(":newdir"), newDirectory);
        query.bindValue(QStringLiteral(":id"), loc_ids.at(i).toVariant());
        if (!query.exec()) {
            LOG_FAILED_QUERY(query) << "could not relocate path of track";
            return {RelocateResult::SqlError, relocatedTracks};
        }
    }

    qDebug() << "Relocated tracks:" << relocatedTracks.size();
    return {RelocateResult::Ok, relocatedTracks};
}
