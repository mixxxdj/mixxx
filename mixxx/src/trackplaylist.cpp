/* -*- mode:C++; indent-tabs-mode:t; tab-width:8; c-basic-offset:4; -*- */
//
// C++ Implementation: trackplaylist
//
// Description:
//
//
// Author: Tue Haste Andersen <haste@diku.dk>, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "trackplaylist.h"
#include "trackcollection.h"
#include "xmlparse.h"
#include <q3dragobject.h>
#include <q3cstring.h>
#include <qprogressbar.h>
#include <qdir.h>
//Added by qt3to4:
#include <Q3StrList>
#include <QDropEvent>
#include <qpushbutton.h>
#include "trackplaylist.h"
#include "track.h"
#include "libraryscanner.h"



Track * TrackPlaylist::spTrack = 0;

TrackPlaylist::TrackPlaylist(TrackCollection * pTrackCollection, QString qName)
{
    m_pTrackCollection = pTrackCollection;
    //m_pTable = 0;
    m_qName = qName;
    iCounter = 0;
    m_bStopLibraryScan = false;
}

TrackPlaylist::TrackPlaylist(TrackCollection * pTrackCollection, QDomNode node)
{
    m_pTrackCollection = pTrackCollection;
    m_bStopLibraryScan = false;
    //m_pTable = 0;

    // Set name of list
    m_qName = XmlParse::selectNodeQString(node, "Name");
    qDebug() << "playlist name" << m_qName.latin1();

    // For each track...
    QDomNode idnode = XmlParse::selectNode(node, "List").firstChild();
    while (!idnode.isNull())
    {
        if (idnode.isElement() && idnode.nodeName()=="Id")
        {
            int id = idnode.toElement().text().toInt();
            TrackInfoObject * pTrack = m_pTrackCollection->getTrack(id);
            if (pTrack && pTrack->checkFileExists())
                addTrack(pTrack);
        }

        idnode = idnode.nextSibling();
    }
}

TrackPlaylist::~TrackPlaylist()
{
}

void TrackPlaylist::setTrack(Track * pTrack)
{
    spTrack = pTrack;
}

void TrackPlaylist::writeXML(QDomDocument &doc, QDomElement &header)
{
    XmlParse::addElement(doc, header, "Name", m_qName);
    QDomElement root = doc.createElement("List");

    for(int i = 0; i < m_qList.size(); i++)
    {
        XmlParse::addElement(doc, root, "Id", QString("%1").arg(m_qList.at(i)->getId()));
    }
    header.appendChild(root);

}


void TrackPlaylist::addTrack(TrackInfoObject * pTrack)
{
    // Currently a track can only appear once in a playlist
    if (m_qList.indexOf(pTrack)!=-1)
        return;

    m_qList.append(pTrack);
    ++iCounter;

    // If this playlist is active, update WTableTrack
    //if (m_pTable)
    //pTrack->insertInTrackTableRow(m_pTable, m_pTable->numRows());

}

void TrackPlaylist::addTrack(QString qLocation)
{
    qDebug() << "Add track" << qLocation; //.latin1());
    TrackInfoObject * pTrack = m_pTrackCollection->getTrack(qLocation);

    //QFileInfo fi(qLocation);
    //emit(progressLoading(fi.baseName()));

    if (pTrack)
        addTrack(pTrack);
}

/*
   void TrackPlaylist::activate(WTrackTable *pTable)
   {
    m_pTable = pTable;

    m_pTable->setNumRows(m_qList.count());

    int i=0;
    TrackInfoObject *it = m_qList.first();
    while (it)
    {
        //qDebug("inserting in row %i",i);
        it->insertInTrackTableRow(m_pTable, i);

        it = m_qList.next();
 ++i;
    }

    // Connect drop events to table to this playlist
    //connect(m_pTable, SIGNAL(dropped(QDropEvent *)), this, SLOT(slotDrop(QDropEvent *)));
   }

   void TrackPlaylist::deactivate()
   {
    if (!m_pTable)
        //return;

    //disconnect(m_pTable, SIGNAL(dropped(QDropEvent *)), this, SLOT(slotDrop(QDropEvent *)));
    if (m_pTable)
    {
        m_pTable->setNumRows(0);

        TrackInfoObject *it = m_qList.first();
        while (it)
        {
            it->clearTrackTableRow();
                        qDebug("removing: %s",it->getFilename());
            it = m_qList.next();
        }
    }

    m_pTable = 0;
   }
 */
