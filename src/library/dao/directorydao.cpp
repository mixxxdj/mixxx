#include <QtSql>
#include <QDir>
#include <QtDebug>
#include <QRegExp>
#include <QStringBuilder>

#include "library/dao/directorydao.h"
#include "library/queryutil.h"

DirectoryDAO::DirectoryDAO(QSqlDatabase& database)
            : m_database(database) {
}

DirectoryDAO::~DirectoryDAO() {
}

void DirectoryDAO::initialize() {
    qDebug() << "DirectoryDAO::initialize" << QThread::currentThread()
             << m_database.connectionName();
}

int DirectoryDAO::addDirectory(const QString& newDir) {
    // Do nothing if the dir to add is a child of a directory that is already in
    // the db.
    ScopedTransaction transaction(m_database);
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
    transaction.commit();
    return ALL_FINE;
}

bool DirectoryDAO::isChildDir(QString testDir, QString dirStr) {
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
    query.prepare("UPDATE " % DIRECTORYDAO_TABLE % " SET " % DIRECTORYDAO_DIR %
                  "=:newFolder WHERE " % DIRECTORYDAO_DIR % " = :oldFolder");
    query.bindValue(":newFolder", newFolder);
    query.bindValue(":oldFolder", oldFolder);
    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "could not relocate directory"
                                << oldFolder << "to" << newFolder;
        return QSet<int>();
    }

    FieldEscaper escaper(m_database);
    // on Windows the absolute path starts with the drive name
    // we also need to check for that
    QString startsWithOldFolder = escaper.escapeStringForLike(
        QDir(oldFolder).absolutePath() + "/", '%') + "%";

    // Also update information in the track_locations table. This is where mixxx
    // gets the location information for a track. Put marks around %1 so that
    // this also works on windows
    query.prepare(QString("SELECT library.id, track_locations.id, track_locations.location "
                          "FROM library INNER JOIN track_locations ON "
                          "track_locations.id = library.location WHERE "
                          "track_locations.location LIKE '%1' ESCAPE '%'")
                  .arg(startsWithOldFolder));
    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "could not relocate path of tracks";
        return QSet<int>();
    }

    QSet<int> ids;
    QList<int> loc_ids;
    QStringList old_locs;
    while (query.next()) {
        ids.insert(query.value(0).toInt());
        loc_ids.append(query.value(1).toInt());
        old_locs.append(query.value(2).toString());
    }

    QString replacement = "UPDATE track_locations SET location = :newloc "
            "WHERE id = :id";
    query.prepare(replacement);
    for (int i = 0; i < loc_ids.size(); ++i) {
        QString newloc = old_locs.at(i);
        newloc.replace(0, oldFolder.size(), newFolder);
        query.bindValue("newloc", newloc);
        query.bindValue("id", loc_ids.at(i));
        if (!query.exec()) {
            LOG_FAILED_QUERY(query) << "could not relocate path of tracks";
            return QSet<int>();
        }
    }

    qDebug() << "Relocated tracks:" << ids.size();
    transaction.commit();
    return ids;
}

QStringList DirectoryDAO::getDirs() {
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
