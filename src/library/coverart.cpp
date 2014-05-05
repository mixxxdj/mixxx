#include "library/coverart.h"

CoverArt::CoverArt(ConfigObject<ConfigValue>* pConfig)
        : m_pConfig(pConfig) {
    if (!QDir().mkpath(getStoragePath())) {
        qDebug() << "WARNING: Could not create cover arts storage path. "
                 << "Mixxx will be unable to store analyses.";
    }
}

CoverArt::~CoverArt() {
}

QString CoverArt::getStoragePath() const {
    QString settingsPath = m_pConfig->getSettingsPath();
    QDir dir(settingsPath.append("/coverArt/"));
    return dir.absolutePath().append("/");
}

QString CoverArt::searchCoverArtFile(TrackInfoObject* pTrack) {
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
    extList << ".jpg" << ".jpeg" << ".png" << ".gif" << ".bmp";

    //
    // Step 1: Look for cover art in cache directory.
    //
    foreach (QString ext, extList) {
        if(QFile::exists(coverArtLocation + ext)) {
            return coverArtLocation + ext;
        }
    }
    coverArtLocation.append(".");
    coverArtLocation.append(defaultImageFormat);

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