QString TrackPlaylist::getListName()
{
    return m_qName;
}

void TrackPlaylist::setListName(QString name)
{
    m_qName = name;

    // Update views
    //if (spTrack)
    //    spTrack->updatePlaylistViews();
}

void TrackPlaylist::slotDrop(QDropEvent * e)
{
    // Check if this drag is a playlist subtype
    QString s;
    Q3CString type("playlist");
    //TODO: PORT TO QT4
    // if (Q3TextDrag::decode(e, s, type))
//     {
//         e->ignore();
//         return;
//     }

    if (!Q3UriDrag::canDecode(e))
    {
        e->ignore();
        return;
    }

    e->accept();
    Q3StrList lst;
    Q3UriDrag::decode(e, lst);

    // For each drop element...
    for (uint i=0; i<lst.count(); ++i )
        addPath(Q3UriDrag::uriToLocalFile(lst.at(i)));
}

void TrackPlaylist::dumpInfo()
{

    qDebug() << "*** Dumping Playlist Information ***";
    qDebug() << "Name: " << getName();
    qDebug() << "List Name: " << getListName();
    qDebug() << "Song Count: " << getSongNum();
    qDebug() << "Listing Songs...";

    TrackCollection * tmpCollection = getCollection();
    qDebug() << "Collection Size: " << tmpCollection->getSize();
    for(int i = 0; i < m_qList.count(); ++i)
    {
        TrackInfoObject * tmpTrack = m_qList.at(i);

        qDebug() << "[" << tmpTrack->getId() << "] " << tmpTrack->getTitle();
    }

    qDebug() << "*** End Playlist Dump ***";

}

void TrackPlaylist::slotCancelLibraryScan()
{
    m_qLibScanMutex.lock();
    m_bStopLibraryScan = true;
    m_qLibScanMutex.unlock();
}

void TrackPlaylist::addPath(QString qPath)
{    
    emit(startedLoading());
    //qDebug() << "addPath";
   
    // Is this a file or directory?
    bool bexists = false;
    TrackCollection * tempCollection = getCollection();
    QDir dir(qPath);
    
    emit(progressLoading(qPath));
        
    //Check if the scan has been cancelled (because this function is called recursively and we can't use
    //terminate() to end the thread safely.)
    m_qLibScanMutex.lock();
    if (m_bStopLibraryScan)
    {
    	m_qLibScanMutex.unlock();
    	return;
    }
    m_qLibScanMutex.unlock();
	
    if (!dir.exists())
    {
        for(int i = 1; i < tempCollection->getSize(); i++)
        {
            if (tempCollection->getTrack(i))
                if(tempCollection->getTrack(i)->getLocation() == qPath)
                {
                    bexists = true;
                    break;
                }
        }
        if(bexists == false)
        {
            addTrack(qPath);
            emit(progressLoading(qPath));
        }
    }
    else
    {
        dir.setFilter(QDir::Dirs);

        // Check if the dir is empty
        if (dir.entryInfoList().isEmpty())
            return;

        QListIterator<QFileInfo> dir_it(dir.entryInfoList());
        QFileInfo d;
        while (dir_it.hasNext())
        {
            d = dir_it.next();
            if (!d.filePath().endsWith(".") && !d.filePath().endsWith(".."))
                addPath(d.filePath());
           emit(progressLoading(d.filePath()));
        }

        // And then add all the files

        dir.setFilter(QDir::Files);
        dir.setNameFilter("*.wav *.Wav *.WAV *.mp3 *.Mp3 *.MP3 *.ogg *.Ogg *.OGG *.aiff *.Aiff *.AIFF *.aif *.Aif *.AIF");
        QListIterator<QFileInfo> it(dir.entryInfoList());          // create list iterator
        QFileInfo fi;   // pointer for traversing

        while (it.hasNext())
        {
            fi = it.next();
            
			QString fn = fi.fileName();
			if (fn.contains("Eutow")) {
				int a = 0;
			}

            //Check if the scan has been cancelled.
            m_qLibScanMutex.lock();
            if (m_bStopLibraryScan)
            {
            	m_qLibScanMutex.unlock();
            	return;
            }
            m_qLibScanMutex.unlock();
            
            for(int i = 1; i < getCollection()->getSize(); ++i)
            {
                /*qDebug("Checking: %s",tempCollection->getTrack(i)->getFilename());*/
                if (tempCollection->getTrack(i))
                    if(tempCollection->getTrack(i)->getFilename() == fi.fileName() &&
						tempCollection->getTrack(i)->getFilepath() == fi.absolutePath()) {

                        bexists = true;
                        emit(progressLoading(fi.fileName())); //We're not actually reloading the library in this case, 
                        			      //just checking if songs exist.
                        break;
                    }
            }
            /*if(bexists==true)
                    qDebug("track exists!");*/
            if(bexists == false)
            {
                /*qDebug("all tracks searched, file does not exist, adding...");*/
                addTrack(fi.filePath());
		emit(progressLoading(fi.fileName()));
            }

        }
    }
    
    emit(finishedLoading());
}

