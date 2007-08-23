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
#include "wtracktable.h"
#include <q3dragobject.h>
#include <q3cstring.h>
#include <qdir.h>
//Added by qt3to4:
#include <Q3StrList>
#include <QDropEvent>
#include "trackplaylist.h"
#include "track.h"

Track *TrackPlaylist::spTrack = 0;

TrackPlaylist::TrackPlaylist(TrackCollection *pTrackCollection, QString qName)
{
    m_pTrackCollection = pTrackCollection;
    //m_pTable = 0;
    m_qName = qName;
	iCounter = 0;
}

TrackPlaylist::TrackPlaylist(TrackCollection *pTrackCollection, QDomNode node)
{
    m_pTrackCollection = pTrackCollection;
    //m_pTable = 0;

    // Set name of list
    m_qName = XmlParse::selectNodeQString(node, "Name");
    qDebug() << "playlist name" << m_qName.latin1();

    // For each track...
    QDomNode idnode = XmlParse::selectNode(node, "List").firstChild();
    while (!idnode.isNull())
    {
        if (idnode.isElement() && idnode.nodeName()=="ID")
        {
            int id = idnode.toElement().text().toInt();
            TrackInfoObject *pTrack = m_pTrackCollection->getTrack(id);
            if (pTrack && pTrack->checkFileExists())
                addTrack(pTrack);
        }

        idnode = idnode.nextSibling();
    }
}

TrackPlaylist::~TrackPlaylist()
{
}

void TrackPlaylist::setTrack(Track *pTrack)
{
    spTrack = pTrack;
}

void TrackPlaylist::writeXML(QDomDocument &doc, QDomElement &header)
{
	TrackInfoObject *it = m_qList.first();
	
		XmlParse::addElement(doc, header, "Name", m_qName);

		QDomElement root = doc.createElement("List");
		
		while (it)
		{
			XmlParse::addElement(doc, root, "ID", QString("%1").arg(it->getId()));
			it = m_qList.next();
		}
		header.appendChild(root);
	
}


void TrackPlaylist::addTrack(TrackInfoObject *pTrack)
{
    // Currently a track can only appear once in a playlist
    if (m_qList.findRef(pTrack)!=-1)
        return;

    m_qList.append(pTrack);
	++iCounter;

    // If this playlist is active, update WTableTrack
    //if (m_pTable)
        //pTrack->insertInTrackTableRow(m_pTable, m_pTable->numRows());

}

void TrackPlaylist::addTrack(QString qLocation)
{
    qDebug() << "Add track" << qLocation;//.latin1());
    TrackInfoObject *pTrack = m_pTrackCollection->getTrack(qLocation);

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

void TrackPlaylist::slotDrop(QDropEvent *e)
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

void TrackPlaylist::addPath(QString qPath)
{
	// Is this a file or directory?
	bool bexists = false;
	TrackCollection *tempCollection = getCollection();
    QDir dir(qPath);

    if (!dir.exists())
	{
		for(int i = 1; i < tempCollection->getSize(); i++)
		{
			if(tempCollection->getTrack(i)->getLocation() == qPath)
			{
				bexists = true;
				break;
			}
		}
		if(bexists == false)
		{
			addTrack(qPath);
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
          } 

        // And then add all the files

  		  dir.setFilter(QDir::Files);
          dir.setNameFilter("*.wav *.Wav *.WAV *.mp3 *.Mp3 *.MP3 *.ogg *.Ogg *.OGG *.aiff *.Aiff *.AIFF *.aif *.Aif *.AIF");
          QListIterator<QFileInfo> it(dir.entryInfoList());        // create list iterator
          QFileInfo  fi;// pointer for traversing

          while (it.hasNext())
          {
			  fi = it.next();
			  for(int i = 1; i < getCollection()->getSize(); ++i)
			  {
				  /*qDebug("Checking: %s",tempCollection->getTrack(i)->getFilename());*/
				  if(tempCollection->getTrack(i)->getFilename() == fi.fileName())
				  {
					  bexists = true;
					  break;
				  }				  
			  }
			  /*if(bexists==true)
				  qDebug("track exists!");*/
			  if(bexists == false)
			  {
				  /*qDebug("all tracks searched, file does not exist, adding...");*/
				  addTrack(fi.filePath());
			  }
			  
          }
     }
}

void TrackPlaylist::slotRemoveTrack(TrackInfoObject *pTrack)
{
    m_qList.remove(pTrack);
}

void TrackPlaylist::updateScores()
{
    // Update the score column for each track
    TrackInfoObject *it = m_qList.first();
    while (it)
    {
        it->updateScore();
        it = m_qList.next();
    }
}

QString TrackPlaylist::getName()
{
    return m_qName;
}

TrackInfoObject *TrackPlaylist::getFirstTrack()
{
    return m_qList.first();    
}

TrackCollection *TrackPlaylist::getCollection()
{
	return m_pTrackCollection;
}

TrackInfoObject *TrackPlaylist::getTrackAt(int index)
{
	return m_qList.at(index);
}
int TrackPlaylist::getSongNum()
{
	
	return m_qList.count();
}
