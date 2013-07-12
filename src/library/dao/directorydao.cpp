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

bool DirectoryDAO::addDirectory(const QString& newDir) {
    // Do nothing if the dir to add is a child of a directory that is already in
    // the db.
    QStringList dirs = getDirs();
    foreach (const QString& dir, dirs) {
        // TODO(rryan): dir needs to end in a slash otherwise we will mis-match.
        if (newDir.startsWith(dir)) {
            return false;
        }
    }

    QSqlQuery query(m_database);
    query.prepare("INSERT INTO " % DIRECTORYDAO_TABLE %
                  " (" % DIRECTORYDAO_DIR % ") VALUES (:dir)");
    query.bindValue(":dir", newDir);
    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "Adding new dir (" % newDir % ") failed.";
        return false;
    }
    return true;
}

bool DirectoryDAO::removeDirectory(const QString& dir) {
    QSqlQuery query(m_database);
    query.prepare("DELETE FROM " % DIRECTORYDAO_TABLE  % " WHERE "
                   % DIRECTORYDAO_DIR % "= :dir");
    query.bindValue(":dir", dir);
    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "purging dir (" % dir % ") failed";
        return false;
    }
    return true;
}


QSet<int> DirectoryDAO::relocateDirectory(const QString& oldFolder,
                                          const QString& newFolder) {
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
