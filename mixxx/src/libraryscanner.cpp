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


LibraryScanner::LibraryScanner()
{
    m_qPlaylists = NULL;
    m_qLibraryPath = "";
}

LibraryScanner::LibraryScanner(TrackPlaylistList* playlists, QString libraryPath)
{
    m_qPlaylists = playlists;
    m_qLibraryPath = libraryPath;
}

LibraryScanner::~LibraryScanner()
{

}

void LibraryScanner::run()
{
    m_qPlaylists->at(0)->addPath(m_qLibraryPath);
    emit scanFinished();
}

void LibraryScanner::scan(QString libraryPath)
{
    m_qLibraryPath = libraryPath;
    scan();
}

void LibraryScanner::scan()
{
    start(); //Calls run()
}


