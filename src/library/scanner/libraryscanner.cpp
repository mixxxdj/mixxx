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

#include <QtDebug>

#include "library/scanner/libraryscanner.h"

#include "sources/soundsourceproxy.h"
#include "library/scanner/recursivescandirectorytask.h"
#include "library/scanner/libraryscannerdlg.h"
#include "library/queryutil.h"
#include "library/coverartutils.h"
#include "library/trackcollection.h"
#include "util/trace.h"
#include "util/file.h"
#include "util/timer.h"
#include "library/scanner/scannerutil.h"

// TODO(rryan) make configurable
const int kScannerThreadPoolSize = 1;

LibraryScanner::LibraryScanner(TrackCollection* collection,
                               UserSettingsPointer pConfig)
              : m_pCollection(collection),
                m_libraryHashDao(m_database),
                m_cueDao(m_database),
                m_playlistDao(m_database),
                m_directoryDao(m_database),
                m_analysisDao(m_database, pConfig),
                m_trackDao(m_database,
                           m_cueDao, m_playlistDao,
                           m_analysisDao, m_libraryHashDao,
                           pConfig),
                m_stateSema(1), // only one transaction is possible at a time
                m_state(IDLE) {
    // Don't initialize m_database here, we need to do it in run() so the DB
    // conn is in the right thread.
    qDebug() << "Starting LibraryScanner thread.";

    // Move LibraryScanner to its own thread so that our signals/slots will
    // queue to our event loop.
    moveToThread(this);
    m_pool.moveToThread(this);

    unsigned static id = 0; // the id of this LibraryScanner, for debugging purposes
    setObjectName(QString("LibraryScanner %1").arg(++id));

    m_pool.setMaxThreadCount(kScannerThreadPoolSize);

    // Listen to signals from our public methods (invoked by other threads) and
    // connect them to our slots to run the command on the scanner thread.
    connect(this, SIGNAL(startScan()),
            this, SLOT(slotStartScan()));

    // Force the GUI thread's Track cache to be cleared when a library
    // scan is finished, because we might have modified the database directly
    // when we detected moved files, and the TIOs corresponding to the moved
    // files would then have the wrong track location.
    if (collection != NULL) { // false only during test
        TrackDAO* dao = &(collection->getTrackDAO());
        connect(this, SIGNAL(scanFinished()), dao, SLOT(clearCache()));
        connect(this, SIGNAL(trackAdded(TrackPointer)),
                dao, SLOT(databaseTrackAdded(TrackPointer)));
        connect(this, SIGNAL(tracksMoved(QSet<TrackId>, QSet<TrackId>)),
                dao, SLOT(databaseTracksMoved(QSet<TrackId>, QSet<TrackId>)));
        connect(this, SIGNAL(tracksChanged(QSet<TrackId>)),
                dao, SLOT(databaseTracksChanged(QSet<TrackId>)));
    }

    m_pProgressDlg.reset(new LibraryScannerDlg());
    connect(this, SIGNAL(progressLoading(QString)),
            m_pProgressDlg.data(), SLOT(slotUpdate(QString)));
    connect(this, SIGNAL(progressHashing(QString)),
            m_pProgressDlg.data(), SLOT(slotUpdate(QString)));
    connect(this, SIGNAL(scanStarted()),
            m_pProgressDlg.data(), SLOT(slotScanStarted()));
    connect(this, SIGNAL(scanFinished()),
            m_pProgressDlg.data(), SLOT(slotScanFinished()));
    connect(m_pProgressDlg.data(), SIGNAL(scanCancelled()),
            this, SLOT(slotCancel()));
    connect(&m_trackDao, SIGNAL(progressVerifyTracksOutside(QString)),
            m_pProgressDlg.data(), SLOT(slotUpdate(QString)));
    connect(&m_trackDao, SIGNAL(progressCoverArt(QString)),
            m_pProgressDlg.data(), SLOT(slotUpdateCover(QString)));

    start();
}

LibraryScanner::~LibraryScanner() {
    cancelAndQuit();

    // There should never be an outstanding transaction when this code is
    // called. If there is, it means we probably aren't committing a transaction
    // somewhere that should be.
    if (m_database.isOpen()) {
        qDebug() << "Closing database" << m_database.connectionName();

        // Rollback any uncommitted transaction
        if (m_database.rollback()) {
            qDebug() << "ERROR: There was a transaction in progress while closing the library scanner connection."
                     << "There is a logic error somewhere.";
        }
        // Close our database connection
        m_database.close();
    }
    qDebug() << "LibraryScanner destroyed";
}

