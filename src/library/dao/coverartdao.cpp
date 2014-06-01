#include <QImage>
#include <QtDebug>
#include <QThread>

#include "library/coverartcache.h"
#include "library/queryutil.h"
#include "library/dao/coverartdao.h"

CoverArtDAO::CoverArtDAO(QSqlDatabase& database)
        : m_database(database),
          m_pCoverArt(CoverArt::instance()) {
}

CoverArtDAO::~CoverArtDAO() {
    deleteUnusedCoverArts();
}

void CoverArtDAO::initialize() {
    connect(CoverArtCache::getInstance(), SIGNAL(pixmapNotFound(TrackPointer)),
            this, SLOT(slotCoverArtScan(TrackPointer)), Qt::UniqueConnection);

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

        query.prepare(QString(
            "INSERT INTO cover_art (location) "
            "VALUES (:location)"));
        query.bindValue(":location", coverLocation);

        if (!query.exec()) {
            LOG_FAILED_QUERY(query) << "couldn't save new cover art";
            return 0;
        }

        coverId = query.lastInsertId().toInt();
    }

    return coverId;
}

void CoverArtDAO::slotCoverArtScan(TrackPointer pTrack) {
    QString coverLocation = pTrack->getCoverArtLocation();

    // handling cases when the file was externally removed from disk-cache
    bool removedFromDisk = false;
    if (!coverLocation.isEmpty()) {
        if (!QFile::exists(coverLocation)) {
            removedFromDisk = true;
        }
    }

    // looking for cover art in disk-cache directory.
    QString newCoverLocation = m_pCoverArt->getDefaultCoverLocation(pTrack);
    if(!QFile::exists(newCoverLocation)) {
        // looking for embedded covers and covers into track directory.
        QImage image = m_pCoverArt->searchCoverArt(pTrack);
        if (!m_pCoverArt->saveImage(image, newCoverLocation)) {
            newCoverLocation.clear();
        }
    }

    if (coverLocation != newCoverLocation) {
        saveCoverLocation(newCoverLocation);
        pTrack->setCoverArtLocation(newCoverLocation);
        if (!newCoverLocation.isEmpty()) {
            CoverArtCache::getInstance()->requestPixmap(pTrack);
        }
    } else if (removedFromDisk) {
        CoverArtCache::getInstance()->requestPixmap(pTrack);
    }
}

void CoverArtDAO::deleteUnusedCoverArts() {
    if (!m_database.isOpen()) {
        return;
    }

    QSqlQuery query(m_database);

    query.prepare("SELECT location FROM cover_art "
                  "WHERE id not in ("
                      "SELECT cover_art FROM cover_art INNER JOIN library "
                      "ON library.cover_art = cover_art.id "
                      "GROUP BY cover_art"
                  ")");

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
        if (m_pCoverArt->deleteFile(coverLocation)) {
            coverLocationList << escaper.escapeString(coverLocation);
        }
    }

    if (coverLocationList.empty()) {
        return;
    }

    query.prepare(QString("DELETE FROM cover_art WHERE location in (%1)")
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
        "SELECT id FROM cover_art "
        "WHERE location = :location"));
    query.bindValue(":location", coverLocation);

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
