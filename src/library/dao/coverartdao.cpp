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
        return 0;
    }

    int coverId = getCoverArtId(coverLocation);
    if (!coverId) { // new cover
        QSqlQuery query(m_database);

        query.prepare("INSERT INTO " % COVERART_TABLE %
                      " (" % COVERARTTABLE_LOCATION % ") VALUES (:location)");
        query.bindValue(":location", coverLocation);

        if (!query.exec()) {
            LOG_FAILED_QUERY(query) << "Saving new cover (" % coverLocation % ") failed.";
            return 0;
        }

        coverId = query.lastInsertId().toInt();
    }

    return coverId;
}

void CoverArtDAO::deleteUnusedCoverArts() {
    if (m_database.isValid()) { // returns true if an error is set
        return;
    }

    QSqlQuery query(m_database);

    query.prepare("SELECT " % COVERARTTABLE_LOCATION %
                  " FROM " % COVERART_TABLE %
                  " WHERE " % COVERARTTABLE_ID % " not in "
                      "(SELECT " % LIBRARYTABLE_COVERART %
                      " FROM " % COVERART_TABLE % " INNER JOIN " LIBRARY_TABLE
                      " ON " LIBRARY_TABLE "." % LIBRARYTABLE_COVERART %
                      " = " % COVERART_TABLE % "." % COVERARTTABLE_ID %
                      " GROUP BY " % LIBRARYTABLE_COVERART % ")");
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return;
    }

    FieldEscaper escaper(m_database);
    QStringList coverLocationList;
    QSqlRecord queryRecord = query.record();
    const int locationColumn = queryRecord.indexOf("location");
    while (query.next()) {
        QString coverLocation = query.value(locationColumn).toString();
        coverLocationList << escaper.escapeString(coverLocation);
    }

    if (coverLocationList.empty()) {
        return;
    }

    query.prepare(QString("DELETE FROM " % COVERART_TABLE %
                          " WHERE " % COVERARTTABLE_LOCATION % " in (%1)")
                  .arg(coverLocationList.join(",")));
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
    }
}

int CoverArtDAO::getCoverArtId(QString coverLocation) {
    if (coverLocation.isEmpty()) {
        return 0;
    }

    QSqlQuery query(m_database);

    query.prepare(QString(
        "SELECT " % COVERARTTABLE_ID % " FROM " % COVERART_TABLE %
        " WHERE " % COVERARTTABLE_LOCATION % "=:location"));
    query.bindValue(":location", coverLocation);
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return 0;
    }

    if (query.next()) {
        return query.value(0).toInt();
    }

    return 0;
}

QString CoverArtDAO::getCoverArtLocation(int id, bool fromTrackId) {
    if (id < 1) {
        return QString();
    }

    QSqlQuery query(m_database);

    if (fromTrackId) {
        query.prepare(QString("SELECT cover_art.location FROM cover_art "
                              "INNER JOIN library "
                              "ON library.cover_art=cover_art.id "
                              "WHERE library.id=:id "));
    } else {
        query.prepare(QString("SELECT location FROM cover_art "
                              "WHERE id=:id"));
    }

    query.bindValue(":id", id);

    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return QString();
    }

    if (query.next()) {
        return query.value(0).toString();
    }

    return QString();
}

// it'll get just the fields which are required for scanCover stuff
CoverArtDAO::CoverArtInfo CoverArtDAO::getCoverArtInfo(int trackId) {
    if (trackId < 1) {
        return CoverArtInfo();
    }

    QSqlQuery query(m_database);
    query.prepare(
        "SELECT album, cover_art.location AS cover, "
        "track_locations.directory as directory, "
        "track_locations.filename as filename, "
        "track_locations.location as location "
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
        coverInfo.trackFilename = query.value(filenameColumn).toString();
        coverInfo.trackDirectory = query.value(directoryColumn).toString();
        coverInfo.trackLocation = query.value(locationColumn).toString();
        return coverInfo;
    }

    return CoverArtInfo();
}
