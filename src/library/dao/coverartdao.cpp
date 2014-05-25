#include <QImage>
#include <QtDebug>
#include <QThread>

#include "library/dao/coverartdao.h"
#include "library/queryutil.h"

CoverArtDAO::CoverArtDAO(QSqlDatabase& database)
        : m_database(database),
          m_pCoverArt(CoverArt::instance()) {
}

CoverArtDAO::~CoverArtDAO() {
}

void CoverArtDAO::initialize() {
    qDebug() << "CoverArtDAO::initialize"
             << QThread::currentThread()
             << m_database.connectionName();
}

int CoverArtDAO::saveCoverLocation(QString location) {
    if (location.isEmpty()) {
        return 0;
    }

    int coverId = getCoverArtID(location);
    // new cover
    if (!coverId) {
        QSqlQuery query(m_database);

        query.prepare(QString(
            "INSERT INTO cover_art (location) "
            "VALUES (:location)"));
        query.bindValue(":location", location);

        if (!query.exec()) {
            LOG_FAILED_QUERY(query) << "couldn't save new cover art";
            return 0;
        }

        coverId = query.lastInsertId().toInt();
    }

    return coverId;
}

void CoverArtDAO::coverArtScan(TrackInfoObject* pTrack) {
    // search cover art file IN DISK
    QString coverArtLocation = m_pCoverArt->searchCoverArtFile(pTrack);

    if (pTrack->getCoverArtLocation() != coverArtLocation) {
        saveCoverLocation(coverArtLocation);
        pTrack->setCoverArtLocation(coverArtLocation);
    }
}

bool CoverArtDAO::deleteCoverArt(const int coverId) {
    QSqlQuery query(m_database);

    query.prepare("DELETE FROM cover_art WHERE id = :id");
    query.bindValue(":id", coverId);

    if (query.exec()) {
        return true;
    } else {
        LOG_FAILED_QUERY(query);
    }

    return false;
}

bool CoverArtDAO::deleteUnusedCoverArts() {
    QSqlQuery query(m_database);

    query.prepare("SELECT id, location FROM cover_art "
                  "WHERE id not in ("
                      "SELECT cover_art FROM cover_art INNER JOIN library "
                      "ON library.cover_art = cover_art.id "
                      "GROUP BY cover_art"
                  ")");

    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return false;
    }

    while (query.next()) {
        if (m_pCoverArt->deleteFile(query.value(1).toString())) {
            deleteCoverArt(query.value(0).toInt());
        }
    }

    return true;
}

int CoverArtDAO::getCoverArtID(QString location) {
    QSqlQuery query(m_database);

    query.prepare(QString(
        "SELECT id FROM cover_art "
        "WHERE location = :location"));
    query.bindValue(":location", location);

    if (query.exec()) {
        if (query.next()) {
            return query.value(0).toInt();
        }
    } else {
        LOG_FAILED_QUERY(query);
    }

    return 0;
}

QString CoverArtDAO::getCoverArtLocation(int id) {
    QSqlQuery query(m_database);

    query.prepare(QString(
        "SELECT location FROM cover_art "
        "WHERE id = :id"));
    query.bindValue(":id", id);

    if (query.exec()) {
        if (query.next()) {
            return query.value(0).toString();
        }
    } else {
        LOG_FAILED_QUERY(query);
    }

    return "";
}
