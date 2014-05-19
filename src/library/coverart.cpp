#include "library/coverart.h"

CoverArt::CoverArt()
        : m_pConfig(NULL),
          m_cDefaultImageFormat("jpg") {
}

CoverArt::~CoverArt() {
}

void CoverArt::setConfig(ConfigObject<ConfigValue>* pConfig) {
    m_pConfig = pConfig;

    if (!QDir().mkpath(getStoragePath())) {
        qDebug() << "WARNING: Could not create cover arts storage path. "
                 << "Mixxx will be unable to store analyses.";
    }
}

QString CoverArt::getStoragePath() const {
    QString settingsPath = m_pConfig->getSettingsPath();
    QDir dir(settingsPath.append("/coverArt/"));
    return dir.absolutePath().append("/");
}

bool CoverArt::deleteFile(const QString& location) {
    QFile file(location);
    if (file.exists()) {
        return file.remove();
    }
    return true;
}

bool CoverArt::saveFile(QImage cover, QString location) {
    if (cover.save(location, m_cDefaultImageFormat)) {
        return true;
    }
    return false;
}

QString CoverArt::searchInDiskCache(QString coverArtName) {
    // Some image extensions
    QStringList extList;
    extList << ".jpg" << ".jpeg" << ".png" << ".gif" << ".bmp";

    // Look for cover art in disk-cache directory.
    QString location = getStoragePath().append(coverArtName);
    foreach (QString ext, extList) {
        if(QFile::exists(location + ext)) {
            return location + ext;
        }
    }

    return QString();
}

QString CoverArt::searchInTrackDirectory(QString directory) {
    // Implements regular expressions for image extensions
    static QList<QRegExp> regExpList;
    QLatin1String format(".(jpe?g|png|gif|bmp)");
    if (regExpList.isEmpty()) {
        regExpList << QRegExp(".*cover.*" + format, Qt::CaseInsensitive)
                   << QRegExp(".*front.*" + format, Qt::CaseInsensitive)
                   << QRegExp(".*folder.*" + format, Qt::CaseInsensitive);
    }

    const QFileInfoList fileList = QDir(directory)
            .entryInfoList(QDir::NoDotAndDotDot |
                           QDir::Files |
                           QDir::Readable);
    foreach (QFileInfo f, fileList) {
        const QString filename = f.fileName();
        foreach (QRegExp re, regExpList) {
            if (filename.contains(re)) {
                return f.absoluteFilePath();
            }
        }
    }

    return QString();
}

QString CoverArt::getDefaultCoverLocation(QString coverArtName) {
    return getStoragePath() + coverArtName + "." + m_cDefaultImageFormat;
}

QString CoverArt::getDefaultCoverName(QString artist,
                                      QString album,
                                      QString filename) {
    if (artist.isEmpty() && album.isEmpty()) {
         return filename;
    } else {
        return artist + " - " + album;
    }
}

QString CoverArt::searchCoverArtFile(TrackInfoObject* pTrack) {
    // creates default cover art name
    QString coverArtName = getDefaultCoverName(pTrack->getArtist(),
                                               pTrack->getAlbum(),
                                               pTrack->getFilename());

    //
    // Step 1: Look for cover art in disk-cache directory.
    //
    QString coverArtLocation = searchInDiskCache(coverArtName);

    if (!coverArtLocation.isEmpty()) {
        return coverArtLocation;  // FOUND!
    }

    // load default location
    coverArtLocation = getDefaultCoverLocation(coverArtName);

    //
    // Step 2: Look for embedded cover art.
    //
    QImage image = pTrack->getEmbeddedCoverArt();

    // If the track doesn't have embedded cover art
    if (image.isNull()) {
        //
        // Step 3: Look for cover stored in track diretory
        //
        image = QImage(searchInTrackDirectory(pTrack->getDirectory()));
    }

    // try to store the image found in our disk-cache!
    if (saveFile(image, coverArtLocation)) {
        return coverArtLocation;  // FOUND!
    }

    //
    // Not found.
    //
    return QString();
}