void LibraryScanner::run() {
    Trace trace("LibraryScanner");
    if (m_pCollection != NULL) { // false only during tests
        if (!m_database.isValid()) {
            m_database = QSqlDatabase::cloneDatabase(m_pCollection->database(), "LIBRARY_SCANNER");
        }

        if (!m_database.isOpen()) {
            // Open the database connection in this thread.
            if (!m_database.open()) {
                qDebug() << "Failed to open database from library scanner thread." << m_database.lastError();
                return;
            }
        }

        m_libraryHashDao.setDatabase(m_database);
        m_cueDao.setDatabase(m_database);
        m_trackDao.setDatabase(m_database);
        m_playlistDao.setDatabase(m_database);
        m_analysisDao.setDatabase(m_database);
        m_directoryDao.setDatabase(m_database);

        m_libraryHashDao.initialize();
        m_cueDao.initialize();
        m_trackDao.initialize();
        m_playlistDao.initialize();
        m_analysisDao.initialize();
        m_directoryDao.initialize();
    }

    // Start the event loop.
    qDebug() << "LibraryScanner event loop starting.";
    exec();
    qDebug() << "LibraryScanner event loop stopped.";
}

void LibraryScanner::slotStartScan() {
    qDebug() << "LibraryScanner::slotStartScan";
    DEBUG_ASSERT(m_state == STARTING);

    // Recursively scan each directory in the directories table.
    m_libraryRootDirs = m_directoryDao.getDirs();
    // If there are no directories then we have nothing to do. Cleanup and
    // finish the scan immediately.
    if (m_libraryRootDirs.isEmpty()) {
        changeScannerState(IDLE);
        return;
    }
    changeScannerState(SCANNING);

    QSet<QString> trackLocations = m_trackDao.getTrackLocations();
    QHash<QString, int> directoryHashes = m_libraryHashDao.getDirectoryHashes();
    QRegExp extensionFilter(SoundSourceProxy::getSupportedFileNamesRegex());
    QRegExp coverExtensionFilter =
            QRegExp(CoverArtUtils::supportedCoverArtExtensionsRegex(),
                    Qt::CaseInsensitive);
    QStringList directoryBlacklist = ScannerUtil::getDirectoryBlacklist();

    m_scannerGlobal = ScannerGlobalPointer(
            new ScannerGlobal(trackLocations, directoryHashes, extensionFilter,
                              coverExtensionFilter, directoryBlacklist));

    m_scannerGlobal->startTimer();

    emit(scanStarted());

    // First, we're going to mark all the directories that we've previously
    // hashed as needing verification. As we search through the directory tree
    // when we rescan, we'll mark any directory that does still exist as
    // verified.
    m_libraryHashDao.invalidateAllDirectories();

    // Mark all the tracks in the library as needing verification of their
    // existence. (ie. we want to check they're still on your hard drive where
    // we think they are)
    m_trackDao.invalidateTrackLocationsInLibrary();

    qDebug() << "Recursively scanning library.";

    // Start scanning the library. This prepares insertion queries in TrackDAO
    // (must be called before calling addTracksAdd) and begins a transaction.
    m_trackDao.addTracksPrepare();

    // First Scan all known directories we have a hash for.
    // In a second stage, we scan all new directories. This guarantees,
    // that we discover always the same folder, in case of duplicated folders
    // by symlinks

    // Queue up recursive scan tasks for every hashed directory. When all tasks
    // are done, TaskWatcher will signal slotFinishHashedScan.
    TaskWatcher* pWatcher = &m_scannerGlobal->getTaskWatcher();
    pWatcher->watchTask();
    connect(pWatcher, SIGNAL(allTasksDone()),
            this, SLOT(slotFinishHashedScan()));

    foreach (const QString& dirPath, m_libraryRootDirs) {
        // Acquire a security bookmark for this directory if we are in a
        // sandbox. For speed we avoid opening security bookmarks when recursive
        // scanning so that relies on having an open bookmark for the containing
        // directory.
        MDir dir(dirPath);
        if (!m_scannerGlobal->testAndMarkDirectoryScanned(dir.dir())) {
            queueTask(new RecursiveScanDirectoryTask(this, m_scannerGlobal,
                                                     dir.dir(),
                                                     dir.token(),
                                                     false));
        }
    }
    pWatcher->taskDone();
}

