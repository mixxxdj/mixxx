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
    m_pTable = 0;
    m_qName = qName;
	iCounter = 0;
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

void TrackPlaylist::setTrack(Track *pTrack)
{
    spTrack = pTrack;
}

void TrackPlaylist::writeXML(QDomDocument &doc, QDomElement &header)
{
    XmlParse::addElement(doc, header, "Name", m_qName);

    QDomElement root = doc.createElement("List");
    TrackInfoObject *it = m_qList.first();
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
    //qDebug("insert in table");

    // If this playlist is active, update WTableTrack
    if (m_pTable)
        pTrack->insertInTrackTableRow(m_pTable, m_pTable->numRows());

}

void TrackPlaylist::addTrack(QString qLocation)
{
    qDebug("Add track %s",qLocation.latin1());
    TrackInfoObject *pTrack = m_pTrackCollection->getTrack(qLocation);

    if (pTrack)
        addTrack(pTrack);
}

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

        TrackInfoObject *it = m_qList.first();
        while (it)
        {
//            qDebug("remove");
            it->removeFromTrackTable();
            it = m_qList.next();
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
    //qDebug("path %s",qPath.latin1());

    // Is this a file or directory?
    QDir dir(qPath);
    if (!dir.exists())
        addTrack(qPath);
    else
    {
        dir.setFilter(QDir::Dirs);

        // Check if the dir is empty
        if (dir.entryInfoList().isEmpty())
            return;

        QFileInfoList dir_list = dir.entryInfoList();
        for (int i = 0; i < dir_list.size(); ++i) {
          QFileInfo fi = dir_list.at(i);
          if (!fi.filePath().endsWith(".") && !fi.filePath().endsWith(".."))
              addPath(fi.filePath());
        }

        //
		// And then add all the files
		//

		// Resize the table
	    //m_pTable->setNumRows(m_pTable->numRows()+dir.count());
        dir.setFilter(QDir::Files);
        dir.setNameFilter("*.wav *.Wav *.WAV *.mp3 *.Mp3 *.MP3 *.ogg *.Ogg *.OGG *.aiff *.Aiff *.AIFF *.aif *.Aif *.AIF");
        //        const QFileInfoList list = dir.entryInfoList();
        //QFileInfoListIterator it(dir.entryInfoList());        // create list iterator
        QListIterator<QFileInfo> it(dir.entryInfoList());        // create list iterator
        QFileInfo fi;                          // pointer for traversing

        while (it.hasNext())
        {
            fi = it.next(); // goto next list element
            //qDebug("add %s",fi->filePath().latin1());
            addTrack(fi.filePath()); 
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

int TrackPlaylist::getSongNum()
{
	return iCounter;
}
