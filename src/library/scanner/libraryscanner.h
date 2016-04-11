/***************************************************************************
                          libraryscanner.h  -  scans library in a thread
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

#ifndef LIBRARYSCANNER_H
#define LIBRARYSCANNER_H

#include <QThread>
#include <QThreadPool>
#include <QList>
#include <QString>
#include <QList>
#include <QObject>
#include <QSqlDatabase>
#include <QStringList>
#include <QRegExp>
#include <QFileInfo>
#include <QLinkedList>
#include <QSemaphore>
#include <QScopedPointer>

#include "library/dao/cratedao.h"
#include "library/dao/cuedao.h"
#include "library/dao/libraryhashdao.h"
#include "library/dao/directorydao.h"
#include "library/dao/playlistdao.h"
#include "library/dao/trackdao.h"
#include "library/dao/analysisdao.h"
#include "library/scanner/scannerglobal.h"
#include "library/scanner/scannertask.h"
#include "util/sandbox.h"
#include "trackinfoobject.h"
#include <gtest/gtest.h>

class LibraryScannerDlg;
class TrackCollection;

class LibraryScanner : public QThread {
    FRIEND_TEST(LibraryScannerTest, ScannerRoundtrip);
    Q_OBJECT
  public:
    LibraryScanner(TrackCollection* collection,
                   UserSettingsPointer pConfig);
    virtual ~LibraryScanner();

  public slots:
    // Call from any thread to start a scan. Does nothing if a scan is already
    // in progress.
    void scan();

    // Call from any thread to cancel the scan.
    void slotCancel();

  signals:
    void scanStarted();
    void scanFinished();
    void progressHashing(QString);
    void progressLoading(QString path);
    void progressCoverArt(QString file);
    void trackAdded(TrackPointer pTrack);
    void tracksMoved(QSet<TrackId> oldTrackIds, QSet<TrackId> newTrackIds);
    void tracksChanged(QSet<TrackId> changedTrackIds);

    // Emitted by scan() to invoke slotStartScan in the scanner thread's event
    // loop.
    void startScan();

  protected:
    void run();

  public slots:
    void queueTask(ScannerTask* pTask);

  private slots:
    void slotStartScan();
    void slotFinishHashedScan();
    void slotFinishUnhashedScan();

    // ScannerTask signal handlers.
    void slotDirectoryHashedAndScanned(const QString& directoryPath,
                                   bool newDirectory, int hash);
    void slotDirectoryUnchanged(const QString& directoryPath);
    void slotTrackExists(const QString& trackPath);
    void slotAddNewTrack(const QString& trackPath);

  private:
    enum ScannerState {
        IDLE,
        STARTING,
        SCANNING,
        CANCELING,
        FINISHED
    };

    void cancelAndQuit();
    void cancel();

    // Allowed State transitions:
    // IDLE -> STARTING
    // STARTING -> IDLE
    // STARTING -> SCANNING
    // SCANNING -> FINISHED
    // FINISHED -> IDLE
    // every state can change to CANCELING
    // CANCELING -> IDLE
    bool changeScannerState(LibraryScanner::ScannerState newState);

    void cleanUpScan();

    // The library trackcollection. Do not touch this from the library scanner
    // thread.
    TrackCollection* m_pCollection;

    // The library scanner thread's database connection.
    QSqlDatabase m_database;

    // The pool of threads used for worker tasks.
    QThreadPool m_pool;

    // The library scanner thread's DAOs.
    LibraryHashDAO m_libraryHashDao;
    CueDAO m_cueDao;
    PlaylistDAO m_playlistDao;
    CrateDAO m_crateDao;
    DirectoryDAO m_directoryDao;
    AnalysisDao m_analysisDao;
    TrackDAO m_trackDao;

    // Global scanner state for scan currently in progress.
    ScannerGlobalPointer m_scannerGlobal;

    // The Semaphore guards the state transitions queued to the
    // Qt even Queue in the way, that you cannot start a
    // new scan while the old one is canceled
    QSemaphore m_stateSema;
    // this is accessed main and LibraryScanner thread
    volatile ScannerState m_state;

    QStringList m_libraryRootDirs;
    QScopedPointer<LibraryScannerDlg> m_pProgressDlg;
};

#endif
