#include <QHashIterator>
#include <QtDebug>
#include <QThread>

#include "library/coverartcache.h"
#include "library/queryutil.h"
#include "library/dao/coverartdao.h"
#include "library/dao/trackdao.h"

CoverArtDAO::CoverArtDAO(QSqlDatabase& database)
        : m_database(database) {
}

CoverArtDAO::~CoverArtDAO() {
    qDebug() << "~CoverArtDAO()";
}

void CoverArtDAO::finish() {
    // As CoverArtCache needs to have an available CoverArtDAO/TrackDAO,
    // it must be destroyed BEFORE them.
    // During the CoverArtCache destruction, some covers and tracks
    // might be updated (queued).
    CoverArtCache::destroy();

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

QSet<QPair<int, int> > CoverArtDAO::saveCoverArt(
                            QHash<int, QPair<QString, QString> > covers) {
    if (covers.isEmpty()) {
        return QSet<QPair<int, int> >();
    }

    // it'll be used to avoid writing a new ID for
    // rows which have the same md5 (not changed).
    QString selectCoverId = QString("SELECT id FROM cover_art WHERE md5='%1'");

    // <trackId, coverId>
    QSet<QPair<int, int> > res;

    // preparing query to insert multi rows
    QString sQuery;
    QHashIterator<int, QPair<QString, QString> > i(covers);
    i.next();
    res.insert(qMakePair(i.key(), -1));
    sQuery = QString("INSERT OR REPLACE INTO cover_art ('id', 'location', 'md5') "
                     "SELECT (%1) AS 'id', '%2' AS 'location', '%3' AS 'md5' ")
                    .arg(selectCoverId.arg(i.value().second))
                    .arg(i.value().first)
                    .arg(i.value().second);

    while (i.hasNext()) {
        i.next();
        res.insert(qMakePair(i.key(), -1));
        sQuery = sQuery % QString("UNION SELECT (%1), '%2', '%3'")
                .arg(selectCoverId.arg(i.value().second))
                .arg(i.value().first)
                .arg(i.value().second);
    }

    QSqlQuery query(m_database);
    if (!query.exec(sQuery)) {
        LOG_FAILED_QUERY(query) << "Failed to save multiple covers!";
        return QSet<QPair<int, int> >();
    }

    QSetIterator<QPair<int, int> > set(res);
    while (set.hasNext()) {
        QPair<int, int> p = set.next();
        int trackId = p.first;
        int coverId = getCoverArtId(covers.value(trackId).second);
        if (coverId > 0) {
            res.remove(p);
            res.insert(qMakePair(trackId, coverId));
        }
    }

    return res;
}

void CoverArtDAO::deleteUnusedCoverArts() {    
    QString covers = "SELECT " % LIBRARYTABLE_COVERART_LOCATION %
                     " FROM " % COVERART_TABLE % " INNER JOIN " LIBRARY_TABLE
                     " ON " LIBRARY_TABLE "." % LIBRARYTABLE_COVERART_LOCATION %
                     " = " % COVERART_TABLE % "." % COVERARTTABLE_ID %
                     " GROUP BY " % LIBRARYTABLE_COVERART_LOCATION;

    QString sQuery = "DELETE FROM " % COVERART_TABLE %
                     " WHERE " % COVERARTTABLE_ID % " NOT IN (" % covers % ")";

    QSqlQuery query(m_database);
    if (!query.exec(sQuery)) {
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

    int coverId = -1;
    if (query.next()) {
        coverId = query.value(0).toInt();
    }

    return coverId;
}

// it'll get just the fields which are required for scanCover stuff
CoverArtDAO::CoverArtInfo CoverArtDAO::getCoverArtInfo(int trackId) {
    if (trackId < 1) {
        return CoverArtInfo();
    }

    // This method can be called a lot of times (by CoverCache),
    // if we use functions like "indexOf()" to find the column numbers
    // it will do at least one new loop for each column and it can bring
    // performance issues...
    QString columns = "album,"                         //0
                      "cover_art.location AS cover,"   //1
                      "cover_art.md5,"                 //2
                      "track_locations.directory,"     //3
                      "track_locations.filename,"      //4
                      "track_locations.location";      //5

    QString sQuery = QString(
         "SELECT " % columns % " FROM Library "
         "INNER JOIN track_locations ON library.location = track_locations.id "
         "LEFT JOIN cover_art ON cover_art.id = library.cover_art "
         "WHERE library.id = %1").arg(trackId);

    QSqlQuery query(m_database);
    if (!query.exec(sQuery)) {
      LOG_FAILED_QUERY(query);
      return CoverArtInfo();
    }

    if (query.next()) {
        CoverArtInfo coverInfo;
        coverInfo.trackId = trackId;
        coverInfo.album = query.value(0).toString();
        coverInfo.coverLocation = query.value(1).toString();
        coverInfo.md5Hash = query.value(2).toString();
        coverInfo.trackDirectory = query.value(3).toString();
        QString filename = query.value(4).toString();
        coverInfo.trackLocation = query.value(5).toString();
        coverInfo.trackBaseName = QFileInfo(filename).baseName();
        return coverInfo;
    }

    return CoverArtInfo();
}
