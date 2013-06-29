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

bool DirectoryDAO::relocateDirectory(QString oldFolder, QString newFolder) {
    ScopedTransaction transaction(m_database);
    QSqlQuery query(m_database);
    // update directory in directories table
    query.prepare("UPDATE " % DIRECTORYDAO_TABLE % " SET " % DIRECTORYDAO_DIR % "="
                  ":newFolder WHERE " % DIRECTORYDAO_DIR % "=:oldFolder");
    query.bindValue(":newFolder", newFolder);
    query.bindValue(":oldFolder", oldFolder);
    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "coud not relocate directory";
        return false;
    }

    // update location and directory in track_locations table
    query.prepare("UPDATE track_locations SET location="
                  "REPLACE(location, :oldFolder1,:newFolder1)"
                  ", directory="
                  "REPLACE(directory,:oldFolder2,:newFolder2) ");
    query.bindValue(":newFolder1", newFolder);
    query.bindValue(":oldFolder1", oldFolder);
    query.bindValue(":newFolder2", newFolder);
    query.bindValue(":oldFolder2", oldFolder);
    // qDebug() << escaper.escapeString(newFolder);
    // qDebug() << escaper.escapeString(oldFolder);
    qDebug() << query.boundValue(":newFolder1");
    qDebug() << query.boundValue(":oldFolder1");
    qDebug() << query.boundValue(":newFolder2");
    qDebug() << query.boundValue(":oldFolder2");
    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "coud not relocate path of tracks";
        return false;
    }
    qDebug() << query.executedQuery() ;
    transaction.commit();
    return true;
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
