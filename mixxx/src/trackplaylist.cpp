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
#include <qdragobject.h>
#include <qcstring.h>
#include <qdir.h>
#include "trackplaylist.h"


TrackPlaylist::TrackPlaylist(TrackCollection *pTrackCollection, QString qName)
{
    m_pTrackCollection = pTrackCollection;
    m_pTable = 0;
    m_qName = qName;
}

TrackPlaylist::TrackPlaylist(TrackCollection *pTrackCollection, QDomNode node)
{
    m_pTrackCollection = pTrackCollection;
    m_pTable = 0;

    // Set name of list
    m_qName = XmlParse::selectNodeQString(node, "Name");
    qDebug("playlist name %s",m_qName.latin1());

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

void TrackPlaylist::writeXML(QDomDocument &doc, QDomElement &header)
{
    XmlParse::addElement(doc, header, "Name", m_qName);

    QDomElement root = doc.createElement("List");
    QPtrList<TrackInfoObject>::iterator it = m_qList.begin();
    while (it!=m_qList.end())
    {
        XmlParse::addElement(doc, root, "ID", QString("%1").arg((*it)->getId()));
        ++it;
    }
    header.appendChild(root);
}

void TrackPlaylist::addTrack(TrackInfoObject *pTrack)
{
    // Currently a track can only appear once in a playlist
    if (m_qList.findRef(pTrack)!=-1)
        return;

    m_qList.append(pTrack);

	//qDebug("insert in table");
    // If this playlist is active, update WTableTrack
    if (m_pTable)
        pTrack->insertInTrackTableRow(m_pTable, m_pTable->numRows());

}

void TrackPlaylist::addTrack(QString qLocation)
{
	//qDebug("Add track %s",qLocation.latin1());
    TrackInfoObject *pTrack = m_pTrackCollection->getTrack(qLocation);

    if (pTrack)
        addTrack(pTrack);
}

void TrackPlaylist::activate(WTrackTable *pTable)
{
    m_pTable = pTable;

    m_pTable->setNumRows(m_qList.count());

    int i=0;
    QPtrList<TrackInfoObject>::iterator it = m_qList.begin();
    while (it!=m_qList.end())
    {
        //qDebug("inserting in row %i",i);
        (*it)->insertInTrackTableRow(m_pTable, i);

        ++it;
        ++i;
    }

    // Connect drop events to table to this playlist
    connect(m_pTable, SIGNAL(dropped(QDropEvent *)), this, SLOT(slotDrop(QDropEvent *)));
}

void TrackPlaylist::deactivate()
{
    if (!m_pTable)
        return;
        
    disconnect(m_pTable, SIGNAL(dropped(QDropEvent *)), this, SLOT(slotDrop(QDropEvent *)));

    if (m_pTable)
    {
        m_pTable->setNumRows(0);

        QPtrList<TrackInfoObject>::iterator it = m_qList.begin();
        while (it!=m_qList.end())
        {
//            qDebug("remove");
            (*it)->removeFromTrackTable();
            ++it;
        }
    }

    m_pTable = 0;
}

QString TrackPlaylist::getListName()
{
    return m_qName;
}

void TrackPlaylist::setListName(QString name)
{
    m_qName = name;
}

void TrackPlaylist::slotDrop(QDropEvent *e)
{
    //qDebug("playlist drop");

    // Check if this drag is a playlist subtype
    QString s;
    QCString type("playlist");
    if (QTextDrag::decode(e, s, type))
    {
        e->ignore();
        return;
    }

    if (!QUriDrag::canDecode(e))
    {
        //qDebug("TrackPlaylist: Could not decode drag object.");
        e->ignore();
        return;
    }

    e->accept();
    QStrList lst;
    QUriDrag::decode(e, lst);

    // For each drop element...
    for (uint i=0; i<lst.count(); ++i )
        addPath(QUriDrag::uriToLocalFile(lst.at(i)));
}

void TrackPlaylist::addPath(QString qPath)
{
    //qDebug("path %s",qPath.latin1());

    // Is this a file or directory?
    QDir dir(qPath);
    if (!dir.exists())
        addTrack(qPath);
    else
    {
        dir.setFilter(QDir::Dirs);

        // Check if the dir is empty
        if (dir.entryInfoList()==0)
            return;

        const QFileInfoList dir_list = *dir.entryInfoList();

        // Call addPath on subdirectories
        QFileInfoListIterator dir_it(dir_list);
        QFileInfo *d;
        while ((d=dir_it.current()))
        {
            if (!d->filePath().endsWith(".") && !d->filePath().endsWith(".."))
                addPath(d->filePath());
            ++dir_it;
        }

        //
		// And then add all the files
		//

		// Resize the table
	    //m_pTable->setNumRows(m_pTable->numRows()+dir.count());
		
		dir.setFilter(QDir::Files);
        dir.setNameFilter("*.wav *.Wav *.WAV *.mp3 *.Mp3 *.MP3 *.ogg *.Ogg *.OGG");
        const QFileInfoList *list = dir.entryInfoList();
        QFileInfoListIterator it(*list);        // create list iterator
        QFileInfo *fi;                          // pointer for traversing

        while ((fi=it.current()))
        {
            //qDebug("add %s",fi->filePath().latin1());
            addTrack(fi->filePath());
            ++it;   // goto next list element
        }

		// Set the size of table to the actual number of items
		//m_pTable->setNumRows(m_qList.count());
	}
}

void TrackPlaylist::slotRemoveTrack(TrackInfoObject *pTrack)
{
    m_qList.remove(pTrack);
    pTrack->removeFromTrackTable();
}

void TrackPlaylist::updateScores()
{
    // Update the score column for each track
    QPtrList<TrackInfoObject>::iterator it = m_qList.begin();
    while (it!=m_qList.end())
    {
        (*it)->updateScore();
        ++it;
    }
}


