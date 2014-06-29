#include <QtDebug>
#include <QThread>

#include "library/dao/coverartdao.h"
#include "library/dao/trackdao.h"
#include "library/queryutil.h"

CoverArtDAO::CoverArtDAO(QSqlDatabase& database)
        : m_database(database) {
}

CoverArtDAO::~CoverArtDAO() {
    deleteUnusedCoverArts();
}

void CoverArtDAO::initialize() {
    qDebug() << "CoverArtDAO::initialize"
             << QThread::currentThread()
             << m_database.connectionName();
}

int CoverArtDAO::saveCoverLocation(QString coverLocation) {
    if (coverLocation.isEmpty()) {
        return -1;
    }

    int coverId = getCoverArtId(coverLocation);
    if (coverId == -1) { // new cover
        QSqlQuery query(m_database);

        query.prepare("INSERT INTO " % COVERART_TABLE %
                      " (" % COVERARTTABLE_LOCATION % ") VALUES (:location)");
        query.bindValue(":location", coverLocation);

        if (!query.exec()) {
            LOG_FAILED_QUERY(query) << "Saving new cover (" % coverLocation % ") failed.";
            return -1;
        }

        coverId = query.lastInsertId().toInt();
    }

    return coverId;
}

void CoverArtDAO::deleteUnusedCoverArts() {
    QSqlQuery query(m_database);

    query.prepare("SELECT " % COVERARTTABLE_ID %
                  " FROM " % COVERART_TABLE %
                  " WHERE " % COVERARTTABLE_ID % " NOT IN "
                      "(SELECT " % LIBRARYTABLE_COVERART %
                      " FROM " % COVERART_TABLE % " INNER JOIN " LIBRARY_TABLE
                      " ON " LIBRARY_TABLE "." % LIBRARYTABLE_COVERART %
                      " = " % COVERART_TABLE % "." % COVERARTTABLE_ID %
                      " GROUP BY " % LIBRARYTABLE_COVERART % ")");
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return;
    }

    QStringList coverIdList;
    QSqlRecord queryRecord = query.record();
    const int coverIdColumn = queryRecord.indexOf("id");
    while (query.next()) {
        coverIdList << query.value(coverIdColumn).toString();
    }

    if (coverIdList.empty()) {
        return;
    }

    query.prepare(QString("DELETE FROM " % COVERART_TABLE %
                          " WHERE " % COVERARTTABLE_ID % " IN (%1)")
                  .arg(coverIdList.join(",")));
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
    }
}

int CoverArtDAO::getCoverArtId(QString coverLocation) {
    if (coverLocation.isEmpty()) {
        return -1;
    }

    QSqlQuery query(m_database);

    query.prepare(QString(
        "SELECT " % COVERARTTABLE_ID % " FROM " % COVERART_TABLE %
        " WHERE " % COVERARTTABLE_LOCATION % "=:location"));
    query.bindValue(":location", coverLocation);
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return -1;
    }

    if (query.next()) {
        return query.value(0).toInt();
    }

    return -1;
}

// it'll get just the fields which are required for scanCover stuff
CoverArtDAO::CoverArtInfo CoverArtDAO::getCoverArtInfo(int trackId) {
    if (trackId < 1) {
        return CoverArtInfo();
    }

    QSqlQuery query(m_database);
    query.prepare(
        "SELECT album, cover_art.location AS cover, "
        "track_locations.directory AS directory, "
        "track_locations.filename AS filename, "
        "track_locations.location AS location "
        "FROM Library INNER JOIN track_locations "
        "ON library.location = track_locations.id "
        "LEFT JOIN cover_art ON cover_art.id = library.cover_art "
        "WHERE library.id=:id "
    );
    query.bindValue(":id", trackId);

    if (!query.exec()) {
      LOG_FAILED_QUERY(query);
      return CoverArtInfo();
    }

    QSqlRecord queryRecord = query.record();
    const int albumColumn = queryRecord.indexOf("album");
    const int coverColumn = queryRecord.indexOf("cover");
    const int directoryColumn = queryRecord.indexOf("directory");
    const int filenameColumn = queryRecord.indexOf("filename");
    const int locationColumn = queryRecord.indexOf("location");

    if (query.next()) {
        CoverArtInfo coverInfo;
        coverInfo.trackId = trackId;
        coverInfo.album = query.value(albumColumn).toString();
        coverInfo.coverLocation = query.value(coverColumn).toString();
        coverInfo.trackDirectory = query.value(directoryColumn).toString();
        coverInfo.trackLocation = query.value(locationColumn).toString();
        coverInfo.trackBaseName = QFileInfo(query.value(filenameColumn)
                                            .toString()).baseName();
        return coverInfo;
    }

    return CoverArtInfo();
}
