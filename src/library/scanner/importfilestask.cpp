#include "library/scanner/importfilestask.h"

#include "trackinfocache.h"
#include "soundsourceproxy.h"
#include "library/scanner/libraryscanner.h"
#include "library/coverartutils.h"
#include "util/timer.h"

ImportFilesTask::ImportFilesTask(LibraryScanner* pScanner,
                                 const ScannerGlobalPointer scannerGlobal,
                                 const QString& dirPath,
                                 const bool prevHashExists,
                                 const int newHash,
                                 const QLinkedList<QFileInfo>& filesToImport,
                                 const QLinkedList<QFileInfo>& possibleCovers,
                                 SecurityTokenPointer pToken)
        : ScannerTask(pScanner, scannerGlobal), m_dirPath(dirPath),
          m_prevHashExists(prevHashExists), m_newHash(newHash),
          m_filesToImport(filesToImport),
          m_possibleCovers(possibleCovers),
          m_pToken(pToken) {
}

void ImportFilesTask::run() {
    ScopedTimer timer("ImportFilesTask::run");
    foreach (const QFileInfo& file, m_filesToImport) {
        // If a flag was raised telling us to cancel the library scan then stop.
        if (m_scannerGlobal->shouldCancel()) {
            setSuccess(false);
            return;
        }

        QString filePath = file.filePath();
        //qDebug() << "ImportFilesTask::run" << filePath;

        // If the file does not exist in the database then add it. If it
        // does then it is either in the user's library OR the user has
        // "removed" the track via "Right-Click -> Remove". These tracks
        // stay in the library, but their mixxx_deleted column is 1.
        if (m_scannerGlobal->trackExistsInDatabase(filePath)) {
            // If the track is in the database, mark it as existing. This code gets
            // executed when other files in the same directory have changed (the
            // directory hash has changed).
            emit(trackExists(filePath));
        } else {
            const TrackRef trackRef(file);
            TrackInfoCacheLocker cacheLocker(
                    TrackInfoCache::instance().resolve(trackRef));
            TrackPointer pTrack(cacheLocker.getResolvedTrack());
            if (pTrack.isNull()) {
                qWarning() << "ImportFilesTask: Skipping inaccessible file"
                        << trackRef;
                continue;
            }

            SoundSourceProxy(pTrack).loadTrackMetadataAndCoverArt();

            // If cover art is not found in the track metadata, populate from
            // possibleCovers.
            // This is a new (never before seen) track so it is safe
            // to parse cover art without checking if we have cover art
            // that is USER_SELECTED. If this changes in the future you
            // MUST check that the cover art is not USER_SELECTED first.
            if (pTrack->getCoverArt().image.isNull()) {
                CoverArt art = CoverArtUtils::selectCoverArtForTrack(
                    pTrack.data(), m_possibleCovers);
                if (!art.image.isNull()) {
                    pTrack->setCoverArt(art);
                }
            }

            // Unlock the cache before emitting any signals
            cacheLocker.unlockCache();

            emit(addNewTrack(pTrack));
        }
    }
    // Insert or update the hash in the database.
    emit(directoryHashedAndScanned(m_dirPath, !m_prevHashExists, m_newHash));
    setSuccess(true);
}
