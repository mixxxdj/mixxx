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
#include "libraryscanner.h"
#include "libraryscannerdlg.h"


LibraryScanner::LibraryScanner()
{
    m_qLibraryPlaylist = NULL;
    m_qLibraryPath = "";
}

LibraryScanner::LibraryScanner(TrackPlaylist* library_playlist, QString libraryPath)
{
    m_qLibraryPlaylist = library_playlist;
    m_qLibraryPath = libraryPath;
    
    qDebug() << "Constructed LibraryScanner!!!";
}

LibraryScanner::~LibraryScanner()
{

}

void LibraryScanner::run()
{ 
    //m_pProgress->slotStartTiming();
    
    //Start scanning the library.
    m_qLibraryPlaylist->addPath(m_qLibraryPath);
    
    qDebug() << "Scan finished cleanly";
    //m_pProgress->slotStopTiming();
    
    emit(scanFinished());
}

void LibraryScanner::scan(QString libraryPath)
{
    m_qLibraryPath = libraryPath;
    m_pProgress = new LibraryScannerDlg();

    //The important part here is that we need to use Qt::BlockingQueuedConnection, because we're sending these signals
    //across threads. Normally you'd use regular QueuedConnections for this, but since we don't have an event loop running and
    //we need the signals to get processed immediately, we have to use BlockingQueuedConnection. (DirectConnection isn't an
    //option for sending signals across threads.)
    connect(m_qLibraryPlaylist, SIGNAL(startedLoading()), m_pProgress, SLOT(slotStartTiming()), Qt::BlockingQueuedConnection);
    connect(m_qLibraryPlaylist, SIGNAL(finishedLoading()), m_pProgress, SLOT(slotStopTiming()), Qt::BlockingQueuedConnection);  
    connect(m_qLibraryPlaylist, SIGNAL(progressLoading(QString)), m_pProgress, SLOT(slotCheckTiming(QString)), Qt::BlockingQueuedConnection);
    
    //connect(m_pProgress, SIGNAL(scanCancelled()), this, SLOT(terminate()));  //This causes a deadlock, don't use it.
    connect(m_pProgress, SIGNAL(scanCancelled()), m_qLibraryPlaylist, SLOT(slotCancelLibraryScan()));
    
    //connect(m_qPlaylists->at(0), SIGNAL(finishedLoading()), this, SIGNAL(scanFinished()), Qt::BlockingQueuedConnection);  

    scan();
}

void LibraryScanner::scan()
{
    start(); //Starts the thread by calling run()
}


