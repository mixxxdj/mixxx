#include <QtSql>
#include <QString>
#include <QtDebug>
#include <QVariant>
#include <QThread>

#include "libraryhashdao.h"
#include "library/queryutil.h"

namespace {

// Store hash values as a signed 64-bit integer. Otherwise values greater
// than 2^63-1 would be converted into a floating point numbers while
// losing precision!!
inline constexpr mixxx::cache_key_signed_t dbHash(mixxx::cache_key_t hash) {
    return mixxx::signedCacheKey(hash);
}

} // anonymous namespace

QHash<QString, mixxx::cache_key_t> LibraryHashDAO::getDirectoryHashes() {
    QSqlQuery query(m_database);
    query.prepare("SELECT hash, directory_path FROM LibraryHashes");
    QHash<QString, mixxx::cache_key_t> hashes;
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
    }

    int hashColumn = query.record().indexOf("hash");
    int directoryPathColumn = query.record().indexOf("directory_path");
    while (query.next()) {
        hashes[query.value(directoryPathColumn).toString()] =
                query.value(hashColumn).toULongLong();
    }

    return hashes;
}

mixxx::cache_key_t LibraryHashDAO::getDirectoryHash(const QString& dirPath) {
    //qDebug() << "LibraryHashDAO::getDirectoryHash" << QThread::currentThread() << m_database.connectionName();
    mixxx::cache_key_t hash = mixxx::invalidCacheKey();

    QSqlQuery query(m_database);
    query.prepare("SELECT hash FROM LibraryHashes "
                  "WHERE directory_path=:directory_path");
    query.bindValue(":directory_path", dirPath);

    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "SELECT hash failed:";
    }
    //Grab a hash for this directory from the database, from the last time it was scanned.
    if (query.next()) {
        hash = query.value(query.record().indexOf("hash")).toULongLong();
        //qDebug() << "prev hash exists" << hash << dirPath;
    } else {
        //qDebug() << "prev hash does not exist" << dirPath;
    }

    return hash;
}

void LibraryHashDAO::saveDirectoryHash(const QString& dirPath, mixxx::cache_key_t hash) {
    //qDebug() << "LibraryHashDAO::saveDirectoryHash" << QThread::currentThread() << m_database.connectionName();
    QSqlQuery query(m_database);
    query.prepare("INSERT INTO LibraryHashes (directory_path, hash, directory_deleted) "
                    "VALUES (:directory_path, :hash, :directory_deleted)");
    query.bindValue(":directory_path", dirPath);
    query.bindValue(":hash", dbHash(hash));
    query.bindValue(":directory_deleted", 0);

    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "Creating new dirhash failed.";
    }
    //qDebug() << "created new hash" << hash;
}

void LibraryHashDAO::updateDirectoryHash(const QString& dirPath,
                                         mixxx::cache_key_t newHash,
                                         int dir_deleted) {
    //qDebug() << "LibraryHashDAO::updateDirectoryHash" << QThread::currentThread() << m_database.connectionName();
    QSqlQuery query(m_database);
    // By definition if we have calculated a new hash for a directory then it
    // exists and no longer needs verification.
    query.prepare("UPDATE LibraryHashes "
            "SET hash=:hash, directory_deleted=:directory_deleted, "
            "needs_verification=0 "
            "WHERE directory_path=:directory_path");
    query.bindValue(":hash", dbHash(newHash));
    query.bindValue(":directory_deleted", dir_deleted);
    query.bindValue(":directory_path", dirPath);

    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "Updating existing dirhash failed.";
    }
    //qDebug() << "updated old existing hash" << newHash << dirPath << dir_deleted;

    //DEBUG: Print out the directory hash we just saved to verify...
    //qDebug() << getDirectoryHash(dirPath);
}

void LibraryHashDAO::updateDirectoryStatuses(const QStringList& dirPaths,
                                             const bool deleted,
                                             const bool verified) {
    //qDebug() << "LibraryHashDAO::updateDirectoryStatus" << QThread::currentThread() << m_database.connectionName();
    FieldEscaper escaper(m_database);
    QStringList escapedDirPaths = escaper.escapeStrings(dirPaths);

    QSqlQuery query(m_database);
    query.prepare(
        QString("UPDATE LibraryHashes "
                "SET directory_deleted=:directory_deleted, "
                "needs_verification=:needs_verification "
                "WHERE directory_path IN (%1)")
        .arg(escapedDirPaths.join(",")));
    query.bindValue(":directory_deleted", deleted ? 1 : 0);
    query.bindValue(":needs_verification", !verified ? 1 : 0);
    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "Updating directory status failed.";
    }
}

void LibraryHashDAO::markAsExisting(const QString& dirPath) {
    //qDebug() << "LibraryHashDAO::markExisting" << QThread::currentThread() << m_database.connectionName();
    QSqlQuery query(m_database);
    query.prepare("UPDATE LibraryHashes "
                  "SET directory_deleted=:directory_deleted "
                  "WHERE directory_path=:directory_path");
    query.bindValue(":directory_deleted", 0);
    query.bindValue(":directory_path", dirPath);
    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "Updating dirhash to mark as existing failed.";
    }
}

void LibraryHashDAO::invalidateAllDirectories() {
    //qDebug() << "LibraryHashDAO::invalidateAllDirectories"
    //<< QThread::currentThread() << m_database.connectionName();
    QSqlQuery query(m_database);
    query.prepare("UPDATE LibraryHashes "
                  "SET needs_verification=1");
    if (!query.exec()) {
        LOG_FAILED_QUERY(query)
                << "Couldn't mark directories previously hashed as needing verification.";
    }
}

void LibraryHashDAO::markUnverifiedDirectoriesAsDeleted() {
    //qDebug() << "LibraryHashDAO::markUnverifiedDirectoriesAsDeleted"
    //<< QThread::currentThread() << m_database.connectionName();
    QSqlQuery query(m_database);
    query.prepare("UPDATE LibraryHashes "
                  "SET directory_deleted=:directory_deleted "
                  "WHERE needs_verification=1");
    query.bindValue(":directory_deleted", 1);
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
    }
}

void LibraryHashDAO::removeDeletedDirectoryHashes() {
    QSqlQuery query(m_database);
    query.prepare("DELETE FROM LibraryHashes WHERE "
               "directory_deleted=:directory_deleted");
    query.bindValue(":directory_deleted", 1);
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
    }
}

QStringList LibraryHashDAO::getDeletedDirectories() {
    QStringList result;
    QSqlQuery query(m_database);
    query.prepare("SELECT directory_path FROM LibraryHashes "
                  "WHERE directory_deleted=1");
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
    }
    const int directoryPathColumn = query.record().indexOf("directory_path");
    while (query.next()) {
        QString directory = query.value(directoryPathColumn).toString();
        result << directory;
    }
    return result;
}
