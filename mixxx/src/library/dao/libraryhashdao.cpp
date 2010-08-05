#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QString>
#include <QtDebug>
#include <QVariant>
#include <QThread>

#include "libraryhashdao.h"

LibraryHashDAO::LibraryHashDAO(QSqlDatabase& database)
        : m_database(database) {

}

LibraryHashDAO::~LibraryHashDAO()
{
}

void LibraryHashDAO::initialize()
{
    qDebug() << "LibraryHashDAO::initialize" << QThread::currentThread() << m_database.connectionName();
}

int LibraryHashDAO::getDirectoryHash(QString dirPath)
{
    //qDebug() << "LibraryHashDAO::getDirectoryHash" << QThread::currentThread() << m_database.connectionName();
    int hash = -1;

    QSqlQuery query(m_database);
    query.prepare("SELECT hash FROM LibraryHashes "
                  "WHERE directory_path=:directory_path");
    query.bindValue(":directory_path", dirPath);

    if (!query.exec()) {
        qDebug() << "SELECT hash failed:" << query.lastError();
    }
    //Grab a hash for this directory from the database, from the last time it was scanned.
    if (query.next()) {
        hash = query.value(query.record().indexOf("hash")).toInt();
        qDebug() << "prev hash exists" << hash << dirPath;
    }
    else {
        qDebug() << "prev hash does not exist" << dirPath;
    }

    return hash;
}

void LibraryHashDAO::saveDirectoryHash(QString dirPath, int hash)
{
    //qDebug() << "LibraryHashDAO::saveDirectoryHash" << QThread::currentThread() << m_database.connectionName();
    QSqlQuery query(m_database);
    query.prepare("INSERT INTO LibraryHashes (directory_path, hash, directory_deleted) "
                    "VALUES (:directory_path, :hash, :directory_deleted)");
    query.bindValue(":directory_path", dirPath);
    query.bindValue(":hash", hash);
    query.bindValue(":directory_deleted", 0);


    if (!query.exec()) {
        qDebug() << "Creating new dirhash failed:" << query.lastError();
    }
    //qDebug() << "created new hash" << hash;
}

void LibraryHashDAO::updateDirectoryHash(QString dirPath, int newHash, int dir_deleted)
{
    //qDebug() << "LibraryHashDAO::updateDirectoryHash" << QThread::currentThread() << m_database.connectionName();
    QSqlQuery query(m_database);
    query.prepare("UPDATE LibraryHashes "
            "SET hash=:hash, directory_deleted=:directory_deleted "
            "WHERE directory_path=:directory_path");
    query.bindValue(":hash", newHash);
    query.bindValue(":directory_deleted", dir_deleted);
    query.bindValue(":directory_path", dirPath);

    if (!query.exec()) {
        qDebug() << "Updating existing dirhash failed:" << query.lastError();
    }
    //qDebug() << "updated old existing hash" << newHash << dirPath << dir_deleted;

    //DEBUG: Print out the directory hash we just saved to verify...
    //qDebug() << getDirectoryHash(dirPath);
}

void LibraryHashDAO::markAsExisting(QString dirPath)
{
    //qDebug() << "LibraryHashDAO::markExisting" << QThread::currentThread() << m_database.connectionName();
    QSqlQuery query(m_database);
    query.prepare("UPDATE LibraryHashes "
                  "SET directory_deleted=:directory_deleted "
                  "WHERE directory_path=:directory_path");
    query.bindValue(":directory_deleted", 0);
    query.bindValue(":directory_path", dirPath);
    if (!query.exec()) {
        qDebug() << "Updating dirhash to mark as existing failed:" << query.lastError();
    }
}

void LibraryHashDAO::markAllDirectoriesAsDeleted()
{
    //qDebug() << "LibraryHashDAO::markAllDirectoriesAsDeleted"
    //<< QThread::currentThread() << m_database.connectionName();
    QSqlQuery query(m_database);
    query.prepare("UPDATE LibraryHashes "
                  "SET directory_deleted=:directory_deleted");
    query.bindValue(":directory_deleted", 1);
    if (!query.exec()) {
        qDebug() << query.lastError();
    }
}

void LibraryHashDAO::removeDeletedDirectoryHashes()
{
    QSqlQuery query(m_database);
    query.prepare("DELETE FROM LibraryHashes WHERE "
               "directory_deleted=:directory_deleted");
    query.bindValue(":directory_deleted", 1);
    Q_ASSERT(query.exec());
}