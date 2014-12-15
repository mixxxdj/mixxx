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

#include "library/libraryscanner.h"

#include "soundsourceproxy.h"
#include "library/legacylibraryimporter.h"
#include "library/scanner/recursivescandirectorytask.h"
#include "libraryscannerdlg.h"
#include "library/queryutil.h"
#include "library/coverartutils.h"
#include "library/trackcollection.h"
#include "util/trace.h"
#include "util/file.h"
#include "util/timer.h"
#include "library/scanner/scannerutil.h"

// TODO(rryan) make configurable
const int kScannerThreadPoolSize = 1;

LibraryScanner::LibraryScanner(QWidget* pParentWidget, TrackCollection* collection)
              : m_pCollection(collection),
                m_libraryHashDao(m_database),
                m_cueDao(m_database),
                m_playlistDao(m_database),
                m_crateDao(m_database),
                m_directoryDao(m_database),
                m_analysisDao(m_database, collection->getConfig()),
                m_trackDao(m_database, m_cueDao, m_playlistDao,
                           m_crateDao, m_analysisDao, m_libraryHashDao,
                           collection->getConfig()) {
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

    // Force the GUI thread's TrackInfoObject cache to be cleared when a library
    // scan is finished, because we might have modified the database directly
    // when we detected moved files, and the TIOs corresponding to the moved
    // files would then have the wrong track location.
    connect(this, SIGNAL(scanFinished()),
            &(collection->getTrackDAO()), SLOT(clearCache()));
    connect(this, SIGNAL(trackAdded(TrackPointer)),
            &(collection->getTrackDAO()), SLOT(databaseTrackAdded(TrackPointer)));
    connect(this, SIGNAL(tracksMoved(QSet<int>, QSet<int>)),
            &(collection->getTrackDAO()), SLOT(databaseTracksMoved(QSet<int>, QSet<int>)));
    connect(this, SIGNAL(tracksChanged(QSet<int>)),
            &(collection->getTrackDAO()), SLOT(databaseTracksChanged(QSet<int>)));

    // Parented to pParentWidget so we don't need to delete it.
    LibraryScannerDlg* pProgress = new LibraryScannerDlg(pParentWidget);
    connect(this, SIGNAL(progressLoading(QString)),
            pProgress, SLOT(slotUpdate(QString)));
    connect(this, SIGNAL(progressHashing(QString)),
            pProgress, SLOT(slotUpdate(QString)));
    connect(this, SIGNAL(scanStarted()),
            pProgress, SLOT(slotScanStarted()));
    connect(this, SIGNAL(scanFinished()),
            pProgress, SLOT(slotScanFinished()));
    connect(pProgress, SIGNAL(scanCancelled()),
            this, SLOT(cancel()));
    connect(&m_trackDao, SIGNAL(progressVerifyTracksOutside(QString)),
            pProgress, SLOT(slotUpdate(QString)));
    connect(&m_trackDao, SIGNAL(progressCoverArt(QString)),
            pProgress, SLOT(slotUpdateCover(QString)));

    start();
}

