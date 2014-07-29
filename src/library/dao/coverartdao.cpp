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

int CoverArtDAO::saveCoverArt(QString coverLocation, QString md5Hash) {
    if (md5Hash.isEmpty()) {
        return -1;
    }

    int coverId = getCoverArtId(md5Hash);
    if (coverId == -1) { // new cover
        QSqlQuery query(m_database);

        query.prepare("INSERT INTO " % COVERART_TABLE % " ("
                      % COVERARTTABLE_LOCATION % "," % COVERARTTABLE_MD5 % ") "
                      "VALUES (:location, :md5)");
        query.bindValue(":location", coverLocation);
        query.bindValue(":md5", md5Hash);

        if (!query.exec()) {
            LOG_FAILED_QUERY(query) << "Saving new cover (" % coverLocation % ") failed.";
            return -1;
        }

        coverId = query.lastInsertId().toInt();
    }

    return coverId;
}

QList<int> CoverArtDAO::saveCoverArt(QList<QPair<QString, QString> > covers) {
    if (covers.isEmpty()) {
        return QList<int>();
    }

    // it'll be used to avoid writing a new ID for
    // rows which have the same md5 (not changed).
    QString selectCoverId = QString("SELECT id FROM cover_art WHERE md5='%1'");

    // preparing query to insert multi rows
    QString sQuery;
    sQuery = QString("INSERT OR REPLACE INTO cover_art ('id', 'location', 'md5') "
                     "SELECT (%1) AS 'id', '%2' AS 'location', '%3' AS 'md5' ")
                    .arg(selectCoverId.arg(covers.first().second))
                    .arg(covers.first().first)
                    .arg(covers.first().second);
    for (int i=1; i<covers.size(); i++) {
        sQuery = sQuery % QString("UNION SELECT (%1), '%2', '%3'")
                .arg(selectCoverId.arg(covers.at(i).second))
                .arg(covers.at(i).first)
                .arg(covers.at(i).second);
    }
    QSqlQuery query(m_database);
    if (!query.exec(sQuery)) {
        LOG_FAILED_QUERY(query) << "Failed to save multiple covers!";
        return QList<int>();
    }

    // getting the last inserted cover id's
    QList<int> coverIds;
    for (int index=0; index<covers.size(); index++) {
        int coverId = getCoverArtId(covers.at(index).second);
        coverIds.append(coverId);
    }

    return coverIds;
}

void CoverArtDAO::deleteUnusedCoverArts() {
    QSqlQuery query(m_database);

    query.prepare("SELECT " % COVERARTTABLE_ID %
                  " FROM " % COVERART_TABLE %
                  " WHERE " % COVERARTTABLE_ID % " NOT IN "
                      "(SELECT " % LIBRARYTABLE_COVERART_LOCATION %
                      " FROM " % COVERART_TABLE % " INNER JOIN " LIBRARY_TABLE
                      " ON " LIBRARY_TABLE "." % LIBRARYTABLE_COVERART_LOCATION %
                      " = " % COVERART_TABLE % "." % COVERARTTABLE_ID %
                      " GROUP BY " % LIBRARYTABLE_COVERART_LOCATION % ")");
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

int CoverArtDAO::getCoverArtId(QString md5Hash) {
    if (md5Hash.isEmpty()) {
        return -1;
    }

    QSqlQuery query(m_database);

    query.prepare(QString(
        "SELECT " % COVERARTTABLE_ID % " FROM " % COVERART_TABLE %
        " WHERE " % COVERARTTABLE_MD5 % "=:md5"));
    query.bindValue(":md5", md5Hash);
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
        "SELECT album, cover_art.location AS cover, cover_art.md5 AS md5, "
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
    const int md5Column = queryRecord.indexOf("md5");
    const int directoryColumn = queryRecord.indexOf("directory");
    const int filenameColumn = queryRecord.indexOf("filename");
    const int locationColumn = queryRecord.indexOf("location");

    if (query.next()) {
        CoverArtInfo coverInfo;
        coverInfo.trackId = trackId;
        coverInfo.album = query.value(albumColumn).toString();
        coverInfo.coverLocation = query.value(coverColumn).toString();
        coverInfo.md5Hash = query.value(md5Column).toString();
        coverInfo.trackDirectory = query.value(directoryColumn).toString();
        coverInfo.trackLocation = query.value(locationColumn).toString();
        coverInfo.trackBaseName = QFileInfo(query.value(filenameColumn)
                                            .toString()).baseName();
        return coverInfo;
    }

    return CoverArtInfo();
}
