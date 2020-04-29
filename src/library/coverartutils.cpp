#include <QDir>
#include <QDirIterator>

#include "library/coverartutils.h"

#include "sources/soundsourceproxy.h"
#include "util/regex.h"


//static
QString CoverArtUtils::defaultCoverLocation() {
    return QString(":/images/library/cover_default.svg");
}

//static
QStringList CoverArtUtils::supportedCoverArtExtensions() {
    QStringList extensions;
    extensions << "jpg" << "jpeg" << "png" << "gif" << "bmp";
    return extensions;
}

//static
QString CoverArtUtils::supportedCoverArtExtensionsRegex() {
    QStringList extensions = supportedCoverArtExtensions();
    return RegexUtils::fileExtensionsRegex(extensions);
}

//static
QImage CoverArtUtils::extractEmbeddedCover(
        TrackFile trackFile) {
    SecurityTokenPointer pToken = Sandbox::openSecurityToken(
            trackFile.asFileInfo(), true);
    return extractEmbeddedCover(
            std::move(trackFile), std::move(pToken));
}

//static
QImage CoverArtUtils::extractEmbeddedCover(
        TrackFile trackFile,
        SecurityTokenPointer pToken) {
    return SoundSourceProxy::importTemporaryCoverImage(
            std::move(trackFile), std::move(pToken));
}

//static
QImage CoverArtUtils::loadCover(const CoverInfo& info) {
    if (info.type == CoverInfo::METADATA) {
        if (info.trackLocation.isEmpty()) {
            qDebug() << "CoverArtUtils::loadCover METADATA cover with empty trackLocation.";
            return QImage();
        }
        const TrackFile trackFile(info.trackLocation);
        return extractEmbeddedCover(trackFile);
    } else if (info.type == CoverInfo::FILE) {
        if (info.trackLocation.isEmpty()) {
            qDebug() << "CoverArtUtils::loadCover FILE cover with empty trackLocation."
                     << "Relative paths will not work.";
            SecurityTokenPointer pToken = Sandbox::openSecurityToken(
                QFileInfo(info.coverLocation), true);
            return QImage(info.coverLocation);
        }

        QFileInfo coverFile(TrackFile(info.trackLocation).directory(), info.coverLocation);
        QString coverFilePath = coverFile.filePath();
        if (!coverFile.exists()) {
            qDebug() << "CoverArtUtils::loadCover FILE cover does not exist:"
                     << coverFilePath;
            return QImage();
        }
        SecurityTokenPointer pToken = Sandbox::openSecurityToken(
            coverFile, true);
        return QImage(coverFilePath);
    } else if (info.type == CoverInfo::NONE) {
        return QImage();
    } else {
        qDebug() << "CoverArtUtils::loadCover unhandled type";
        DEBUG_ASSERT(false);
        return QImage();
    }
}

//static
CoverInfo CoverArtUtils::guessCoverInfo(const Track& track) {
    CoverInfo coverInfo;

    coverInfo.trackLocation = track.getLocation();
    coverInfo.source = CoverInfo::GUESSED;

    const auto trackFile = track.getFileInfo();
    QImage image = extractEmbeddedCover(trackFile, track.getSecurityToken());
    if (!image.isNull()) {
        // TODO() here we my introduce a duplicate hash code
        coverInfo.hash = calculateHash(image);
        coverInfo.coverLocation = QString();
        coverInfo.type = CoverInfo::METADATA;
        qDebug() << "CoverArtUtils::guessCover found metadata art" << coverInfo;
        return coverInfo;
    }

    QLinkedList<QFileInfo> possibleCovers = findPossibleCoversInFolder(
            trackFile.directory());
    coverInfo = selectCoverArtForTrack(track, possibleCovers);
    if (coverInfo.type == CoverInfo::FILE) {
        qDebug() << "CoverArtUtils::guessCover found file art" << coverInfo;
    } else {
        qDebug() << "CoverArtUtils::guessCover didn't find art" << coverInfo;
    }
    return coverInfo;
}

