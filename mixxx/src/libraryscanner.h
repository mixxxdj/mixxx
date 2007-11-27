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
#include "trackplaylistlist.h"

class LibraryScanner : public QThread
{
    Q_OBJECT
    
    public:
        LibraryScanner();
        LibraryScanner(TrackPlaylistList* playlists, QString libraryPath);
        ~LibraryScanner();
        void run();
        void scan(QString libraryPath);
        void scan();
        
    signals:
        void scanFinished();
    private:
        TrackPlaylistList* m_qPlaylists;    //The list of playlists
        QString m_qLibraryPath;             //The path to the library on disk
};

#endif