// is called when all tasks of the first stage are done (threads are finished)
void LibraryScanner::slotFinishHashedScan() {
    qDebug() << "LibraryScanner::slotFinishHashedScan";
    VERIFY_OR_DEBUG_ASSERT(!m_scannerGlobal.isNull()) {
        qWarning() << "No scanner global state exists in LibraryScanner::slotFinishHashedScan";
        return;
    }

    TaskWatcher* pWatcher = &m_scannerGlobal->getTaskWatcher();
    disconnect(pWatcher, SIGNAL(allTasksDone()),
            this, SLOT(slotFinishHashedScan()));

    if (m_scannerGlobal->unhashedDirs().empty()) {
        // bypass the second stage
        slotFinishUnhashedScan();
        return;
    }

    // Queue up recursive scan tasks for every unhashed directory, discovered
    // in the first stage. When all tasks
    // are done, TaskWatcher will signal slotFinishUnhashedScan.
    pWatcher->watchTask();
    connect(pWatcher, SIGNAL(allTasksDone()),
            this, SLOT(slotFinishUnhashedScan()));

    foreach (const DirInfo& dirInfo, m_scannerGlobal->unhashedDirs()) {
        // no testAndMarkDirectoryScanned() here, because all unhashedDirs()
        // are already tracked
        queueTask(new RecursiveScanDirectoryTask(this, m_scannerGlobal,
                                                 dirInfo.dir(),
                                                 dirInfo.token(),
                                                 true));
    }
    pWatcher->taskDone();
}

void LibraryScanner::cleanUpScan() {
    // At the end of a scan, mark all tracks and directories that weren't
    // "verified" as "deleted" (as long as the scan wasn't canceled half way
    // through). This condition is important because our rescanning algorithm
    // starts by marking all tracks and dirs as unverified, so a canceled scan
    // might leave half of your library as unverified. Don't want to mark those
    // tracks/dirs as deleted in that case) :)

    // Start a transaction for all the library hashing (moved file
    // detection) stuff.
    ScopedTransaction transaction(m_database);

    qDebug() << "Marking tracks in changed directories as verified";
    m_trackDao.markTrackLocationsAsVerified(m_scannerGlobal->verifiedTracks());

    qDebug() << "Marking unchanged directories and tracks as verified";
    m_libraryHashDao.updateDirectoryStatuses(
            m_scannerGlobal->verifiedDirectories(),
            false,
            true);
    m_trackDao.markTracksInDirectoriesAsVerified(
            m_scannerGlobal->verifiedDirectories());

    // After verifying tracks and directories via recursive scanning of the
    // library directories the only unverified tracks will be files that are
    // outside of the library directories, files that have been
    // moved/deleted/renamed and are in duplicate directories by symlinks or
    // non normalized paths.
    qDebug() << "Checking remaining unverified tracks.";
    if (!m_trackDao.verifyRemainingTracks(
            m_libraryRootDirs,
            m_scannerGlobal->shouldCancelPointer())) {
        // canceled
        return;
    }

    qDebug() << "Marking unverified tracks as deleted.";
    m_trackDao.markUnverifiedTracksAsDeleted();

    qDebug() << "Marking unverified directories as deleted.";
    m_libraryHashDao.markUnverifiedDirectoriesAsDeleted();

    // For making the scanner slow, during debugging
    //qDebug() << "Burn CPU";
    //for (int i = 0;i < 1000000000; i++) asm("nop");

    // Check to see if the "deleted" tracks showed up in another location,
    // and if so, do some magic to update all our tables.
    qDebug() << "Detecting moved files.";
    QSet<TrackId> tracksMovedSetOld;
    QSet<TrackId> tracksMovedSetNew;
    if (!m_trackDao.detectMovedTracks(&tracksMovedSetOld,
            &tracksMovedSetNew,
            m_scannerGlobal->addedTracks(),
            m_scannerGlobal->shouldCancelPointer())) {
        // canceled
        return;
    }

    // Remove the hashes for any directories that have been marked as
    // deleted to clean up. We need to do this otherwise we can skip over
    // songs if you move a set of songs from directory A to B, then back to
    // A.
    m_libraryHashDao.removeDeletedDirectoryHashes();

    transaction.commit();

    qDebug() << "Detecting cover art for unscanned files.";
    QSet<TrackId> coverArtTracksChanged;
    m_trackDao.detectCoverArtForTracksWithoutCover(
            m_scannerGlobal->shouldCancelPointer(), &coverArtTracksChanged);

    // Update BaseTrackCache via signals connected to the main TrackDAO.
    emit(tracksMoved(tracksMovedSetOld, tracksMovedSetNew));
    emit(tracksChanged(coverArtTracksChanged));
}


