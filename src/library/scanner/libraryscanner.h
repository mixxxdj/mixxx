#pragma once

#include <gtest/gtest_prod.h>

#include <QList>
#include <QScopedPointer>
#include <QSemaphore>
#include <QString>
#include <QThread>
#include <QThreadPool>

#include "library/dao/analysisdao.h"
#include "library/dao/cuedao.h"
#include "library/dao/directorydao.h"
#include "library/dao/libraryhashdao.h"
#include "library/dao/playlistdao.h"
#include "library/dao/trackdao.h"
#include "library/scanner/scannerglobal.h"
#include "track/track_decl.h"
#include "track/trackid.h"
#include "util/db/dbconnectionpool.h"

class ScannerTask;
class LibraryScannerDlg;

class LibraryScanner : public QThread {
    FRIEND_TEST(LibraryScannerTest, ScannerRoundtrip);
    Q_OBJECT
  public:
    LibraryScanner(
            mixxx::DbConnectionPoolPtr pDbConnectionPool,
            const UserSettingsPointer& pConfig);
    ~LibraryScanner() override;

  public slots:
    // Call from any thread to start a scan. Does nothing if a scan is already
    // in progress.
    void scan();

    // Call from any thread to cancel the scan.
    void slotCancel();

  signals:
    void scanStarted();
    void scanFinished();
    void progressHashing(const QString&);
    void progressLoading(const QString& path);
    void progressCoverArt(const QString& file);
    void trackAdded(TrackPointer pTrack);
    void tracksChanged(const QSet<TrackId>& changedTrackIds);
    void tracksRelocated(const QList<RelocatedTrack>& relocatedTracks);

    // Emitted by scan() to invoke slotStartScan in the scanner thread's event
    // loop.
    void startScan();

  protected:
    void run() override;

  public slots:
    void queueTask(ScannerTask* pTask);

  private slots:
    void slotStartScan();
    void slotFinishHashedScan();
    void slotFinishUnhashedScan();

    // ScannerTask signal handlers.
    void slotDirectoryHashedAndScanned(const QString& directoryPath,
                                   bool newDirectory, mixxx::cache_key_t hash);
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

    mixxx::DbConnectionPoolPtr m_pDbConnectionPool;

    // The pool of threads used for worker tasks.
    QThreadPool m_pool;

    // The library scanner thread's DAOs.
    LibraryHashDAO m_libraryHashDao;
    CueDAO m_cueDao;
    PlaylistDAO m_playlistDao;
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

    QList<mixxx::FileInfo> m_libraryRootDirs;
    QScopedPointer<LibraryScannerDlg> m_pProgressDlg;
};
