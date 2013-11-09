/***************************************************************************
                        libraryscanner.cpp  -  scans library in a thread
                            -------------------
    begin                : 11/27/2007
    copyright            : (C) 2007 Albert Santoni
    email                : gamegod \a\t users.sf.net
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <QtCore>
#include <QtDebug>
#include <QDesktopServices>
#include <QTimer>

#include "soundsourceproxy.h"
#include "library/legacylibraryimporter.h"
#include "libraryscanner.h"
#include "libraryscannerdlg.h"
#include "library/queryutil.h"
#include "trackinfoobject.h"

#define MAX_CHUNK_SIZE 50

LibraryScanner::LibraryScanner(TrackCollection* pTrackCollection) :
    m_pTrackCollection(pTrackCollection),
    m_database(pTrackCollection->getDatabase()),
    m_pProgress(NULL),
    m_libraryHashDao(m_database),
    m_cueDao(m_database),
    m_playlistDao(m_database),
    m_crateDao(m_database),
    m_analysisDao(m_database, pTrackCollection->getConfig()),
    m_trackDao(m_database, m_cueDao, m_playlistDao, m_crateDao,
               m_analysisDao, pTrackCollection->getConfig()),
    m_nameFilters(SoundSourceProxy::supportedFileExtensionsString().split(" ")),
    m_bCancelLibraryScan(false) {

    qDebug() << "Constructed LibraryScanner";

    // Force the GUI thread's TrackInfoObject cache to be cleared when a library
    // scan is finished, because we might have modified the database directly
    // when we detected moved files, and the TIOs corresponding to the moved
    // files would then have the wrong track location.
    connect(this, SIGNAL(scanFinished()),
            &(pTrackCollection->getTrackDAO()), SLOT(clearCache()));

    // The "Album Artwork" folder within iTunes stores Album Arts.
    // It has numerous hundreds of sub folders but no audio files
    // We put this folder on a "black list"
    // On Windows, the iTunes folder is contained within the standard music folder
    // Hence, Mixxx will scan the "Album Arts folder" for standard users which is wasting time
    QString iTunesArtFolder = QDir::toNativeSeparators(
                QDesktopServices::storageLocation(QDesktopServices::MusicLocation) + "/iTunes/Album Artwork" );
    m_directoriesBlacklist << iTunesArtFolder;
    qDebug() << "iTunes Album Art path is:" << iTunesArtFolder;

#ifdef __WINDOWS__
    //Blacklist the _Serato_ directory that pollutes "My Music" on Windows.
    QString seratoDir = QDir::toNativeSeparators(
                QDesktopServices::storageLocation(QDesktopServices::MusicLocation) + "/_Serato_" );
    m_directoriesBlacklist << seratoDir;
#endif
}

LibraryScanner::~LibraryScanner() {
    // IMPORTANT NOTE: This code runs in the GUI thread, so it should _NOT_ use
    //                the m_trackDao that lives inside this class. It should use
    //                the DAOs that live in m_pTrackCollection.

    if (isRunning()) {
        // Cancel any running library scan...
        cancel();
        wait(); // Wait for thread to finish
    }

    // tro's lambda idea. This code calls Synchronously!
    m_pTrackCollection->callSync(
                [this] (void) {
        // Do housekeeping on the LibraryHashes table.
        ScopedTransaction transaction(m_pTrackCollection->getDatabase());

        // Mark the corresponding file locations in the track_locations table as deleted
        // if we find one or more deleted directories.
        QStringList deletedDirs;
        QSqlQuery query(m_pTrackCollection->getDatabase());
        query.prepare("SELECT directory_path FROM LibraryHashes "
                      "WHERE directory_deleted=1");
        if (query.exec()) {
            const int directoryPathColumn = query.record().indexOf("directory_path");
            while (query.next()) {
                QString directory = query.value(directoryPathColumn).toString();
                deletedDirs << directory;
            }
        } else {
            LOG_FAILED_QUERY(query) << "Couldn't SELECT deleted directories.";
        }

        // Delete any directories that have been marked as deleted...
        query.finish();
        query.exec("DELETE FROM LibraryHashes "
                   "WHERE directory_deleted=1");

        // Print out any SQL error, if there was one.
        if (query.lastError().isValid()) {
            LOG_FAILED_QUERY(query);
        }

        QString dir;
        foreach(dir, deletedDirs) {
            m_pTrackCollection->getTrackDAO().markTrackLocationsAsDeleted(dir);
        }
        transaction.commit();
    }, __PRETTY_FUNCTION__);

    qDebug() << "LibraryScanner destroyed";
}

void LibraryScanner::run() {
    unsigned static id = 0; // the id of this thread, for debugging purposes
            //XXX copypasta (should factor this out somehow), -kousu 2/2009
    QThread::currentThread()->setObjectName(QString("LibraryScanner %1").arg(++id));
    //m_pProgress->slotStartTiming();

    // Lower our priority to help not grind crappy computers.
    setPriority(QThread::LowPriority);

    qRegisterMetaType<QSet<int> >("QSet<int>");

    m_libraryHashDao.setDatabase(m_database);
    m_cueDao.setDatabase(m_database);
    m_trackDao.setDatabase(m_database);
    m_playlistDao.setDatabase(m_database);
    m_analysisDao.setDatabase(m_database);

    m_libraryHashDao.initialize();
    m_cueDao.initialize();
    m_trackDao.initialize();
    m_playlistDao.initialize();
    m_analysisDao.initialize();

    resetCancel();

    QTime t2;
    t2.start();

    // Try to upgrade the library from 1.7 (XML) to 1.8+ (DB) if needed. If the
    // upgrade_filename already exists, then do not try to upgrade since we have
    // already done it.
    // TODO(XXX) SETTINGS_PATH may change in new Mixxx Versions. Here we need
    // the SETTINGS_PATH from Mixxx V <= 1.7
    QString upgrade_filename = QDir::homePath().append("/").append(SETTINGS_PATH).append("DBUPGRADED");
    qDebug() << "upgrade filename is " << upgrade_filename;
    QFile upgradefile(upgrade_filename);
    if (!upgradefile.exists()) {
        LegacyLibraryImporter libImport(m_pTrackCollection, m_trackDao, m_playlistDao);
        connect(&libImport, SIGNAL(progress(QString)),
                m_pProgress, SLOT(slotUpdate(QString)),
                Qt::BlockingQueuedConnection);
        ScopedTransaction transaction(m_database);
        libImport.import();
        transaction.commit();
        qDebug("Legacy importer took %d ms", t2.elapsed());
    }

    // Refresh the name filters in case we loaded new
    // SoundSource plugins.
    m_nameFilters = SoundSourceProxy::supportedFileExtensionsString().split(" ");

    // Time the library scanner.
    QTime t;
    t.start();

    m_pTrackCollection->callSync([this](void) {
        // First, we're going to mark all the directories that we've
        // previously hashed as needing verification. As we search through the directory tree
        // when we rescan, we'll mark any directory that does still exist as verified.
        m_libraryHashDao.invalidateAllDirectories();
        // Mark all the tracks in the library as needing
        // verification of their existance...
        // (ie. we want to check they're still on your hard drive where
        // we think they are)
        m_trackDao.invalidateTrackLocationsInLibrary(m_qLibraryPath);
    }, __FUNCTION__);


    qDebug() << "Recursively scanning library.";
    // Start scanning the library.
    // this will prepare some querys in TrackDAO, this needs be done because
    // TrackCollection will call TrackDAO::addTracksAdd and this
    // function needs the querys
    QStringList verifiedDirectories;

    bool bScanFinishedCleanly = recursiveScan(m_qLibraryPath, verifiedDirectories);                                     ////////////////////////////////////

    if (!bScanFinishedCleanly) {
        qDebug() << "Recursive scan interrupted.";
    } else {
        qDebug() << "Recursive scan finished cleanly.";
    }

    //Verify all Tracks inside Library but outside the library path
    m_trackDao.verifyTracksOutside(m_qLibraryPath, &m_bCancelLibraryScan);


    // tro's lambda idea. This code calls Synchronously!
    m_pTrackCollection->callSync(
                [this, &bScanFinishedCleanly, &verifiedDirectories, &t] (void) {
        // Start a transaction for all the library hashing (moved file detection)
        // stuff.
        ScopedTransaction transaction(m_database);

        // At the end of a scan, mark all tracks and directories that
        // weren't "verified" as "deleted" (as long as the scan wasn't canceled
        // half way through. This condition is important because our rescanning
        // algorithm starts by marking all tracks and dirs as unverified, so a
        // canceled scan might leave half of your library as unverified. Don't
        // want to mark those tracks/dirs as deleted in that case) :)
        QSet<int> tracksMovedSetOld;
        QSet<int> tracksMovedSetNew;
        if (bScanFinishedCleanly) {
            qDebug() << "Marking unchanged directories and tracks as verified";
            m_libraryHashDao.updateDirectoryStatuses(verifiedDirectories, false, true);
            m_trackDao.markTracksInDirectoriesAsVerified(verifiedDirectories);

            qDebug() << "Marking unverified tracks as deleted.";
            m_trackDao.markUnverifiedTracksAsDeleted();
            qDebug() << "Marking unverified directories as deleted.";
            m_libraryHashDao.markUnverifiedDirectoriesAsDeleted();

            // Check to see if the "deleted" tracks showed up in another location,
            // and if so, do some magic to update all our tables.
            qDebug() << "Detecting moved files.";
            m_trackDao.detectMovedFiles(&tracksMovedSetOld, &tracksMovedSetNew);

            // Remove the hashes for any directories that have been
            // marked as deleted to clean up. We need to do this otherwise
            // we can skip over songs if you move a set of songs from directory
            // A to B, then back to A.
            m_libraryHashDao.removeDeletedDirectoryHashes();

            transaction.commit();
            qDebug() << "Scan finished cleanly";
        } else {
            transaction.rollback();
            qDebug() << "Scan cancelled";
        }

        qDebug("Scan took: %d ms", t.elapsed());

        // Update BaseTrackCache via the main TrackDao
        m_pTrackCollection->getTrackDAO().databaseTracksMoved(tracksMovedSetOld, tracksMovedSetNew);

        emit(scanFinished());
    }, __PRETTY_FUNCTION__);
}

void LibraryScanner::scan(const QString& libraryPath, QWidget *parent) {
    m_qLibraryPath = libraryPath;
    m_pProgress = new LibraryScannerDlg(parent);
    m_pProgress->setAttribute(Qt::WA_DeleteOnClose);

    connect(this, SIGNAL(progressLoading(QString)),
            m_pProgress, SLOT(slotUpdate(QString)),
            Qt::QueuedConnection);
    connect(this, SIGNAL(progressHashing(QString)),
            m_pProgress, SLOT(slotUpdate(QString)),
            Qt::QueuedConnection);
    connect(this, SIGNAL(scanFinished()),
            m_pProgress, SLOT(slotScanFinished()),
            Qt::QueuedConnection);
    connect(m_pProgress, SIGNAL(scanCancelled()),
            this, SLOT(cancel()),
            Qt::QueuedConnection);
    connect(m_pProgress, SIGNAL(scanPaused()),
            this, SLOT(pause()),
            Qt::QueuedConnection);
    connect(m_pProgress, SIGNAL(scanResumed()),
            this, SLOT(resume()),
            Qt::QueuedConnection);
    connect(&m_trackDao, SIGNAL(progressVerifyTracksOutside(QString)),
            m_pProgress, SLOT(slotUpdate(QString)),
            Qt::QueuedConnection);

    connect(&m_timer, SIGNAL(timeout()), this, SLOT(updateProgress()));
    m_timer.start(500);

    scan();
}

//slot
void LibraryScanner::cancel() {
    m_bCancelLibraryScan = true;
    m_pauseMutex.unlock();
}

void LibraryScanner::resetCancel() {
    m_bCancelLibraryScan = false;
}

void LibraryScanner::pause() {
    m_pauseMutex.lock();
}

void LibraryScanner::resume() {
    m_pauseMutex.unlock();
}

void LibraryScanner::updateProgress() {
    emit(progressLoading(m_tmpTrackPath));
}

void LibraryScanner::scan() {
    start(); // Starts the thread by calling run()
}

// Do a non-recursive import of all the songs in a directory. Does NOT decend into subdirectories.
//    @param trackDao The track data access object which provides a connection to the database.
//    We use this parameter in order to make this function callable from separate threads. You need to use a different DB connection for each thread.
//    @return true if the scan completed without being cancelled. False if the scan was cancelled part-way through.
//
bool LibraryScanner::importDirectory(const QString& directory,
                                     const QStringList& nameFilters,
                                     volatile bool* cancel) {
    //qDebug() << "TrackCollection::importDirectory(" << directory<< ")";
    emit(startedLoading());
    //get a list of the contents of the directory and go through it.
    QDirIterator it(directory, nameFilters, QDir::Files | QDir::NoDotAndDotDot);
    while (it.hasNext()) {
        //If a flag was raised telling us to cancel the library scan then stop.
        if (*cancel) {
            return false;
        }
        m_pauseMutex.lock();
        if (*cancel) {
            return false;
        }
        addTrackToChunk(it.next());
        m_pauseMutex.unlock();
    }
    emit(finishedLoading());
    return true;
}

void LibraryScanner::addTrackToChunk(const QString filePath) {
    if (m_tracksListInCnunk.count() < MAX_CHUNK_SIZE) {
        m_tracksListInCnunk.append(filePath);
    } else {
        m_pTrackCollection->callSync( [this] (void) {
            addChunkToDatabase();
        }, "addTrackToChunk");
    }
}

void LibraryScanner::addChunkToDatabase() {
    DBG() << "Adding chunk to DB: " << m_tracksListInCnunk;
    m_trackDao.addTracksPrepare();
    foreach (m_tmpTrackPath, m_tracksListInCnunk) {
        // If the track is in the database, mark it as existing. This code gets exectuted
        // when other files in the same directory have changed (the directory hash has changed).
        m_trackDao.markTrackLocationAsVerified(m_tmpTrackPath);

        // If the file already exists in the database, continue and go on to
        // the next file.

        // If the file doesn't already exist in the database, then add
        // it. If it does exist in the database, then it is either in the
        // user's library OR the user has "removed" the track via
        // "Right-Click -> Remove". These tracks stay in the library, but
        // their mixxx_deleted column is 1.
        if (!m_trackDao.trackExistsInDatabase(m_tmpTrackPath)) {
            //qDebug() << "Loading" << it.fileName();
            TrackPointer pTrack = TrackPointer(new TrackInfoObject(m_tmpTrackPath), &QObject::deleteLater);
            if (m_trackDao.addTracksAdd(pTrack.data(), false)) {
                // Successful added
                // signal the main instance of TrackDao, that there is a
                // new Track in the database
                m_pTrackCollection->getTrackDAO().databaseTrackAdded(pTrack);
            } else {
                qDebug() << "Track ("+m_tmpTrackPath+") could not be added";
            }
        }
    }
    m_trackDao.addTracksFinish();
    m_tracksListInCnunk.clear();
}

// Recursively scan a music library. Doesn't import tracks for any directories thatmixxx
// have already been scanned and have not changed. Changes are tracked by performing
// a hash of the directory's file list, and those hashes are stored in the database.
bool LibraryScanner::recursiveScan(const QString& dirPath, QStringList& verifiedDirectories) {
    QDirIterator fileIt(dirPath, m_nameFilters, QDir::Files | QDir::NoDotAndDotDot);
    QString currentFile;
    bool bScanFinishedCleanly = true;
    //qDebug() << "Scanning dir:" << dirPath;
    QString newHashStr;
    bool prevHashExists = false;
    int newHash = -1;
    int prevHash = -1;
    // Note: A hash of "0" is a real hash if the directory contains no files!
    m_tracksListInCnunk.clear();

    while (fileIt.hasNext()) {
        currentFile = fileIt.next();
        //qDebug() << currentFile;
        newHashStr += currentFile;
    }

    // Calculate a hash of the directory's file list.
    newHash = qHash(newHashStr);

    // Try to retrieve a hash from the last time that directory was scanned.
    prevHash = m_libraryHashDao.getDirectoryHash(dirPath);
    prevHashExists = !(prevHash == -1);

    // Compare the hashes, and if they don't match, rescan the files in that directory!
    if (prevHash != newHash) {
        //If we didn't know about this directory before...
        if (!prevHashExists) {
            m_libraryHashDao.saveDirectoryHash(dirPath, newHash);
        } else {
            // Contents of a known directory have changed. Just need to update
            // the old hash in the database and then rescan it.
            qDebug() << "old hash was" << prevHash << "and new hash is" << newHash;
            m_libraryHashDao.updateDirectoryHash(dirPath, newHash, 0);
        }

        // Rescan that mofo!
        bScanFinishedCleanly = importDirectory(dirPath, m_nameFilters, &m_bCancelLibraryScan);
    } else { //prevHash == newHash
        // Add the directory to the verifiedDirectories list, so that later they
        // (and the tracks inside them) will be marked as verified
        emit(progressHashing(dirPath));
        verifiedDirectories.append(dirPath);
    }

    // Let us break out of library directory hashing (the actual file scanning
    // stuff is in TrackCollection::importDirectory)
    if (m_bCancelLibraryScan) {
        return false;
    }
    // Look at all the subdirectories and scan them recursively...
    QDirIterator dirIt(dirPath, QDir::Dirs | QDir::NoDotAndDotDot);
    while (dirIt.hasNext() && bScanFinishedCleanly) {
        QString nextPath = dirIt.next();
        //qDebug() << "nextPath: " << nextPath;

        // Skip the iTunes Album Art Folder since it is probably a waste of
        // time.
        if (m_directoriesBlacklist.contains(nextPath)) {
            continue;
        }
        if (!recursiveScan(nextPath, verifiedDirectories)) {
            bScanFinishedCleanly = false;
        }
    }
    return bScanFinishedCleanly;
}

// Table: LibraryHashes
// PRIMARY KEY string directory
// string hash

// Recursive Algorithm:
// 1) QDirIterator, iterate over all _files_ in a directory to construct a giant string.
// 2) newHash = Hash that string.
// 3) prevHash = SELECT from LibraryHashes * WHERE directory == strDirectory
// 4) if (prevHash != newHash) scanDirectory(strDirectory); //Do a NON-RECURSIVE scan of the files in that dir.
// 5) For each directory in strDirectory, execute this algorithm.
