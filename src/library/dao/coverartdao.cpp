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
    connect(CoverArtCache::instance(), SIGNAL(pixmapNotFound(int)),
            this, SLOT(slotCoverArtScan(int)), Qt::UniqueConnection);

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

void CoverArtDAO::slotCoverArtScan(int trackId) {
    TrackPointer pTrack = getTrackFromDB(trackId);

    if (pTrack.isNull()) {
        return;
    }

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
        int coverId = saveCoverLocation(newCoverLocation);
        updateLibrary(trackId, coverId);
        if (!newCoverLocation.isEmpty()) {
            CoverArtCache::instance()->requestPixmap(newCoverLocation, trackId);
        }
    } else if (removedFromDisk) {
        CoverArtCache::instance()->requestPixmap(newCoverLocation, trackId);
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

    if (query.exec()) {
        if (query.next()) {
            return query.value(0).toString();
        }
    } else {
        LOG_FAILED_QUERY(query);
    }

    return QString();
}

// it'll get just the fields which are required for scanCover stuff
TrackPointer CoverArtDAO::getTrackFromDB(int trackId) {
    if (trackId < 1) {
        return TrackPointer();
    }

    QSqlQuery query(m_database);
    query.prepare(
        "SELECT artist, album, track_locations.location as location, "
        "cover_art.location AS cover "
        "FROM Library INNER JOIN track_locations "
        "ON library.location = track_locations.id "
        "LEFT JOIN cover_art ON cover_art.id = library.cover_art "
        "WHERE library.id=:id "
    );
    query.bindValue(":id", trackId);

    if (query.exec()) {
        QSqlRecord queryRecord = query.record();
        const int artistColumn = queryRecord.indexOf("artist");
        const int albumColumn = queryRecord.indexOf("album");
        const int locationColumn = queryRecord.indexOf("location");
        const int coverColumn = queryRecord.indexOf("cover");

        if (query.next()) {
            QString artist = query.value(artistColumn).toString();
            QString album = query.value(albumColumn).toString();
            QString location = query.value(locationColumn).toString();
            QString cover = query.value(coverColumn).toString();

            TrackPointer pTrack = TrackPointer(new TrackInfoObject(
                                                   location,
                                                   SecurityTokenPointer(),
                                                   false));
            pTrack->setArtist(artist);
            pTrack->setAlbum(album);
            pTrack->setCoverArtLocation(cover);

            return pTrack;
        }
    } else {
        LOG_FAILED_QUERY(query);
    }

    return TrackPointer();
}

QString CoverArtDAO::getDefaultCoverLocation(int trackId) {
    if (trackId < 1) {
        return QString();
    }

    QSqlQuery query(m_database);
    query.prepare(
        "SELECT artist, album, filename "
        "FROM Library INNER JOIN track_locations "
        "ON library.location = track_locations.id "
        "WHERE library.id=:id "
    );
    query.bindValue(":id", trackId);

    if (query.exec()) {
        QSqlRecord queryRecord = query.record();
        const int artistColumn = queryRecord.indexOf("artist");
        const int albumColumn = queryRecord.indexOf("album");
        const int filenameColumn = queryRecord.indexOf("filename");

        if (query.next()) {
            QString artist = query.value(artistColumn).toString();
            QString album = query.value(albumColumn).toString();
            QString filename = query.value(filenameColumn).toString();

            QString coverName = m_pCoverArt->getDefaultCoverName(artist, album, filename);
            return m_pCoverArt->getDefaultCoverLocation(coverName);
        }
    } else {
        LOG_FAILED_QUERY(query);
    }

    return QString();
}

bool CoverArtDAO::updateLibrary(int trackId, int coverId) {
    if (trackId < 1 || coverId < 1) {
        return false;
    }
    QSqlQuery query(m_database);
    query.prepare("UPDATE library SET cover_art=:coverId WHERE id=:trackId");
    query.bindValue(":coverId", coverId);
    query.bindValue(":trackId", trackId);

    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "couldn't update library.cover_art";
        return false;
    }
    return true;
}
