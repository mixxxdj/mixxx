#include <QtSql>
#include <QtDebug>
#include <QStringBuilder>

#include "directorydao.h"
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

bool DirectoryDAO::addDirectory(QString dir){
    FieldEscaper escaper(m_database);
    QSqlQuery query(m_database);
    query.prepare("INSERT OR REPLACE INTO "% DIRECTORYDAO_TABLE %
                  " ("% DIRECTORYDAO_DIR %") VALUES :dir");
    query.bindValue(":dir",escaper.escapeString(dir));

    if (!query.exec()) {
        qDebug() << "Adding new dir ("% dir %") failed:"
                 <<query.lastError();
        LOG_FAILED_QUERY(query);
        return false;
    }
    return true;
}

bool DirectoryDAO::purgeDirectory(QString dir){
    FieldEscaper escaper(m_database);
    QSqlQuery query(m_database);
    query.prepare("DELETE FROM "% DIRECTORYDAO_TABLE  %" WHERE "
                   %DIRECTORYDAO_DIR%"=:dir");
    query.bindValue(":dir",escaper.escapeString(dir));
    if (!query.exec()) {
        qDebug() << "purging dir ("%dir%") failed:"<<query.lastError();
        return false;
    }
    return true;
}

bool DirectoryDAO::relocateDirectory(QString oldFolder, QString newFolder){
    ScopedTransaction transaction(m_database);
    FieldEscaper escaper(m_database);
    QSqlQuery query(m_database);
    // update directory in directories table
    query.prepare("UPDATE "%DIRECTORYDAO_TABLE%" SET "%DIRECTORYDAO_DIR%"="
                  ":newFolder WHERE "%DIRECTORYDAO_DIR%"=:oldFolder");
    query.bindValue(":newFolder", escaper.escapeString(newFolder));
    query.bindValue(":oldFolder", escaper.escapeString(oldFolder));
    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "coud not relocate directory";
        return false;
    }

    // update location and directory in track_locations table
    // TODO(kain88) do this in trackdao
    query.prepare("UPDATE track_locations SET location="
                  "REPLACE(location,:oldFolder,:newFolder)"
                  ", directory="
                  "REPLACE(directory,:oldFolder,:newFolder) "
                  "WHERE "%DIRECTORYDAO_DIR%"=:oldFolder");
    query.bindValue(":newFolder", escaper.escapeString(newFolder));
    query.bindValue(":oldFolder", escaper.escapeString(oldFolder));
    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "coud not relocate path of tracks";
        return false;
    }

    // updating the dir_id column is not necessary because it does not change
    transaction.commit();
    return true;
}

QStringList DirectoryDAO::getDirs(){
    QSqlQuery query(m_database);
    query.prepare("SELECT " % DIRECTORYDAO_DIR % " FROM " % DIRECTORYDAO_TABLE);
    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "There are no directories saved in the db";
    }
    QStringList dirs;
    while (query.next()) {
        QString dir = query.value(query.record().indexOf(DIRECTORYDAO_DIR)).toString();
        // remove all the ' that got added by FieldEscaper
        dirs << dir.replace("'","");
    }
    return dirs;
}

int DirectoryDAO::getDirId(const QString dir){
    QSqlQuery query(m_database);
    FieldEscaper escaper(m_database);
    query.prepare("SELECT " % DIRECTORYDAO_ID % " FROM " % DIRECTORYDAO_TABLE %
                  " WHERE " % DIRECTORYDAO_DIR %" = :dir");
    query.bindValue(":dir",escaper.escapeString(dir));
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
    }
    int id=-1;
    while (query.next()) {
        id = query.value(query.record().indexOf(DIRECTORYDAO_ID)).toInt();
    }
    return id;
}

bool DirectoryDAO::upgradeDatabase(QString dir){
    ScopedTransaction transaction(m_database);
    QSqlQuery query(m_database);
    // Default all Values to 0
    query.prepare("UPDATE track_locations SET maindir_id = 0");
    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << " could not update TrackLocations";
        return false;
    }

    // add dir to directory table and get the resulting ID
    addDirectory(dir);
    QString dirId = QString::number(getDirId(dir));

    // if the complete filename contains dir set maindir_id to dirId
    query.prepare("UPDATE track_locations SET maindir_id = :dirId"
                  "WHERE instr(track_locations.location, dir) > 0");
    query.bindValue(":dirId", dirId);
    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << " could not update TrackLocations";
        return false;
    }

    transaction.commit();
    return true;
}
