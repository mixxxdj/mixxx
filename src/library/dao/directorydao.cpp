#include "library/dao/directorydao.h"

#include <QDir>

#include "library/queryutil.h"
#include "util/db/fwdsqlquery.h"
#include "util/db/sqllikewildcardescaper.h"
#include "util/db/sqllikewildcards.h"
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
        VERIFY_OR_DEBUG_ASSERT(RemoveResult::Ok == removeDirectory(oldDir)) {
            kLogger.warning()
                    << "Failed to remove obsolete child directory"
                    << oldDir;
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
        const QString& oldFolder,
        const QString& newFolder) const {
    // TODO(rryan): This method could use error reporting. It can fail in
    // mysterious ways for example if a track in the oldFolder also has a zombie
    // track location in newFolder then the replace query will fail because the
    // location column becomes non-unique.
    QSqlQuery query(m_database);
    query.prepare("UPDATE " % kTable % " SET " % kLocationColumn %
            "=:newFolder WHERE " % kLocationColumn % " = :oldFolder");
    query.bindValue(":newFolder", newFolder);
    query.bindValue(":oldFolder", oldFolder);
    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "could not relocate directory"
                                << oldFolder << "to" << newFolder;
        return {};
    }

    // on Windows the absolute path starts with the drive name
    // we also need to check for that
    QString startsWithOldFolder = SqlLikeWildcardEscaper::apply(
                                          QDir(oldFolder).absolutePath() + "/", kSqlLikeMatchAll) +
            kSqlLikeMatchAll;

    // Also update information in the track_locations table. This is where mixxx
    // gets the location information for a track. Put marks around %1 so that
    // this also works on windows
    query.prepare(QString(
            "SELECT library.id, track_locations.id, track_locations.location "
            "FROM library INNER JOIN track_locations ON "
            "track_locations.id = library.location WHERE "
            "track_locations.location LIKE '%1' ESCAPE '%2'")
                          .arg(startsWithOldFolder, kSqlLikeMatchAll));
    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "could not relocate path of tracks";
        return {};
    }

    QList<DbId> loc_ids;
    QList<RelocatedTrack> relocatedTracks;
    while (query.next()) {
        loc_ids.append(DbId(query.value(1).toInt()));
        auto trackId = TrackId(query.value(0));
        auto oldLocation = query.value(2).toString();
        auto missingTrackRef = TrackRef::fromFileInfo(
                TrackFile(oldLocation),
                std::move(trackId));
        const int oldSuffixLen = oldLocation.size() - oldFolder.size();
        QString newLocation = newFolder + oldLocation.right(oldSuffixLen);
        auto addedTrackRef = TrackRef::fromFileInfo(
                TrackFile(newLocation) /*without TrackId*/);
        relocatedTracks.append(RelocatedTrack(
                std::move(missingTrackRef),
                std::move(addedTrackRef)));
    }

    QString replacement =
            "UPDATE track_locations SET location = :newloc "
            "WHERE id = :id";
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
