#include "library/coverartutils.h"

#include <QDir>
#include <QDirIterator>
#include <QtConcurrentRun>

#include "sources/soundsourceproxy.h"
#include "track/track.h"
#include "util/compatibility.h"
#include "util/logger.h"
#include "util/regex.h"


namespace {

mixxx::Logger kLogger("CoverArtUtils");

// The concurrent guessing of cover art in background tasks
// is enabled, unless it is explicitly disabled during tests!
volatile bool s_enableConcurrentGuessingOfTrackCoverInfo = true;

} // anonymous namespace

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
        TrackFile trackFile,
        SecurityTokenPointer pToken) {
    return SoundSourceProxy::importTemporaryCoverImage(
            std::move(trackFile), std::move(pToken));
}

//static
QList<QFileInfo> CoverArtUtils::findPossibleCoversInFolder(const QString& folder) {
    // Search for image files in the track directory.
    QRegExp coverArtFilenames(supportedCoverArtExtensionsRegex(),
                              Qt::CaseInsensitive);
    QDirIterator it(folder,
                    QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
    QFile currentFile;
    QFileInfo currentFileInfo;
    QList<QFileInfo> possibleCovers;
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
CoverInfoRelative CoverArtUtils::selectCoverArtForTrack(
        const Track& track,
        const QList<QFileInfo>& covers) {
    return selectCoverArtForTrack(
            track.getFileInfo(),
            track.getAlbum(),
            covers);
}

//static
CoverInfoRelative CoverArtUtils::selectCoverArtForTrack(
        const TrackFile& trackFile,
        const QString& albumName,
        const QList<QFileInfo>& covers) {
    CoverInfoRelative coverInfoRelative;
    DEBUG_ASSERT(coverInfoRelative.type == CoverInfo::NONE);
    DEBUG_ASSERT(coverInfoRelative.imageDigest().isNull());
    DEBUG_ASSERT(coverInfoRelative.coverLocation.isNull());
    coverInfoRelative.source = CoverInfo::GUESSED;
    if (covers.isEmpty()) {
        return coverInfoRelative;
    }

    PreferredCoverType bestType = NONE;
    const QFileInfo* bestInfo = nullptr;

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
                bestInfo = &file;
                // This is the best type (TRACK_BASENAME) so we know we're done.
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

    if (bestInfo) {
        const QImage image(bestInfo->filePath());
        if (!image.isNull()) {
            coverInfoRelative.type = CoverInfo::FILE;
            coverInfoRelative.coverLocation = bestInfo->fileName();
            coverInfoRelative.setImage(image);
        }
    }

    return coverInfoRelative;
}

CoverInfoRelative CoverInfoGuesser::guessCoverInfo(
        const TrackFile& trackFile,
        const QString& albumName,
        const QImage& embeddedCover) {
    if (!embeddedCover.isNull()) {
        CoverInfoRelative coverInfo;
        coverInfo.source = CoverInfo::GUESSED;
        coverInfo.type = CoverInfo::METADATA;
        coverInfo.setImage(embeddedCover);
        DEBUG_ASSERT(coverInfo.coverLocation.isNull());
        return coverInfo;
    }

    const auto trackFolder = trackFile.directory();
    if (trackFolder != m_cachedFolder) {
        m_cachedFolder = trackFile.directory();
        m_cachedPossibleCoversInFolder =
                CoverArtUtils::findPossibleCoversInFolder(
                        m_cachedFolder);
    }
    return CoverArtUtils::selectCoverArtForTrack(
            trackFile,
            albumName,
            m_cachedPossibleCoversInFolder);
}

CoverInfoRelative CoverInfoGuesser::guessCoverInfoForTrack(
        const Track& track) {
    const auto trackFile = track.getFileInfo();
    if (kLogger.debugEnabled()) {
        kLogger.debug()
                << "Guessing cover art for track"
                << trackFile;
    }
    return guessCoverInfo(
            trackFile,
            track.getAlbum(),
            CoverArtUtils::extractEmbeddedCover(
                    trackFile,
                    track.getSecurityToken()));
}

void CoverInfoGuesser::guessAndSetCoverInfoForTrack(
        Track& track) {
    track.setCoverInfo(guessCoverInfoForTrack(track));
}

void CoverInfoGuesser::guessAndSetCoverInfoForTracks(
        const TrackPointerList& tracks) {
    for (const auto& pTrack : tracks) {
        VERIFY_OR_DEBUG_ASSERT(pTrack) {
            continue;
        }
        guessAndSetCoverInfoForTrack(*pTrack);
    }
}

void guessTrackCoverInfoConcurrently(
        TrackPointer pTrack) {
    VERIFY_OR_DEBUG_ASSERT(pTrack) {
        return;
    }
    if (s_enableConcurrentGuessingOfTrackCoverInfo) {
        QtConcurrent::run([pTrack] {
            CoverInfoGuesser().guessAndSetCoverInfoForTrack(*pTrack);
        });
    } else {
        // Disabled only during tests
        CoverInfoGuesser().guessAndSetCoverInfoForTrack(*pTrack);
    }
}

void disableConcurrentGuessingOfTrackCoverInfoDuringTests() {
    s_enableConcurrentGuessingOfTrackCoverInfo = false;
}
