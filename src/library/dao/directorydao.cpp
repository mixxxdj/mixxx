#include <QtSql>
#include <QDir>
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

int DirectoryDAO::addDirectory(const QString& newDir) {
    // Do nothing if the dir to add is a child of a directory that is already in
    // the db.:
    QStringList dirs = getDirs();
    QString childDir("");
    QString parentDir("");
    foreach (const QString& dir, dirs) {
        if (isChildDir(newDir, dir)) {
            childDir = dir;
        }
        if (isChildDir(dir, newDir)) {
            parentDir = dir;
        }
    }

    if (!childDir.isEmpty()) {
        return ALREADY_WATCHING;
    }

    if (!parentDir.isEmpty()) {
        // removeing the old directory won't harm because we are adding the
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

bool DirectoryDAO::isChildDir(QString testDir, QString dir){
    // Qt internally always uses '/' as a seperator so this should also work
    // fine on windows -- kain88 (sep 2013)
    QStringList testDirNames = QDir(testDir).absolutePath().split('/');
    QStringList dirNames = QDir(dir).absolutePath().split('/');

    bool related = false;
    // testDir could be a child if the path a more items.
    if (testDirNames.size() > dirNames.size()){
        // To test if we have a child folder we go through all the folders and check
        // them for equality. 'related' is set to true at the beginning so we can use
        // the AND operation that will convert 'related' to false once one folder level
        // does not match anymore
        related = true;
        for (int i=0; i < dirNames.size(); ++i) {
            related = (testDirNames.at(i) == dirNames.at(i)) && related;
        }
    }

    // qDebug() << "--- test related function ---";
    // qDebug() << "testDir " << testDir;
    // qDebug() << "dir" << dir;
    // qDebug() << "related = " << related;
    // qDebug() << "-----------------------------";
    return related;
}

int DirectoryDAO::removeDirectory(const QString& dir) {
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


QSet<int> DirectoryDAO::relocateDirectory(const QString& oldFolder,
                                          const QString& newFolder) {
    // TODO(rryan): This method could use error reporting. It can fail in
    // mysterious ways for example if a track in the oldFolder also has a zombie
    // track location in newFolder then the replace query will fail because the
    // location column becomes non-unique.
    ScopedTransaction transaction(m_database);
    QSqlQuery query(m_database);
    query.prepare("UPDATE " % DIRECTORYDAO_TABLE % " SET " % DIRECTORYDAO_DIR % "="
                  ":newFolder WHERE " % DIRECTORYDAO_DIR % " = :oldFolder");
    query.bindValue(":newFolder", newFolder);
    query.bindValue(":oldFolder", oldFolder);
    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "coud not relocate directory"
                                << oldFolder << "to" << newFolder;
        return QSet<int>();
    }

    FieldEscaper escaper(m_database);
    QString startsWithOldFolder =
            escaper.escapeString(escaper.escapeStringForLike(oldFolder, '%') + '%');

    // Also update information in the track_locations table. This is where mixxx
    // gets the location information for a track.
    // TODO(rryan): This query replaces ALL occurrences of oldDir that are in
    // location, not just those starting with the string. We should do this
    // update outside of a query for safety.
    query.prepare(QString("UPDATE track_locations SET "
                          "location = REPLACE(location, :oldLoc, :newLoc), "
                          "directory = REPLACE(directory, :oldDir, :newDir) "
                          "WHERE location LIKE %1 ESCAPE '%'")
                  .arg(startsWithOldFolder));
    query.bindValue("oldLoc", oldFolder);
    query.bindValue("newLoc", newFolder);
    query.bindValue("oldDir", oldFolder);
    query.bindValue("newDir", newFolder);
    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "could not relocate path of tracks";
        return QSet<int>();
    }

    // TODO(rryan): This query selects tracks that were already in newFolder.
    QString startsWithNewFolder =
            escaper.escapeString(escaper.escapeStringForLike(newFolder, '%') + '%');
    query.prepare(QString("SELECT library.id FROM library INNER JOIN track_locations ON "
                          "track_locations.id = library.location WHERE "
                          "track_locations.location LIKE %1 ESCAPE '%'")
                  .arg(startsWithNewFolder));
    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "coud not relocate path of tracks";
        return QSet<int>();
    }

    QSet<int> ids;
    const int idColumn = query.record().indexOf("id");
    while (query.next()) {
        ids.insert(query.value(idColumn).toInt());
    }
    qDebug() << "Relocated tracks:" << ids.size();

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
    const int dirColumn = query.record().indexOf(DIRECTORYDAO_DIR);
    while (query.next()) {
        dirs << query.value(dirColumn).toString();
    }
    return dirs;
}
