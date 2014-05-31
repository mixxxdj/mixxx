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
    return cover.save(location, m_cDefaultImageFormat);
}

QString CoverArt::saveEmbeddedCover(QImage cover, QString artist,
                                    QString album, QString filename) {
    if (cover.isNull()) {
        return "";
    }

    QString coverName = getDefaultCoverName(artist, album, filename);
    QString location = getDefaultCoverLocation(coverName);

    if (saveFile(cover, location)) {
        return location;
    }

    return "";
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

QString CoverArt::searchCoverArtFile(TrackPointer pTrack) {
    //
    // Step 1: Look for cover art in disk-cache directory.
    //
    QString coverLocation = getDefaultCoverLocation(pTrack);
    if(QFile::exists(coverLocation)) {
        return coverLocation; // FOUND!
    }

    //
    // Step 2: Look for embedded cover art.
    //
    coverLocation = pTrack->parseCoverArt();
    if (!coverLocation.isEmpty()) {
        return coverLocation; // FOUND!
    }

    //
    // Step 3: Look for cover stored in track diretory.
    //
    QImage image(searchInTrackDirectory(pTrack->getDirectory()));

    if (!image.isNull()) {
        // try to store the image in our disk-cache!
        coverLocation = getDefaultCoverLocation(pTrack);
        if (saveFile(image, coverLocation)) {
            return coverLocation; // FOUND!
        }
    }

    //
    // Not found.
    //
    return QString();
}