void TrackPlaylist::slotRemoveTrack(TrackInfoObject * pTrack)
{
    m_qList.remove(pTrack);
}

void TrackPlaylist::updateScores()
{
    // Update the score column for each track

    for(int i = 0; i < m_qList.size(); i++)
    {
        m_qList.at(i)->updateScore();
    }
}

QString TrackPlaylist::getName()
{
    return m_qName;
}

TrackInfoObject * TrackPlaylist::getFirstTrack()
{
    return m_qList.first();
}

TrackCollection * TrackPlaylist::getCollection()
{
    return m_pTrackCollection;
}

int TrackPlaylist::getIndexOf(int id)
{
    for(int i = 0; i < m_qList.count(); ++i)
    {
        TrackInfoObject * tmpTrack = m_qList.at(i);

        if(tmpTrack->getId() == id)
            return i;
    }
    return -1;
}

TrackInfoObject * TrackPlaylist::getTrackAt(int index)
{
    if(index < 0)
        return NULL;

    if(index >= m_qList.size())
        return NULL;

    return m_qList.at(index);
}
int TrackPlaylist::getSongNum()
{

    return m_qList.count();
}

void TrackPlaylist::sortByScore(bool ascending)
{
    if(ascending)
    {
        qStableSort(m_qList.begin(), m_qList.end(), ScoreLesser);
    }
    else
    {
        qStableSort(m_qList.begin(), m_qList.end(), ScoreGreater);
    }
}
void TrackPlaylist::sortByTitle(bool ascending)
{
    if(ascending)
    {
        qStableSort(m_qList.begin(), m_qList.end(), TitleLesser);
    }
    else
    {
        qStableSort(m_qList.begin(), m_qList.end(), TitleGreater);
    }
}
void TrackPlaylist::sortByArtist(bool ascending)
{
    if(ascending)
    {
        qStableSort(m_qList.begin(), m_qList.end(), ArtistLesser);
    }
    else
    {
        qStableSort(m_qList.begin(), m_qList.end(), ArtistGreater);
    }
}
void TrackPlaylist::sortByType(bool ascending)
{
    if(ascending)
    {
        qStableSort(m_qList.begin(), m_qList.end(), TypeLesser);
    }
    else
    {
        qStableSort(m_qList.begin(), m_qList.end(), TypeGreater);
    }
}
void TrackPlaylist::sortByDuration(bool ascending)
{
    if(ascending)
    {
        qStableSort(m_qList.begin(), m_qList.end(), DurationLesser);
    }
    else
    {
        qStableSort(m_qList.begin(), m_qList.end(), DurationGreater);
    }
}
void TrackPlaylist::sortByBitrate(bool ascending)
{
    if(ascending)
    {
        qStableSort(m_qList.begin(), m_qList.end(), BitrateLesser);
    }
    else
    {
        qStableSort(m_qList.begin(), m_qList.end(), BitrateGreater);
    }
}
void TrackPlaylist::sortByBpm(bool ascending)
{
    if(ascending)
    {
        qStableSort(m_qList.begin(), m_qList.end(), BpmLesser);
    }
    else
    {
        qStableSort(m_qList.begin(), m_qList.end(), BpmGreater);
    }
}
void TrackPlaylist::sortByComment(bool ascending)
{
    if(ascending)
    {
        qStableSort(m_qList.begin(), m_qList.end(), CommentLesser);
    }
    else
    {
        qStableSort(m_qList.begin(), m_qList.end(), CommentGreater);
    }
}

