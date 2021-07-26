#include "library/dao/directorydao.h"

#include <QDir>
#include <QtDebug>

#include "library/queryutil.h"
#include "util/db/sqllikewildcardescaper.h"
#include "util/db/sqllikewildcards.h"
#include "util/db/sqlstringformatter.h"

namespace {

bool isChildDir(const QString& testDir, const QString& dirStr) {
    QDir test = QDir(testDir);
    QDir dir = QDir(dirStr);
    bool child = dir == test;
    while (test.cdUp()) {
        if (dir == test) {
            child = true;
        }
    }
    // qDebug() << "--- test related function ---";
    // qDebug() << "testDir " << testDir;
    // qDebug() << "dir" << dirStr;
    // qDebug() << "child = " << child;
    // qDebug() << "-----------------------------";
    return child;
}

} // anonymous namespace

int DirectoryDAO::addDirectory(const QString& newDir) const {
    // Do nothing if the dir to add is a child of a directory that is already in
    // the db.
    QStringList dirs = getDirs();
    QString childDir;
    QString parentDir;
    foreach (const QString& dir, dirs) {
        if (isChildDir(newDir, dir)) {
            childDir = dir;
        }
        if (isChildDir(dir, newDir)) {
            parentDir = dir;
        }
    }

    if (!childDir.isEmpty()) {
        qDebug() << "return already watching";
        return ALREADY_WATCHING;
    }

    if (!parentDir.isEmpty()) {
        // removing the old directory won't harm because we are adding the
        // parent later in this function
        removeDirectory(parentDir);
    }

    QSqlQuery query(m_database);
    query.prepare("INSERT INTO " % DIRECTORYDAO_TABLE %
                  " (" % DIRECTORYDAO_DIR % ") VALUES (:dir)");
    query.bindValue(":dir", newDir);
    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "Adding new dir (" % newDir % ") failed.";
        return SQL_ERROR;
    }
    return ALL_FINE;
}

int DirectoryDAO::removeDirectory(const QString& dir) const {
    QSqlQuery query(m_database);
    query.prepare("DELETE FROM " % DIRECTORYDAO_TABLE  % " WHERE "
                   % DIRECTORYDAO_DIR % "= :dir");
    query.bindValue(":dir", dir);
    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "purging dir (" % dir % ") failed";
        return SQL_ERROR;
    }
    return ALL_FINE;
}

QList<RelocatedTrack> DirectoryDAO::relocateDirectory(
        const QString& oldFolder,
        const QString& newFolder) const {
    DEBUG_ASSERT(oldFolder == QDir(oldFolder).absolutePath());
    DEBUG_ASSERT(newFolder == QDir(newFolder).absolutePath());
    // TODO(rryan): This method could use error reporting. It can fail in
    // mysterious ways for example if a track in the oldFolder also has a zombie
    // track location in newFolder then the replace query will fail because the
    // location column becomes non-unique.
    QSqlQuery query(m_database);
    query.prepare("UPDATE " % DIRECTORYDAO_TABLE % " SET " % DIRECTORYDAO_DIR %
            "=:newFolder WHERE " % DIRECTORYDAO_DIR % "=:oldFolder");
    query.bindValue(":newFolder", newFolder);
    query.bindValue(":oldFolder", oldFolder);
    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "could not relocate directory"
                                << oldFolder << "to" << newFolder;
        return {};
    }

    // Appending '/' is required to disambiguate files from parent
    // directories, e.g. "a/b.mp3" and "a/b/c.mp3" where "a/b" would
    // match both instead of only files in the parent directory "a/b/".
    DEBUG_ASSERT(!oldFolder.endsWith('/'));
    const QString oldFolderPrefix = oldFolder + '/';
    const QString startsWithOldFolder = SqlLikeWildcardEscaper::apply(
                                                oldFolderPrefix, kSqlLikeMatchAll) +
            kSqlLikeMatchAll;

    query.prepare(QStringLiteral(
            "SELECT library.id,track_locations.id,track_locations.location "
            "FROM library INNER JOIN track_locations ON "
            "track_locations.id=library.location WHERE "
            "track_locations.location LIKE %1 ESCAPE '%2'")
                          .arg(SqlStringFormatter::format(
                                       m_database, startsWithOldFolder),
                                  kSqlLikeMatchAll));
    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "could not relocate path of tracks";
        return {};
    }

    QList<DbId> loc_ids;
    QList<RelocatedTrack> relocatedTracks;
    while (query.next()) {
        const auto oldLocation = query.value(2).toString();
        const int oldSuffixLen = oldLocation.size() - oldFolder.size();
        QString newLocation = newFolder + oldLocation.right(oldSuffixLen);
        // LIKE is case-insensitive! We cannot decide if the file system
        // at the old location was case-sensitive or case-insensitive
        // and must assume that the stored path is at least case-correct.
        DEBUG_ASSERT(oldLocation.startsWith(oldFolderPrefix, Qt::CaseInsensitive));
        if (!oldLocation.startsWith(oldFolderPrefix, Qt::CaseSensitive)) {
            qDebug() << "Skipping relocation of" << oldLocation
                     << "to" << newLocation;
            continue;
        }
        loc_ids.append(DbId(query.value(1).toInt()));
        const auto trackId = TrackId(query.value(0));
        auto missingTrackRef = TrackRef::fromFileInfo(
                TrackFile(oldLocation),
                std::move(trackId));
        auto addedTrackRef = TrackRef::fromFileInfo(
                TrackFile(newLocation)); // without TrackId, because no new track will be added!
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

QStringList DirectoryDAO::getDirs() const {
    QSqlQuery query(m_database);
    query.prepare("SELECT " % DIRECTORYDAO_DIR % " FROM " % DIRECTORYDAO_TABLE);
    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "could not retrieve directory list from database";
    }
    QStringList dirs;
    const int dirColumn = query.record().indexOf(DIRECTORYDAO_DIR);
    while (query.next()) {
        dirs << query.value(dirColumn).toString();
    }
    return dirs;
}
