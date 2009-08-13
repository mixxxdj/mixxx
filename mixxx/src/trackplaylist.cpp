/* -*- mode:C++; indent-tabs-mode:s; tab-width:4; c-basic-offset:4; -*- */
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
#include "libraryscanner.h"
#include "defs_audiofiles.h"


/** Note: if you use this, you MUST manually call playlist->setTrackCollection()... */
TrackPlaylist::TrackPlaylist() : QObject(), QList<TrackInfoObject*>()
{
	m_pTrackCollection = NULL;
	m_qName = "Uninitialized playlist";
}

TrackPlaylist::TrackPlaylist(TrackCollection * pTrackCollection, QString qName) : QObject(), QList<TrackInfoObject*>()
{
    m_pTrackCollection = pTrackCollection;
    //m_pTable = 0;
    m_qName = qName;
}

TrackPlaylist::TrackPlaylist(TrackCollection * pTrackCollection, QDomNode node): QObject(), QList<TrackInfoObject*>()
{
    m_pTrackCollection = pTrackCollection;
    //m_pTable = 0;

    loadFromXMLNode(node);

}
void TrackPlaylist::setTrackCollection(TrackCollection * pTrackCollection)
{
    m_pTrackCollection = pTrackCollection;
}

void TrackPlaylist::loadFromXMLNode(QDomNode node)
{
	/*
    // Set name of playlist
    m_qName = XmlParse::selectNodeQString(node, "Name");
    qDebug() << "playlist name" << m_qName;

	//Set comment for playlist
	m_qComment = XmlParse::selectNodeQString(node, "Comment");

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
    }*/
}

QString TrackPlaylist::getName()
{
    return m_qName;
}

TrackPlaylist::~TrackPlaylist()
{
}

void TrackPlaylist::writeXML(QDomDocument &doc, QDomElement &header)
{
    XmlParse::addElement(doc, header, "Name", m_qName);
    XmlParse::addElement(doc, header, "Comment", m_qComment);
    QDomElement root = doc.createElement("List");

    for(int i = 0; i < this->size(); i++)
    {
        XmlParse::addElement(doc, root, "Id", QString("%1").arg(this->at(i)->getId()));
    }
    header.appendChild(root);

}


void TrackPlaylist::addTrack(TrackInfoObject * pTrack)
{
    // Currently a track can only appear once in a playlist
    if (this->indexOf(pTrack)!=-1)
    {
    	qDebug() << "FIXME: Duplicate tracks not allowed in playlists.";
        return;
    }

    this->append(pTrack);

    // If this playlist is active, update WTableTrack
    //if (m_pTable)
    //pTrack->insertInTrackTableRow(m_pTable, m_pTable->numRows());

}

void TrackPlaylist::addTrack(QString qLocation)
{
    //qDebug() << "Add track" << qLocation;
    TrackInfoObject * pTrack = m_pTrackCollection->getTrack(qLocation);

    //QFileInfo fi(qLocation);
    //emit(progressLoading(fi.baseName()));

    if (pTrack)
        addTrack(pTrack);
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
    //for (uint i=0; i<lst.count(); ++i )
    //    addPath(Q3UriDrag::uriToLocalFile(lst.at(i)));
}

void TrackPlaylist::dumpInfo()
{

    qDebug() << "*** Dumping Playlist Information ***";
    qDebug() << "Name: " << getName();
    qDebug() << "List Name: " << getListName();
    qDebug() << "Listing Songs...";



/*
    TrackCollection * tmpCollection = getCollection();
    qDebug() << "Collection Size: " << tmpCollection->getSize();
    for(int i = 0; i < this->count(); ++i)
    {
        TrackInfoObject * tmpTrack = this->at(i);

        qDebug() << "[" << tmpTrack->getId() << "] " << tmpTrack->getTitle();
    }
*/
    qDebug() << "*** End Playlist Dump ***";

}

QString TrackPlaylist::getComment()
{
	return m_qComment;
}
void TrackPlaylist::setComment(QString comment)
{
	m_qComment = comment;
}

void TrackPlaylist::sortByScore(bool ascending)
{
    if(ascending)
    {
        qStableSort(this->begin(), this->end(), ScoreLesser);
    }
    else
    {
        qStableSort(this->begin(), this->end(), ScoreGreater);
    }
}
void TrackPlaylist::sortByTitle(bool ascending)
{
    if(ascending)
    {
        qStableSort(this->begin(), this->end(), TitleLesser);
    }
    else
    {
        qStableSort(this->begin(), this->end(), TitleGreater);
    }
}
void TrackPlaylist::sortByArtist(bool ascending)
{
    if(ascending)
    {
        qStableSort(this->begin(), this->end(), ArtistLesser);
    }
    else
    {
        qStableSort(this->begin(), this->end(), ArtistGreater);
    }
}
void TrackPlaylist::sortByType(bool ascending)
{
    if(ascending)
    {
        qStableSort(this->begin(), this->end(), TypeLesser);
    }
    else
    {
        qStableSort(this->begin(), this->end(), TypeGreater);
    }
}
void TrackPlaylist::sortByDuration(bool ascending)
{
    if(ascending)
    {
        qStableSort(this->begin(), this->end(), DurationLesser);
    }
    else
    {
        qStableSort(this->begin(), this->end(), DurationGreater);
    }
}
void TrackPlaylist::sortByBitrate(bool ascending)
{
    if(ascending)
    {
        qStableSort(this->begin(), this->end(), BitrateLesser);
    }
    else
    {
        qStableSort(this->begin(), this->end(), BitrateGreater);
    }
}
void TrackPlaylist::sortByBpm(bool ascending)
{
    if(ascending)
    {
        qStableSort(this->begin(), this->end(), BpmLesser);
    }
    else
    {
        qStableSort(this->begin(), this->end(), BpmGreater);
    }
}
void TrackPlaylist::sortByComment(bool ascending)
{
    if(ascending)
    {
        qStableSort(this->begin(), this->end(), CommentLesser);
    }
    else
    {
        qStableSort(this->begin(), this->end(), CommentGreater);
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