LibraryScanner::~LibraryScanner() {
    // A scan is running.
    if (m_scannerGlobal) {
        // Cancel any running library scan.
        cancel();

        // Wait for the thread pool to empty. This is important because
        // ScannerTasks have pointers to the LibraryScanner and can cause a
        // segfault if they run after the LibraryScanner has been destroyed.
        m_pool.waitForDone();

        // Quit the event loop gracefully.
        quit();

        // Wait for thread to finish
        wait();
    }

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

    if (!m_database.isValid()) {
        m_database = QSqlDatabase::cloneDatabase(m_pCollection->getDatabase(), "LIBRARY_SCANNER");
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

    // Start the event loop.
    qDebug() << "LibraryScanner event loop starting.";
    exec();
    qDebug() << "LibraryScanner event loop stopped.";
}

void LibraryScanner::slotStartScan() {
    qDebug() << "LibraryScanner::slotStartScan";
    QSet<QString> trackLocations = m_trackDao.getTrackLocations();
    QHash<QString, int> directoryHashes = m_libraryHashDao.getDirectoryHashes();
    QRegExp extensionFilter =
            QRegExp(SoundSourceProxy::supportedFileExtensionsRegex(),
                    Qt::CaseInsensitive);
    QRegExp coverExtensionFilter =
            QRegExp(CoverArtUtils::supportedCoverArtExtensionsRegex(),
                    Qt::CaseInsensitive);
    QStringList directoryBlacklist = ScannerUtil::getDirectoryBlacklist();

    m_scannerGlobal = ScannerGlobalPointer(
        new ScannerGlobal(trackLocations, directoryHashes, extensionFilter,
                          coverExtensionFilter, directoryBlacklist));
    m_scannerGlobal->startTimer();

    emit(scanStarted());

    // Try to upgrade the library from 1.7 (XML) to 1.8+ (DB) if needed. If the
    // upgrade_filename already exists, then do not try to upgrade since we have
    // already done it.
    // TODO(XXX) SETTINGS_PATH may change in new Mixxx Versions. Here we need
    // the SETTINGS_PATH from Mixxx V <= 1.7
    QString upgrade_filename = QDir::homePath().append("/").append(SETTINGS_PATH).append("DBUPGRADED");
    qDebug() << "upgrade filename is " << upgrade_filename;
    QFile upgradefile(upgrade_filename);
    if (!upgradefile.exists()) {
        QTime t2;
        t2.start();
        LegacyLibraryImporter libImport(m_trackDao, m_playlistDao);
        connect(&libImport, SIGNAL(progress(QString)),
                this, SIGNAL(progressLoading(QString)));
        ScopedTransaction transaction(m_database);
        libImport.import();
        transaction.commit();
        qDebug("Legacy importer took %d ms", t2.elapsed());
    }

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

    // Recursivly scan each directory in the directories table.
    QStringList dirs = m_directoryDao.getDirs();

    // If there are no directories then we have nothing to do. Cleanup and
    // finish the scan immediately.
    if (dirs.isEmpty()) {
        slotFinishScan();
        return;
    }

    // Queue up recursive scan tasks for every directory. When all tasks are
    // done, TaskWatcher will signal slotFinishScan.
    TaskWatcher* pWatcher = &m_scannerGlobal->getTaskWatcher();
    connect(pWatcher, SIGNAL(allTasksDone()),
            this, SLOT(slotFinishScan()));

    foreach (const QString& dirPath, dirs) {
        // Acquire a security bookmark for this directory if we are in a
        // sandbox. For speed we avoid opening security bookmarks when recursive
        // scanning so that relies on having an open bookmark for the containing
        // directory.
        MDir dir(dirPath);

        queueTask(new RecursiveScanDirectoryTask(this, m_scannerGlobal, dir.dir(),
                                                 dir.token()));
    }
}

void LibraryScanner::slotFinishScan() {
    qDebug() << "LibraryScanner::slotFinishScan";
    if (m_scannerGlobal.isNull()) {
        qWarning() << "No scanner global state exists in LibraryScanner::slotFinishScan";
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

    QStringList verifiedTracks = m_scannerGlobal->verifiedTracks();
    QStringList verifiedDirectories = m_scannerGlobal->verifiedDirectories();

    // At the end of a scan, mark all tracks and directories that weren't
    // "verified" as "deleted" (as long as the scan wasn't canceled half way
    // through). This condition is important because our rescanning algorithm
    // starts by marking all tracks and dirs as unverified, so a canceled scan
    // might leave half of your library as unverified. Don't want to mark those
    // tracks/dirs as deleted in that case) :)
    if (bScanFinishedCleanly) {
        QSet<int> tracksMovedSetOld;
        QSet<int> tracksMovedSetNew;
        QSet<int> coverArtTracksChanged;

        // Start a transaction for all the library hashing (moved file
        // detection) stuff.
        ScopedTransaction transaction(m_database);

        qDebug() << "Marking tracks in changed directories as verified";
        m_trackDao.markTrackLocationsAsVerified(verifiedTracks);

        qDebug() << "Marking unchanged directories and tracks as verified";
        m_libraryHashDao.updateDirectoryStatuses(verifiedDirectories, false, true);
        m_trackDao.markTracksInDirectoriesAsVerified(verifiedDirectories);

        // After verifying tracks and directories via recursive scanning of the
        // library directories the only unverified tracks will be files that are
        // outside of the library directories and files that have been
        // moved/deleted/renamed.
        qDebug() << "Checking remaining unverified tracks.";
        m_trackDao.verifyRemainingTracks();

        qDebug() << "Marking unverified tracks as deleted.";
        m_trackDao.markUnverifiedTracksAsDeleted();

        qDebug() << "Marking unverified directories as deleted.";
        m_libraryHashDao.markUnverifiedDirectoriesAsDeleted();

        // Check to see if the "deleted" tracks showed up in another location,
        // and if so, do some magic to update all our tables.
        qDebug() << "Detecting moved files.";
        m_trackDao.detectMovedFiles(&tracksMovedSetOld, &tracksMovedSetNew);

        // Remove the hashes for any directories that have been marked as
        // deleted to clean up. We need to do this otherwise we can skip over
        // songs if you move a set of songs from directory A to B, then back to
        // A.
        m_libraryHashDao.removeDeletedDirectoryHashes();

        transaction.commit();

        qDebug() << "Detecting cover art for unscanned files.";
        m_trackDao.detectCoverArtForUnknownTracks(
            m_scannerGlobal->shouldCancelPointer(), &coverArtTracksChanged);

        // Update BaseTrackCache via signals connected to the main TrackDAO.
        emit(tracksMoved(tracksMovedSetOld, tracksMovedSetNew));
        emit(tracksChanged(coverArtTracksChanged));

        qDebug() << "Scan finished cleanly";
    } else {
        qDebug() << "Scan cancelled";
    }

    // TODO(XXX) doesn't take into account verifyRemainingTracks.
    qDebug("Scan took: %lld ns. "
           "%d unchanged directories. "
           "%d changed/added directories. "
           "%d tracks verified from changed/added directories. "
           "%d new tracks.",
           m_scannerGlobal->timerElapsed(),
           verifiedDirectories.size(),
           m_scannerGlobal->numScannedDirectories(),
           verifiedTracks.size(),
           m_scannerGlobal->numAddedTracks());

    emit(scanFinished());
    m_scannerGlobal.clear();
}

void LibraryScanner::scan() {
    if (m_scannerGlobal) {
        qDebug() << "Scan already in progress.";
        return;
    }
    emit(startScan());
}

void LibraryScanner::cancel() {
    if (m_scannerGlobal) {
        m_scannerGlobal->setShouldCancel(true);
    }
}

void LibraryScanner::taskDone(bool success) {
    //qDebug() << "LibraryScanner::taskDone" << success;
    ScopedTimer timer("LibraryScanner::taskDone");
    if (!success && m_scannerGlobal) {
        m_scannerGlobal->setScanFinishedCleanly(false);
    }
}

void LibraryScanner::queueTask(ScannerTask* pTask) {
    //qDebug() << "LibraryScanner::queueTask" << pTask;
    ScopedTimer timer("LibraryScanner::queueTask");
    if (m_scannerGlobal.isNull() || m_scannerGlobal->shouldCancel()) {
        return;
    }
    m_scannerGlobal->getTaskWatcher().watchTask(pTask, SIGNAL(taskDone(bool)));
    connect(pTask, SIGNAL(taskDone(bool)),
            this, SLOT(taskDone(bool)));
    connect(pTask, SIGNAL(queueTask(ScannerTask*)),
            this, SLOT(queueTask(ScannerTask*)));
    connect(pTask, SIGNAL(directoryHashed(QString, bool, int)),
            this, SLOT(directoryHashed(QString, bool, int)));
    connect(pTask, SIGNAL(directoryUnchanged(QString)),
            this, SLOT(directoryUnchanged(QString)));
    connect(pTask, SIGNAL(trackExists(QString)),
            this, SLOT(trackExists(QString)));
    connect(pTask, SIGNAL(addNewTrack(TrackPointer)),
            this, SLOT(addNewTrack(TrackPointer)));

    // Progress signals.
    connect(pTask, SIGNAL(progressLoading(QString)),
            this, SIGNAL(progressLoading(QString)));
    connect(pTask, SIGNAL(progressHashing(QString)),
            this, SIGNAL(progressHashing(QString)));

    m_pool.start(pTask);
}

void LibraryScanner::directoryHashed(const QString& directoryPath,
                                     bool newDirectory, int hash) {
    ScopedTimer timer("LibraryScanner::directoryHashed");
    // qDebug() << "LibraryScanner::directoryHashed" << directoryPath
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

void LibraryScanner::directoryUnchanged(const QString& directoryPath) {
    ScopedTimer timer("LibraryScanner::directoryUnchanged");
    //qDebug() << "LibraryScanner::directoryUnchanged" << directoryPath;
    if (m_scannerGlobal) {
        m_scannerGlobal->addVerifiedDirectory(directoryPath);
    }
    emit(progressHashing(directoryPath));
}

void LibraryScanner::trackExists(const QString& trackPath) {
    //qDebug() << "LibraryScanner::trackExists" << trackPath;
    ScopedTimer timer("LibraryScanner::trackExists");
    if (m_scannerGlobal) {
        m_scannerGlobal->addVerifiedTrack(trackPath);
    }
}

void LibraryScanner::addNewTrack(TrackPointer pTrack) {
    //qDebug() << "LibraryScanner::addNewTrack" << pTrack;
    ScopedTimer timer("LibraryScanner::addNewTrack");
    // For statistics tracking.
    if (m_scannerGlobal) {
        m_scannerGlobal->trackAdded();
    }
    if (m_trackDao.addTracksAdd(pTrack.data(), false)) {
        // Successfully added. Signal the main instance of TrackDAO,
        // that there is a new track in the database.
        emit(trackAdded(pTrack));
        emit(progressLoading(pTrack->getLocation()));
    } else {
        qWarning() << "Track ("+pTrack->getLocation()+") could not be added";
    }
}