//static
QLinkedList<QFileInfo> CoverArtUtils::findPossibleCoversInFolder(const QString& folder) {
    // Search for image files in the track directory.
    QRegExp coverArtFilenames(supportedCoverArtExtensionsRegex(),
                              Qt::CaseInsensitive);
    QDirIterator it(folder,
                    QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
    QFile currentFile;
    QFileInfo currentFileInfo;
    QLinkedList<QFileInfo> possibleCovers;
    while (it.hasNext()) {
        it.next();
        currentFileInfo = it.fileInfo();
        if (currentFileInfo.isFile() &&
            coverArtFilenames.indexIn(currentFileInfo.fileName()) != -1) {
            possibleCovers.append(currentFileInfo);
        }
    }
    return possibleCovers;
}

//static
CoverInfo CoverArtUtils::selectCoverArtForTrack(
        const Track& track,
        const QLinkedList<QFileInfo>& covers) {

    CoverInfoRelative coverInfoRelative =
            selectCoverArtForTrack(track.getFileInfo(), track.getAlbum(), covers);
    return CoverInfo(coverInfoRelative, track.getLocation());
}

//static
CoverInfoRelative CoverArtUtils::selectCoverArtForTrack(
        const TrackFile& trackFile,
        const QString& albumName,
        const QLinkedList<QFileInfo>& covers) {
    CoverInfoRelative coverInfoRelative;
    coverInfoRelative.source = CoverInfo::GUESSED;
    if (covers.isEmpty()) {
        return coverInfoRelative;
    }

    PreferredCoverType bestType = NONE;
    const QFileInfo* bestInfo = NULL;

    // If there is a single image then we use it unconditionally. Otherwise
    // we use the priority order described in PreferredCoverType. Notably,
    // if there are multiple image files in the folder we require they match
    // one of the name patterns -- otherwise we run the risk of picking an
    // arbitrary image that happens to be in the same folder as some of the
    // user's music files.
    if (covers.size() == 1) {
        bestInfo = &covers.first();
    } else {
        // TODO(XXX) Sort instead so that we can fall-back if one fails to
        // open?
        foreach (const QFileInfo& file, covers) {
            const QString coverBaseName = file.baseName();
            if (bestType > TRACK_BASENAME &&
                coverBaseName.compare(trackFile.baseName(),
                                      Qt::CaseInsensitive) == 0) {
                bestType = TRACK_BASENAME;
                bestInfo = &file;
                // This is the best type so we know we're done.
                break;
            } else if (bestType > ALBUM_NAME &&
                       coverBaseName.compare(albumName,
                                             Qt::CaseInsensitive) == 0) {
                bestType = ALBUM_NAME;
                bestInfo = &file;
            } else if (bestType > COVER &&
                       coverBaseName.compare(QLatin1String("cover"),
                                             Qt::CaseInsensitive) == 0) {
                bestType = COVER;
                bestInfo = &file;
            } else if (bestType > FRONT &&
                       coverBaseName.compare(QLatin1String("front"),
                                             Qt::CaseInsensitive) == 0) {
                bestType = FRONT;
                bestInfo = &file;
            } else if (bestType > ALBUM &&
                       coverBaseName.compare(QLatin1String("album"),
                                             Qt::CaseInsensitive) == 0) {
                bestType = ALBUM;
                bestInfo = &file;
            } else if (bestType > FOLDER &&
                       coverBaseName.compare(QLatin1String("folder"),
                                             Qt::CaseInsensitive) == 0) {
                bestType = FOLDER;
                bestInfo = &file;
            }
        }
    }

    if (bestInfo != NULL) {
        QImage image(bestInfo->filePath());
        if (!image.isNull()) {
            coverInfoRelative.source = CoverInfo::GUESSED;
            coverInfoRelative.type = CoverInfo::FILE;
            // TODO() here we may introduce a duplicate hash code
            coverInfoRelative.hash = calculateHash(image);
            coverInfoRelative.coverLocation = bestInfo->fileName();
            return coverInfoRelative;
        }
    }

    return coverInfoRelative;
}