// is called when all tasks of the second stage are done (threads are finished)
void LibraryScanner::slotFinishUnhashedScan() {
    qDebug() << "LibraryScanner::slotFinishUnhashedScan";
    VERIFY_OR_DEBUG_ASSERT(!m_scannerGlobal.isNull()) {
        qWarning() << "No scanner global state exists in LibraryScanner::slotFinishUnhashedScan";
        return;
    }

    bool bScanFinishedCleanly = m_scannerGlobal->scanFinishedCleanly();

    if (bScanFinishedCleanly) {
        qDebug() << "Recursive scanning finished cleanly.";
    } else {
        qDebug() << "Recursive scanning interrupted by the user.";
    }

    // Finish adding the tracks -- rollback the transaction if the scan did not
    // finish cleanly and the user did not cancel the transaction.
    m_trackDao.addTracksFinish(!m_scannerGlobal->shouldCancel() &&
                               !bScanFinishedCleanly);

    if (!m_scannerGlobal->shouldCancel() && bScanFinishedCleanly) {
        cleanUpScan();
    }

    if (!m_scannerGlobal->shouldCancel() && bScanFinishedCleanly) {
        qDebug() << "Scan finished cleanly";
    } else {
        qDebug() << "Scan cancelled";
    }

    // TODO(XXX) doesn't take into account verifyRemainingTracks.
    qDebug("Scan took: %s. "
           "%d unchanged directories. "
           "%d changed/added directories. "
           "%d tracks verified from changed/added directories. "
           "%d new tracks.",
           m_scannerGlobal->timerElapsed().formatNanosWithUnit().toLocal8Bit().constData(),
           m_scannerGlobal->verifiedDirectories().size(),
           m_scannerGlobal->numScannedDirectories(),
           m_scannerGlobal->verifiedTracks().size(),
           m_scannerGlobal->addedTracks().size());

    m_scannerGlobal.clear();
    changeScannerState(FINISHED);
    // now we may accept new scan commands

    emit(scanFinished());
}

void LibraryScanner::scan() {
    if (changeScannerState(STARTING)) {
        emit(startScan());
    }
}

// this is called after pressing the cancel button in the scanner
// progress dialog
void LibraryScanner::slotCancel() {
    // Wait until there is no scan starting.
    // All pending scan start request are canceled
    // as well until the scanner is idle again.
    changeScannerState(CANCELING);
    cancel();
    changeScannerState(IDLE);
}

void LibraryScanner::cancelAndQuit() {
    changeScannerState(CANCELING);
    cancel();
    // Quit the event loop gracefully and stay in CANCELING state until all
    // pendig signals are processed
    quit();
    wait();
    changeScannerState(IDLE);
}

// be sure we hold the m_stateSema and we are in CANCELING state
void LibraryScanner::cancel() {
    DEBUG_ASSERT(m_state == CANCELING);


    // we need to make a local copy because cancel is called
    // from any thread  but m_scannerGlobal may be cleared
    // in the LibraryScanner thread in the meanwhile
    ScannerGlobalPointer scanner = m_scannerGlobal;
    if (scanner) {
        scanner->cancel();
    }

    // Wait for the thread pool to empty. This is important because ScannerTasks
    // have pointers to the LibraryScanner and can cause a segfault if they run
    // after the LibraryScanner has been destroyed.
    m_pool.waitForDone();
}

void LibraryScanner::queueTask(ScannerTask* pTask) {
    //qDebug() << "LibraryScanner::queueTask" << pTask;
    ScopedTimer timer("LibraryScanner::queueTask");
    if (m_scannerGlobal.isNull() || m_scannerGlobal->shouldCancel()) {
        return;
    }
    m_scannerGlobal->getTaskWatcher().watchTask();
    connect(pTask, SIGNAL(queueTask(ScannerTask*)),
            this, SLOT(queueTask(ScannerTask*)));
    connect(pTask, SIGNAL(directoryHashedAndScanned(QString, bool, int)),
            this, SLOT(slotDirectoryHashedAndScanned(QString, bool, int)));
    connect(pTask, SIGNAL(directoryUnchanged(QString)),
            this, SLOT(slotDirectoryUnchanged(QString)));
    connect(pTask, SIGNAL(trackExists(QString)),
            this, SLOT(slotTrackExists(QString)));
    connect(pTask, SIGNAL(addNewTrack(QString)),
            this, SLOT(slotAddNewTrack(QString)));

    // Progress signals.
    // Pass directly to the main thread
    connect(pTask, SIGNAL(progressLoading(QString)),
            this, SIGNAL(progressLoading(QString)));
    connect(pTask, SIGNAL(progressHashing(QString)),
            this, SIGNAL(progressHashing(QString)));

    m_pool.start(pTask);
}

