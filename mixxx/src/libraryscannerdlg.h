/***************************************************************************
                          LibraryScannerDlg.cpp  -  shows library scanning
                                                       progress
                             -------------------
    begin                : 11/27/2007
    copyright            : (C) 2007 Albert Santoni and Adam Davison
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


#ifndef LIBRARYSCANNERDLG_H
#define LIBRARYSCANNERDLG_H

#include <QThread>
#include <QtCore>
#include <QtGui>
#include "trackplaylistlist.h"

class LibraryScannerDlg : public QObject
{
    Q_OBJECT
    
    public:
        LibraryScannerDlg();
        LibraryScannerDlg(TrackPlaylistList* playlists);
        ~LibraryScannerDlg();
    public slots:
	    void slotStartTiming();
	    void slotStopTiming();
	    void slotCheckTiming(QString path);  
	    void slotCancel();  
        
    signals:
        void scanFinished();
        void scanCancelled();
    private:
        TrackPlaylistList* m_qPlaylists;    //The list of playlists
        QString m_qLibraryPath;             //The path to the library on disk
	    void setupTiming();        
            
	    static QTime m_timer;
	    static int m_timeruses;
	    static QWidget* m_progress;
	    static QLabel* m_current;
	    static QPushButton* m_cancel;
	    static bool m_timersetup;
	    QMutex m_qMutex;
	    bool m_bCancelled;    
};

#endif
