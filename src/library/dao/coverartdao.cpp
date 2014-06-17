#include <QImage>
#include <QtDebug>
#include <QThread>

#include "library/coverartcache.h"
#include "library/queryutil.h"
#include "library/dao/coverartdao.h"
#include "soundsourceproxy.h"

CoverArtDAO::CoverArtDAO(QSqlDatabase& database,
                         ConfigObject<ConfigValue>* pConfig)
        : m_database(database),
          m_cDefaultImageFormat("jpg") {
    QString storagePath = pConfig->getSettingsPath() % "/coverArt/";
    if (!QDir().mkpath(storagePath)) {
        qDebug() << "WARNING: Could not create cover arts storage path. "
                 << "Mixxx will be unable to store covers.";
    }
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
    coverArtInfo coverInfo = getCoverArtInfo(trackId);

    QString coverLocation = coverInfo.currentCoverLocation;

    // handling cases when the file was externally removed from disk-cache
    bool removedFromDisk = false;
    if (!coverLocation.isEmpty()) {
        if (!QFile::exists(coverLocation)) {
            removedFromDisk = true;
        }
    }

    // looking for cover art in disk-cache directory.
    QString newCoverLocation = coverInfo.defaultCoverLocation;
    if(!QFile::exists(newCoverLocation)) {
        // Looking for embedded cover art.
        QImage image = searchEmbeddedCover(coverInfo.trackLocation);
        if (image.isNull()) {
            // Looking for cover stored in track diretory.
            image.load(searchInTrackDirectory(coverInfo.trackDirectory));
        }

        if (!saveImage(image, newCoverLocation)) {
            newCoverLocation.clear(); // not found
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
        coverInfo.currentCoverLocation = query.value(coverColumn).toString();
        coverInfo.trackDirectory = query.value(directoryColumn).toString();
        coverInfo.trackLocation = query.value(locationColumn).toString();

        // building default cover art location
        QString artist = query.value(artistColumn).toString();
        QString album = query.value(albumColumn).toString();
        if (artist.isEmpty() && album.isEmpty()) {
            coverInfo.defaultCoverLocation = query.value(filenameColumn).toString();
        } else {
            coverInfo.defaultCoverLocation = artist % " - " % album;
        }
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

bool CoverArtDAO::deleteFile(const QString& location) {
    QFile file(location);
    if (file.exists()) {
        return file.remove();
    }
    return true;
}

bool CoverArtDAO::saveImage(QImage cover, QString location) {
    if (cover.isNull()) {
        return false;
    }
    return cover.save(location, m_cDefaultImageFormat);
}

QString CoverArtDAO::searchInTrackDirectory(QString directory) {
    QDir dir(directory);
    dir.setFilter(QDir::NoDotAndDotDot | QDir::Files | QDir::Readable);
    dir.setSorting(QDir::Size | QDir::Reversed);

    QStringList nameFilters;
    nameFilters << "*.jpg" << "*.jpeg" << "*.png" << "*.gif" << "*.bmp";
    dir.setNameFilters(nameFilters);

    QString coverLocation;
    QStringList imglist = dir.entryList();
    if (imglist.size() > 0) {
        coverLocation = directory % "/" % imglist[0];
    }

    return coverLocation;
}

// this method will parse the information stored in the sound file
// just to extract the embedded cover art
QImage CoverArtDAO::searchEmbeddedCover(QString trackLocation) {
    SecurityTokenPointer securityToken = Sandbox::openSecurityToken(
                                             QDir(trackLocation), true);
    SoundSourceProxy proxy(trackLocation, securityToken);
    Mixxx::SoundSource* pProxiedSoundSource = proxy.getProxiedSoundSource();
    if (pProxiedSoundSource != NULL && proxy.parseHeader() == OK) {
        return pProxiedSoundSource->getCoverArt();
    }
    return QImage();
}
