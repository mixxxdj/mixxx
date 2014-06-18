#include <QtDebug>
#include <QThread>

#include "library/queryutil.h"
#include "library/dao/coverartdao.h"

CoverArtDAO::CoverArtDAO(QSqlDatabase& database,
                         ConfigObject<ConfigValue>* pConfig)
        : m_database(database),
          m_pConfig(pConfig),
          m_cDefaultImageFormat("jpg") {
    if (!QDir().mkpath(getStoragePath())) {
        qDebug() << "WARNING: Could not create cover arts storage path. "
                 << "Mixxx will be unable to store covers.";
    }
}

CoverArtDAO::~CoverArtDAO() {
    deleteUnusedCoverArts();
}

void CoverArtDAO::initialize() {
    qDebug() << "CoverArtDAO::initialize"
             << QThread::currentThread()
             << m_database.connectionName();
}

// cover art disk-cache
QString CoverArtDAO::getStoragePath() {
    return m_pConfig->getSettingsPath() % "/coverArt/";
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

void CoverArtDAO::deleteUnusedCoverArts() {
    if (m_database.isValid()) { // returns true if an error is set
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
        if (deleteFile(coverLocation)) {
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

bool CoverArtDAO::deleteFile(const QString& location) {
    QFile file(location);
    if (file.exists()) {
        return file.remove();
    }
    return true;
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
CoverArtDAO::coverArtInfo CoverArtDAO::getCoverArtInfo(int trackId) {
    if (trackId < 1) {
        return coverArtInfo();
    }

    QSqlQuery query(m_database);
    query.prepare(
        "SELECT artist, album, cover_art.location AS cover, "
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
      return coverArtInfo();
    }

    QSqlRecord queryRecord = query.record();
    const int artistColumn = queryRecord.indexOf("artist");
    const int albumColumn = queryRecord.indexOf("album");
    const int coverColumn = queryRecord.indexOf("cover");
    const int directoryColumn = queryRecord.indexOf("directory");
    const int filenameColumn = queryRecord.indexOf("filename");
    const int locationColumn = queryRecord.indexOf("location");

    if (query.next()) {
        coverArtInfo coverInfo;
        coverInfo.trackId = trackId;
        coverInfo.currentCoverLocation = query.value(coverColumn).toString();
        coverInfo.trackDirectory = query.value(directoryColumn).toString();
        coverInfo.trackLocation = query.value(locationColumn).toString();

        // building default cover art location
        QString artist = query.value(artistColumn).toString();
        QString album = query.value(albumColumn).toString();
        QString defaultCoverLoc = getStoragePath();
        if (artist.isEmpty() && album.isEmpty()) {
            defaultCoverLoc.append(query.value(filenameColumn).toString());
        } else {
            defaultCoverLoc.append(artist % " - " % album);
        }
        defaultCoverLoc.append(".");
        defaultCoverLoc.append(m_cDefaultImageFormat);
        coverInfo.defaultCoverLocation = defaultCoverLoc;
        return coverInfo;
    }

    return coverArtInfo();
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
