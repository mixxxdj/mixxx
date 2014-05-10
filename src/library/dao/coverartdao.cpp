#include <QImage>
#include <QtDebug>
#include <QThread>

#include "library/dao/coverartdao.h"
#include "library/queryutil.h"

CoverArtDAO::CoverArtDAO(QSqlDatabase& database,
                         ConfigObject<ConfigValue>* pConfig)
        : m_database(database),
          m_pCoverArt(new CoverArt(pConfig)) {
}

CoverArtDAO::~CoverArtDAO() {
}

void CoverArtDAO::initialize() {
    qDebug() << "CoverArtDAO::initialize"
             << QThread::currentThread()
             << m_database.connectionName();
}

void CoverArtDAO::saveCoverArt(TrackInfoObject* pTrack) {
    // search cover art file (in disk) and get its location
    QString coverArtLocation = m_pCoverArt->searchCoverArtFile(pTrack);

    // cover art found
    if (!coverArtLocation.isEmpty()) {
        // search cover.location in database
        int coverId = getCoverArtID(coverArtLocation);

        QSqlQuery query(m_database);

        // if cover art isn't in database
        if (!coverId) { // insert new
            query.prepare(QString(
                "INSERT INTO cover_art (location) "
                "VALUES (:location)"));
            query.bindValue(":location", coverArtLocation);
            if (!query.exec()) {
                LOG_FAILED_QUERY(query) << "couldn't save new cover art";
                return;
            }
            coverId = query.lastInsertId().toInt();
        }

        // update library.cover_art with coverId
        query.prepare(QString(
            "UPDATE library "
            "SET cover_art = :coverId "
            "WHERE id = :trackId"));
        query.bindValue(":coverId", coverId);
        query.bindValue(":trackId", pTrack->getId());
        if (!query.exec()) {
            LOG_FAILED_QUERY(query) << "couldn't update existing cover art";
            return;
        }
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
