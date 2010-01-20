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

class LibraryScannerDlg : public QObject
{
    Q_OBJECT
    
    public:
        LibraryScannerDlg();
        ~LibraryScannerDlg();
    public slots:
	    void slotUpdate(QString path);  
	    void slotCancel();  
        void slotScanFinished();
        
    signals:
        void scanCancelled();
    private:
        QString m_qLibraryPath;             //The path to the library on disk
            
	    QTime m_timer;
        QVBoxLayout* m_layout;
        QLabel* m_label;
	    QWidget* m_progress;
	    QLabel* m_current;
	    QPushButton* m_cancel;
	    bool m_bCancelled;    
};

#endif
