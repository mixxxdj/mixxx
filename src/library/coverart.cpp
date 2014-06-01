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

bool CoverArt::saveImage(QImage cover, QString location) {
    if (cover.isNull()) {
        return false;
    }

    return cover.save(location, m_cDefaultImageFormat);
}

QString CoverArt::searchInTrackDirectory(QString directory) {
    // Implements regular expressions for image extensions
    static QList<QRegExp> regExpList;
    QLatin1String format(".(jpe?g|png|gif|bmp)");
    if (regExpList.isEmpty()) {
        regExpList << QRegExp(".*cover.*" + format, Qt::CaseInsensitive)
                   << QRegExp(".*album.*" + format, Qt::CaseInsensitive)
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

QString CoverArt::getDefaultCoverName(QString artist,
                                      QString album,
                                      QString filename) {
    if (artist.isEmpty() && album.isEmpty()) {
         return filename;
    } else {
        return artist + " - " + album;
    }
}

QString CoverArt::getDefaultCoverLocation(QString coverArtName) {
    return getStoragePath() + coverArtName + "." + m_cDefaultImageFormat;
}

QString CoverArt::getDefaultCoverLocation(TrackPointer pTrack) {
    QString coverArtName = getDefaultCoverName(pTrack->getArtist(),
                                               pTrack->getAlbum(),
                                               pTrack->getFilename());

    return getDefaultCoverLocation(coverArtName);
}

QImage CoverArt::searchCoverArt(TrackPointer pTrack) {
    //
    // Step 1: Look for embedded cover art.
    //
    QImage image = pTrack->parseCoverArt();
    if (!image.isNull()) {
        return image; // FOUND!
    }

    //
    // Step 2: Look for cover stored in track diretory.
    //
    image.load(searchInTrackDirectory(pTrack->getDirectory()));
    if (!image.isNull()) {
        return image; // FOUND!
    }

    return QImage();
}
