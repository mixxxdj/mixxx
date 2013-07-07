#include <QtSql>
#include <QtDebug>
#include <QStringBuilder>

#include "library/dao/directorydao.h"
#include "library/queryutil.h"

DirectoryDAO::DirectoryDAO(QSqlDatabase& database)
            : m_database(database) {
}

DirectoryDAO::~DirectoryDAO(){
}

void DirectoryDAO::initialize() {
    qDebug() << "DirectoryDAO::initialize" << QThread::currentThread() 
             << m_database.connectionName();
}

bool DirectoryDAO::addDirectory(QString newDir) {
    // Do nothing if the dir to add is actualle a child of a directory that
    // is already in the db
    QStringList dirs = getDirs();
    foreach (QString dir, dirs) {
        if ( newDir.indexOf(dir) != -1 ) {
            return false;
        }
    }

    QSqlQuery query(m_database);
    query.prepare("INSERT INTO " % DIRECTORYDAO_TABLE %
                  " (" % DIRECTORYDAO_DIR % ") VALUES (:dir)");
    query.bindValue(":dir",newDir);
    if (!query.exec()) {
        qDebug() << "Adding new dir (" % newDir % ") failed:" << query.lastError();
        LOG_FAILED_QUERY(query);
        return false;
    }
    return true;
}

bool DirectoryDAO::purgeDirectory(QString dir) {
    QSqlQuery query(m_database);
    query.prepare("DELETE FROM " % DIRECTORYDAO_TABLE  % " WHERE "
                   % DIRECTORYDAO_DIR % "=:dir");
    query.bindValue(":dir", dir);
    if (!query.exec()) {
        qDebug() << "purging dir (" % dir % ") failed:"<<query.lastError();
        return false;
    }
    return true;
}

QSet<int> DirectoryDAO::relocateDirectory(QString oldFolder, QString newFolder) {
    ScopedTransaction transaction(m_database);
    QSqlQuery query(m_database);
    query.prepare("UPDATE " % DIRECTORYDAO_TABLE % " SET " % DIRECTORYDAO_DIR % "="
                  ":newFolder WHERE " % DIRECTORYDAO_DIR % "=:oldFolder");
    query.bindValue(":newFolder", newFolder);
    query.bindValue(":oldFolder", oldFolder);
    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "coud not relocate directory";
        return QSet<int>();
    }
    // Also update information in the track_locations table. This is were
    // mixxx gets the location information for a track
    query.prepare("UPDATE track_locations SET location="
                  "REPLACE(location, :oldLoc,:newLoc), "
                  "directory=REPLACE(directory,:oldDir,:newDir) "
                  "WHERE instr(location,:loc)>0");
    query.bindValue("oldLoc",oldFolder);
    query.bindValue("newLoc",newFolder);
    query.bindValue("oldDir",oldFolder);
    query.bindValue("newDir",newFolder);
    query.bindValue("loc",oldFolder);
    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "coud not relocate path of tracks";
        return QSet<int>();
    }

    query.prepare("SELECT library.id FROM library INNER JOIN track_locations ON "
                  "track_locations.id = library.location WHERE "
                  "instr(track_locations.location,:loc)>0");
    query.bindValue(":loc",newFolder);

    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "coud not relocate path of tracks";
        return QSet<int>();
    }

    QSet<int> ids;
    while (query.next()) {
        ids.insert(query.value(query.record().indexOf("library.id")).toInt());
    }

    transaction.commit();
    return ids;
}

QStringList DirectoryDAO::getDirs() {
    QSqlQuery query(m_database);
    query.prepare("SELECT " % DIRECTORYDAO_DIR % " FROM " % DIRECTORYDAO_TABLE);
    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "There are no directories saved in the db";
    }
    QStringList dirs;
    while (query.next()) {
        QString dir = query.value(query.record().indexOf(DIRECTORYDAO_DIR)).toString();
        dirs << dir;
    }
    return dirs;
}
