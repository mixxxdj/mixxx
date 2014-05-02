#include <QImage>
#include <QtDebug>
#include <QThread>

#include "library/dao/coverartdao.h"

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

void CoverArtDAO::saveCoverArt(TrackInfoObject* pTrack) {
    QString coverArtLocation = searchCoverArt(pTrack);

    // cover art found
    if (coverArtLocation != "") {
        // TODO: sql insertion
    }
}

QString CoverArtDAO::searchCoverArt(TrackInfoObject* pTrack) {
    const char* defaultImageFormat = "JPG";
    QString coverArtName = QString("%1 %2")
                                   .arg(pTrack->getArtist())
                                   .arg(pTrack->getAlbum());

    // Starts with default location
    QString coverArtLocation = getStoragePath().append(coverArtName);

    // Some image extensions
    QLatin1String format(".(jpe?g|png|gif|bmp)");

    //
    // Step 1: Look for cover art in cache directory.
    //
    if(QFile::exists(coverArtLocation + format)) {
        return coverArtLocation;
    }

    //
    // Step 2: Look for embedded cover art.
    //
    QImage image = pTrack->getCoverArt();
    // If the track has embedded cover art, store it
    if (!image.isNull()) {
        if(image.save(coverArtLocation, defaultImageFormat)) {
            return coverArtLocation;
        }
    }

    //
    // Step 3: Look for cover stored in track diretory
    //
    // Implements regular expressions for image extensions
    static QList<QRegExp> regExpList;
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
                    return coverArtLocation;
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

QString CoverArtDAO::getStoragePath() const {
    QString settingsPath = m_pConfig->getSettingsPath();
    QDir dir(settingsPath.append("/coverArt/"));
    return dir.absolutePath().append("/");
}