void LibraryScanner::slotDirectoryHashedAndScanned(const QString& directoryPath,
                                               bool newDirectory, int hash) {
    ScopedTimer timer("LibraryScanner::slotDirectoryHashedAndScanned");
    //qDebug() << "LibraryScanner::sloDirectoryHashedAndScanned" << directoryPath
    //          << newDirectory << hash;

    // For statistics tracking -- if we hashed a directory then we scanned it
    // (it was changed or new).
    if (m_scannerGlobal) {
        m_scannerGlobal->directoryScanned();
    }

    if (newDirectory) {
        m_libraryHashDao.saveDirectoryHash(directoryPath, hash);
    } else {
        m_libraryHashDao.updateDirectoryHash(directoryPath, hash, 0);
    }
    emit(progressHashing(directoryPath));
}

void LibraryScanner::slotDirectoryUnchanged(const QString& directoryPath) {
    ScopedTimer timer("LibraryScanner::slotDirectoryUnchanged");
    //qDebug() << "LibraryScanner::slotDirectoryUnchanged" << directoryPath;
    if (m_scannerGlobal) {
        m_scannerGlobal->addVerifiedDirectory(directoryPath);
    }
    emit(progressHashing(directoryPath));
}

void LibraryScanner::slotTrackExists(const QString& trackPath) {
    //qDebug() << "LibraryScanner::slotTrackExists" << trackPath;
    ScopedTimer timer("LibraryScanner::slotTrackExists");
    if (m_scannerGlobal) {
        m_scannerGlobal->addVerifiedTrack(trackPath);
    }
}

void LibraryScanner::slotAddNewTrack(const QString& trackPath) {
    //qDebug() << "LibraryScanner::slotAddNewTrack" << trackPath;
    ScopedTimer timer("LibraryScanner::addNewTrack");
    // For statistics tracking and to detect moved tracks
    TrackPointer pTrack(m_trackDao.addTracksAddFile(trackPath, false));
    if (pTrack) {
        // The track's actual location might differ from the
        // given trackPath
        const QString trackLocation(pTrack->getLocation());
        // Acknowledge successful track addition
        if (m_scannerGlobal) {
            m_scannerGlobal->trackAdded(trackLocation);
        }
        // Signal the main instance of TrackDAO, that there is
        // a new track in the database.
        emit(trackAdded(pTrack));
        emit(progressLoading(trackLocation));
    } else {
        // Acknowledge failed track addition
        // TODO(XXX): Is it really intended to acknowledge a failed
        // track addition with a trackAdded() signal??
        if (m_scannerGlobal) {
            m_scannerGlobal->trackAdded(trackPath);
        }
        qWarning()
                << "Failed to add track to library:"
                << trackPath;
    }
}

bool LibraryScanner::changeScannerState(ScannerState newState) {
    switch (newState) {
    case IDLE:
        // we are leaving STARTING  or CANCELING state
        // m_state is already IDLE if a scan was canceled
        m_state = IDLE;
        m_stateSema.release();
        return true;
    case STARTING:
        // we need to hold the m_stateSema during the STARTING state
        // to prevent loosing cancel commands or start the scanner
        // twice
        if (m_stateSema.tryAcquire()) {
            if (m_state != IDLE) {
                qDebug() << "LibraryScanner: Scan already in progress.";
                m_stateSema.release();
                return false;
            }
            m_state = STARTING;
            return true;
        } else {
            qDebug() << "LibraryScanner: can't acquire semaphore, state =" << m_state;
            return false;
        }
    case SCANNING:
        DEBUG_ASSERT(m_state == STARTING);
        // Transition protected by the semaphore is over now
        // Allow canceling
        m_state = SCANNING;
        m_stateSema.release();
        return true;
    case CANCELING:
        DEBUG_ASSERT(m_state != CANCELING);
        // canceling is always possible, but wait
        // until there is no scan starting.
        // It must be unlocked by changeScannerState(IDLE);
        m_stateSema.acquire();
        m_state = CANCELING;
        return true;
    case FINISHED:
        // we must not acquire the semaphore here, because
        // it is already acquired in case we
        // are canceling.
        // There is no race condition, since the state
        // is set to IDLE after canceling as well
        m_state = IDLE;
        return true;
    default:
        DEBUG_ASSERT(false);
        return false;
    }
}
