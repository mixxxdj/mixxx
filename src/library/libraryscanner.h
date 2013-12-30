
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
#include <QList>
#include <QString>
#include <QList>
#include <QWidget>
#include <QSqlDatabase>
#include <QStringList>

#include "library/dao/cratedao.h"
#include "library/dao/cuedao.h"
#include "library/dao/libraryhashdao.h"
#include "library/dao/directorydao.h"
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
    LibraryScanner(TrackCollection* collection);
    virtual ~LibraryScanner();

    void run();
    void scan(QWidget *parent);

  public slots:
    void cancel();
    void resetCancel();

  signals:
    void scanFinished();
    void progressHashing(QString);

  private:
    bool recursiveScan(const QString& dirPath, QStringList& verifiedDirectories);
    TrackCollection* m_pCollection; // The library trackcollection
    QSqlDatabase m_database; // Hang on to a different DB connection
                             // since we run in a different thread
    LibraryScannerDlg* m_pProgress; // The library scanning window
    LibraryHashDAO m_libraryHashDao;
    CueDAO m_cueDao;
    PlaylistDAO m_playlistDao;
    CrateDAO m_crateDao;
    DirectoryDAO m_directoryDao;
    AnalysisDao m_analysisDao;
    TrackDAO m_trackDao;
    QStringList m_nameFilters;
    volatile bool m_bCancelLibraryScan;
    QStringList m_directoriesBlacklist;
};

#endif
