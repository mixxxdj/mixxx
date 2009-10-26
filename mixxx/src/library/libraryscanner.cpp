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
#include "defs_audiofiles.h"
#include "libraryscanner.h"
#include "libraryscannerdlg.h"


LibraryScanner::LibraryScanner()
{
	m_pCollection = NULL;
    m_qLibraryPath = "";
}

LibraryScanner::LibraryScanner(TrackCollection* collection)
{
    m_pCollection = collection;
    //m_qLibraryPath = libraryPath;

    qDebug() << "Constructed LibraryScanner!!!";
    
    QSqlDatabase::database().transaction();
    QSqlQuery query;
    query.exec("CREATE TABLE LibraryHashes (directory_path VARCHAR(256) primary key, "
               "hash INTEGER, directory_deleted INTEGER)");
    QSqlDatabase::database().commit();
    
    //Print out any SQL error, if there was one.
    if (query.lastError().isValid()) {
     	qDebug() << query.lastError();
    }

}

LibraryScanner::~LibraryScanner()
{
    //Do housekeeping on the LibraryHashes table. Delete any directories that have been marked as deleted...
    QSqlDatabase::database().transaction();
    QSqlQuery query;
    query.exec("DELETE FROM LibraryHashes "
               "WHERE directory_deleted=1");
    QSqlDatabase::database().commit();
        //Print out any SQL error, if there was one.
    if (query.lastError().isValid()) {
     	qDebug() << query.lastError();
    }
}

void LibraryScanner::run()
{
    unsigned static id = 0; //the id of this thread, for debugging purposes //XXX copypasta (should factor this out somehow), -kousu 2/2009
    QThread::currentThread()->setObjectName(QString("LibraryScanner %1").arg(++id));
    //m_pProgress->slotStartTiming();

    //Start scanning the library.
	
	//First, we're going to temporarily mark all the directories that we've 
	//previously hashed as "deleted". As we search through the directory tree 
	//when we rescan, we'll mark any directory that does still exist as such.
	QSqlDatabase::database().transaction();
	QSqlQuery query;
    query.prepare("UPDATE LibraryHashes "
          "SET directory_deleted=:directory_deleted");
    query.bindValue(":directory_deleted", 1);
    query.exec();
    QSqlDatabase::database().commit();       
    	
    recursiveScan(m_qLibraryPath);

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
    connect(m_pCollection, SIGNAL(progressLoading(QString)), m_pProgress, SLOT(slotUpdate(QString)), Qt::BlockingQueuedConnection);
    connect(this, SIGNAL(scanFinished()), m_pProgress, SLOT(slotScanFinished()));
    connect(m_pProgress, SIGNAL(scanCancelled()), m_pCollection, SLOT(slotCancelLibraryScan()));

    scan();
}

void LibraryScanner::scan()
{
    start(); //Starts the thread by calling run()
}

/** Recursively scan a music library. Doesn't import tracks for any directories that 
    have already been scanned and have not changed. Changes are tracked by performing
    a hash of the directory's file list, and those hashes are stored in the database.
*/
void LibraryScanner::recursiveScan(QString dirPath)
{
    QStringList nameFilters;
    nameFilters = QString(MIXXX_SUPPORTED_AUDIO_FILETYPES).split(" ");
	QDirIterator fileIt(dirPath, nameFilters, QDir::Files | QDir::NoDotAndDotDot);
	QString currentFile;
	
	//qDebug() << "Scanning dir:" << dirPath;
	
	QString newHashStr;
	bool prevHashExists = false;
	int newHash;
	int prevHash;
	
	while (fileIt.hasNext())
	{
	    currentFile = fileIt.next();
	    //qDebug() << currentFile;
	    newHashStr += currentFile;
	}
	
	newHash = qHash(newHashStr);
	
	QSqlDatabase::database().transaction();
 	TrackInfoObject* track = NULL;
    
    QSqlQuery query;
    query.exec("SELECT * FROM LibraryHashes "
                  "WHERE directory_path == \"" + dirPath + "\"");
    //Print out any SQL error, if there was one.
    if (query.lastError().isValid()) {
     	qDebug() << "SELECT hash failed:" << query.lastError();
    }
    //Grab a hash for this directory from the database, from the last time it was scanned.
    if (query.next()) {
        prevHash = query.value(query.record().indexOf("hash")).toInt();
        prevHashExists = true;
        //qDebug() << "prev hash exists";
    }
    else {
        prevHashExists = false;
        prevHash = 0;
        //qDebug() << "prev hash does not exist";
    }
    

    QSqlDatabase::database().commit();
    
    //Compare the hashes, and if they don't match, rescan the files in that directory!    
    if (prevHash != newHash) 
    {
        if (!prevHashExists) {
            query.prepare("INSERT INTO LibraryHashes (directory_path, hash, directory_deleted) "
                          "VALUES (:directory_path, :hash, :directory_deleted)");
            query.bindValue(":directory_path", dirPath);
            query.bindValue(":hash", newHash);
            query.bindValue(":directory_deleted", 0);
            query.exec();
            
            if (query.lastError().isValid()) {
             	qDebug() << "Creating new dirhash failed:" << query.lastError();
            }
        }
        else //Just need to update the old hash in the database
        {
            query.prepare("UPDATE LibraryHashes "
                  "SET hash=:hash, directory_deleted=:directory_deleted "
                  "WHERE directory_path = \"" + dirPath + "\"");
            query.bindValue(":hash", newHash);
            query.bindValue(":directory_deleted", 0);
            query.exec();
            
            if (query.lastError().isValid()) {
                qDebug() << "Updating existing dirhash failed:" << query.lastError();
            }
        }
        
        //Rescan that mofo!
        m_pCollection->importDirectory(dirPath);
    }
    else //prevHash == newHash
    {
        //The files in the directory haven't changed, so we don't need to do anything, right?
        //Wrong! We need to mark the directory in the database as "existing", so that we can
        //keep track of directories that have been deleted to stop the database from keeping
        //rows about deleted directories around. :)
            
        query.prepare("UPDATE LibraryHashes "
              "SET directory_deleted=:directory_deleted "
              "WHERE hash = " + QString("%1").arg(newHash));
        query.bindValue(":directory_deleted", 0);
        query.exec();
        
        if (query.lastError().isValid()) {
            qDebug() << "Updating dirhash to mark as existing failed:" << query.lastError();
        }        
    }
    
    //Look at all the subdirectories and scan them recursively...
    QDirIterator dirIt(dirPath, QDir::Dirs | QDir::NoDotAndDotDot);
    while (dirIt.hasNext())
    {
        recursiveScan(dirIt.next());
    }
}

/**
   Table: LibraryHashes
   PRIMARY KEY string directory
   string hash

    
   Recursive Algorithm:
   1) QDirIterator, iterate over all _files_ in a directory to construct a giant string.
   2) newHash = Hash that string.
   3) prevHash = SELECT from LibraryHashes * WHERE directory == strDirectory
   4) if (prevHash != newHash) scanDirectory(strDirectory); //Do a NON-RECURSIVE scan of the files in that dir.
   5) For each directory in strDirectory, execute this algorithm.
   
  */

