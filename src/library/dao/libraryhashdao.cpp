
#include <QtSql>
#include <QString>
#include <QtDebug>
#include <QVariant>
#include <QThread>

#include "libraryhashdao.h"
#include "library/queryutil.h"

LibraryHashDAO::LibraryHashDAO(QSqlDatabase& database)
        : m_database(database) {

}

LibraryHashDAO::~LibraryHashDAO()
{
}

void LibraryHashDAO::initialize() {
    qDebug() << "LibraryHashDAO::initialize" << QThread::currentThread()
             << m_database.connectionName();
}

QHash<QString, int> LibraryHashDAO::getDirectoryHashes() {
    QSqlQuery query(m_database);
    query.prepare("SELECT hash, directory_path FROM LibraryHashes");
    QHash<QString, int> hashes;
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
    }

    int hashColumn = query.record().indexOf("hash");
    int directoryPathColumn = query.record().indexOf("directory_path");
    while (query.next()) {
        hashes[query.value(directoryPathColumn).toString()] =
                query.value(hashColumn).toInt();
    }

    return hashes;
}

int LibraryHashDAO::getDirectoryHash(const QString& dirPath) {
    //qDebug() << "LibraryHashDAO::getDirectoryHash" << QThread::currentThread() << m_database.connectionName();
    int hash = -1;

    QSqlQuery query(m_database);
    query.prepare("SELECT hash FROM LibraryHashes "
                  "WHERE directory_path=:directory_path");
    query.bindValue(":directory_path", dirPath);

    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "SELECT hash failed:";
    }
    //Grab a hash for this directory from the database, from the last time it was scanned.
    if (query.next()) {
        hash = query.value(query.record().indexOf("hash")).toInt();
        //qDebug() << "prev hash exists" << hash << dirPath;
    } else {
        //qDebug() << "prev hash does not exist" << dirPath;
    }

    return hash;
}

void LibraryHashDAO::saveDirectoryHash(const QString& dirPath, const int hash) {
    //qDebug() << "LibraryHashDAO::saveDirectoryHash" << QThread::currentThread() << m_database.connectionName();
    QSqlQuery query(m_database);
    query.prepare("INSERT INTO LibraryHashes (directory_path, hash, directory_deleted) "
                    "VALUES (:directory_path, :hash, :directory_deleted)");
    query.bindValue(":directory_path", dirPath);
    query.bindValue(":hash", hash);
    query.bindValue(":directory_deleted", 0);

    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "Creating new dirhash failed.";
    }
    //qDebug() << "created new hash" << hash;
}

void LibraryHashDAO::updateDirectoryHash(const QString& dirPath,
                                         const int newHash,
                                         const int dir_deleted) {
    //qDebug() << "LibraryHashDAO::updateDirectoryHash" << QThread::currentThread() << m_database.connectionName();
    QSqlQuery query(m_database);
    // By definition if we have calculated a new hash for a directory then it
    // exists and no longer needs verification.
    query.prepare("UPDATE LibraryHashes "
            "SET hash=:hash, directory_deleted=:directory_deleted, "
            "needs_verification=0 "
            "WHERE directory_path=:directory_path");
    query.bindValue(":hash", newHash);
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