bool ScoreLesser(const TrackInfoObject * tio1, const TrackInfoObject * tio2)
{
    return tio1->getScore() < tio2->getScore();
}
bool TitleLesser(const TrackInfoObject * tio1, const TrackInfoObject * tio2)
{
    return tio1->getTitle() < tio2->getTitle();
}
bool ArtistLesser(const TrackInfoObject * tio1, const TrackInfoObject * tio2)
{
    return tio1->getArtist() < tio2->getArtist();
}
bool TypeLesser(const TrackInfoObject * tio1, const TrackInfoObject * tio2)
{
    return tio1->getType() < tio2->getType();
}
bool DurationLesser(const TrackInfoObject * tio1, const TrackInfoObject * tio2)
{
    return tio1->getDurationStr() < tio2->getDurationStr();
}
bool BitrateLesser(const TrackInfoObject * tio1, const TrackInfoObject * tio2)
{
    return tio1->getBitrate() < tio2->getBitrate();
}
bool BpmLesser(const TrackInfoObject * tio1, const TrackInfoObject * tio2)
{
    return tio1->getBpm() < tio2->getBpm();
}
bool CommentLesser(const TrackInfoObject * tio1, const TrackInfoObject * tio2)
{
    return tio1->getComment() < tio2->getComment();
}
bool ScoreGreater(const TrackInfoObject * tio1, const TrackInfoObject * tio2)
{
    return tio1->getScore() > tio2->getScore();
}
bool TitleGreater(const TrackInfoObject * tio1, const TrackInfoObject * tio2)
{
    return tio1->getTitle() > tio2->getTitle();
}
bool ArtistGreater(const TrackInfoObject * tio1, const TrackInfoObject * tio2)
{
    return tio1->getArtist() > tio2->getArtist();
}
bool TypeGreater(const TrackInfoObject * tio1, const TrackInfoObject * tio2)
{
    return tio1->getType() > tio2->getType();
}
bool DurationGreater(const TrackInfoObject * tio1, const TrackInfoObject * tio2)
{
    return tio1->getDurationStr() > tio2->getDurationStr();
}
bool BitrateGreater(const TrackInfoObject * tio1, const TrackInfoObject * tio2)
{
    return tio1->getBitrate() > tio2->getBitrate();
}
bool BpmGreater(const TrackInfoObject * tio1, const TrackInfoObject * tio2)
{
    return tio1->getBpm() > tio2->getBpm();
}
bool CommentGreater(const TrackInfoObject * tio1, const TrackInfoObject * tio2)
{
    return tio1->getComment() > tio2->getComment();
}

int TrackPlaylist::operator<(TrackPlaylist * p2)
{
    //TrackPlaylist * p1 = (TrackPlaylist *)item1;
    //TrackPlaylist * p2 = (TrackPlaylist *)item2;

    if (this->getListName()==p2->getListName())
        return 0;
    else if (this->getListName()>p2->getListName())
        return 1;
    else
        return -1;
}
