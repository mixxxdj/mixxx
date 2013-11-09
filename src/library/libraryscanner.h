
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
#include <QtCore>
#include <QList>

#include "library/dao/cratedao.h"
#include "library/dao/cuedao.h"
#include "library/dao/libraryhashdao.h"
#include "library/dao/playlistdao.h"
#include "library/dao/trackdao.h"
#include "library/dao/analysisdao.h"

#include "libraryscannerdlg.h"
#include "trackcollection.h"

class TrackInfoObject;

class LibraryScanner : public QThread {
    Q_OBJECT
  public:
    LibraryScanner();
    LibraryScanner(TrackCollection* pTrackCollection);
    virtual ~LibraryScanner();

    void run();
    void scan(const QString &libraryPath, QWidget *parent);
    void scan();
    bool recursiveScan(const QString &dirPath, QStringList& verifiedDirectories);
    bool importDirectory(const QString &directory, const QStringList &nameFilters, volatile bool *cancel);
    void addTrackToChunk(const QString filePath);
    void addChunkToDatabase();

    QMutex m_pauseMutex;
  public slots:
    void cancel();
    void resetCancel();
    void pause();
    void resume();
    void updateProgress();
  signals:
    void startedLoading();
    void progressLoading(QString path);
    void finishedLoading();
    void scanFinished();
    void progressHashing(QString);
  private:
    TrackCollection* m_pTrackCollection; // The library trackcollection
    QSqlDatabase m_database; // Hang on to a different DB connection
                             // since we run in a different thread
    QString m_qLibraryPath; // The path to the library on disk
    LibraryScannerDlg* m_pProgress; // The library scanning window

    LibraryHashDAO m_libraryHashDao;
    CueDAO m_cueDao;
    PlaylistDAO m_playlistDao;
    CrateDAO m_crateDao;
    AnalysisDao m_analysisDao;
    TrackDAO m_trackDao;

    QStringList m_nameFilters;
    volatile bool m_bCancelLibraryScan;
    QStringList m_directoriesBlacklist;
    QStringList m_tracksListInCnunk;
    QTimer m_timer;
    QString m_tmpTrackPath;
};

#endif
