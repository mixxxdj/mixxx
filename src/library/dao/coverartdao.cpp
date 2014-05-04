#include <QImage>
#include <QtDebug>
#include <QThread>

#include "library/dao/coverartdao.h"
#include "library/queryutil.h"

CoverArtDAO::CoverArtDAO(QSqlDatabase& database, ConfigObject<ConfigValue>* pConfig)
        : m_pConfig(pConfig),
          m_database(database) {
    if (!QDir().mkpath(getStoragePath())) {
        qDebug() << "WARNING: Could not create cover arts storage path. "
                 << "Mixxx will be unable to store analyses.";
    }
}

CoverArtDAO::~CoverArtDAO() {
}

void CoverArtDAO::initialize() {
    qDebug() << "CoverArtDAO::initialize"
             << QThread::currentThread()
             << m_database.connectionName();
}

QString CoverArtDAO::getStoragePath() const {
    QString settingsPath = m_pConfig->getSettingsPath();
    QDir dir(settingsPath.append("/coverArt/"));
    return dir.absolutePath().append("/");
}

void CoverArtDAO::saveCoverArt(TrackInfoObject* pTrack) {
    // search cover art file (in disk) and get its location
    QString coverArtLocation = searchCoverArtFile(pTrack);

    // cover art found
    if (coverArtLocation != "") {
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
    return 0;
}

QString CoverArtDAO::searchCoverArtFile(TrackInfoObject* pTrack) {
    const char* defaultImageFormat = "jpg";

    // default cover art name
    QString coverArtName;
    if (pTrack->getArtist() != "") {
        coverArtName.append(pTrack->getArtist());
    }
    if (pTrack->getAlbum() != "") {
        coverArtName.append(pTrack->getAlbum());
    }
    if (coverArtName == "") {
        coverArtName = pTrack->getFilename();
    }

    // Starts with default location
    QString coverArtLocation = getStoragePath().append(coverArtName);

    // Some image extensions
    QStringList extList;
    extList << "jpg" << "jpeg" << "png" << "gif" << "bmp";

    //
    // Step 1: Look for cover art in cache directory.
    //
    foreach (QString ext, extList) {
        if(QFile::exists(coverArtLocation + ext)) {
            return coverArtLocation + "." + ext;
        }
    }

    //
    // Step 2: Look for embedded cover art.
    //
    QImage image = pTrack->getCoverArt();
    // If the track has embedded cover art, store it
    if (!image.isNull()) {
        if(image.save(coverArtLocation, defaultImageFormat)) {
            return coverArtLocation + "." + defaultImageFormat;
        }
    }

    //
    // Step 3: Look for cover stored in track diretory
    //
    // Implements regular expressions for image extensions
    static QList<QRegExp> regExpList;
    QLatin1String format(".(jpe?g|png|gif|bmp)");
    if (regExpList.isEmpty()) {
        regExpList << QRegExp(".*cover.*" + format, Qt::CaseInsensitive)
                   << QRegExp(".*front.*" + format, Qt::CaseInsensitive)
                   << QRegExp(".*folder.*" + format, Qt::CaseInsensitive);
    }

    const QFileInfoList fileList = QDir(pTrack->getDirectory())
            .entryInfoList(QDir::NoDotAndDotDot |
                           QDir::Files |
                           QDir::Readable);
    foreach (QFileInfo f, fileList) {
        const QString filename = f.fileName();
        foreach (QRegExp re, regExpList) {
            if (filename.contains(re)) {
                QImage image(f.absoluteFilePath());
                if (image.save(coverArtLocation, defaultImageFormat)) {
                    return coverArtLocation + "." + defaultImageFormat;
                }
                break;
            }
        }
    }

    //
    // Not found.
    //
    return "";
}
