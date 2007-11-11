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

bool TrackPlaylist::m_timersetup = false;
QTime TrackPlaylist::m_timer;
int TrackPlaylist::m_timeruses = 0;
QWidget* TrackPlaylist::m_progress = 0;
QLabel* TrackPlaylist::m_current = 0;

Track * TrackPlaylist::spTrack = 0;

TrackPlaylist::TrackPlaylist(TrackCollection * pTrackCollection, QString qName)
{
    m_pTrackCollection = pTrackCollection;
    //m_pTable = 0;
    m_qName = qName;
    iCounter = 0;

	if (!m_timersetup) {
		m_timersetup = true;
		setupTiming();
	}
}

TrackPlaylist::TrackPlaylist(TrackCollection * pTrackCollection, QDomNode node)
{

	if (!m_timersetup) {
		m_timersetup = true;
		setupTiming();
	}

    m_pTrackCollection = pTrackCollection;
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

void TrackPlaylist::setupTiming() {
	m_timeruses = 0;
	m_progress = new QWidget();
	QVBoxLayout* layout = new QVBoxLayout();
	layout->addWidget(new QLabel("It's taking Mixxx a minute to scan your music library, please wait a minute..."));
	m_current = new QLabel();
	layout->addWidget(m_current);
	m_progress->setLayout(layout);
}

void TrackPlaylist::startTiming() {
	if (m_timeruses == 0) {
		m_timer = QTime::currentTime();
	}
	m_timeruses++;
}

void TrackPlaylist::stopTiming() {
	m_timeruses--;
	if (m_timeruses == 0) {
		m_progress->setVisible(false);
	}
}

void TrackPlaylist::checkTiming(QString path) {
	if (!m_progress->isVisible() && m_timer.elapsed() > 1000) {
		m_progress->setVisible(true);
	}

	// This is a bit ghetto because we're in the startup thread, you can't really
	// repaint the gui properly, so this may all fall over at some point...
	if (m_progress->isVisible()) {
		m_current->setText("Scanning: " + path);
		m_current->repaint();
	}
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

void TrackPlaylist::addPath(QString qPath)
{
	startTiming();

    // Is this a file or directory?
    bool bexists = false;
    TrackCollection * tempCollection = getCollection();
    QDir dir(qPath);

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
			checkTiming(qPath);
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
			checkTiming(d.filePath());
        }

        // And then add all the files

        dir.setFilter(QDir::Files);
        dir.setNameFilter("*.wav *.Wav *.WAV *.mp3 *.Mp3 *.MP3 *.ogg *.Ogg *.OGG *.aiff *.Aiff *.AIFF *.aif *.Aif *.AIF");
        QListIterator<QFileInfo> it(dir.entryInfoList());          // create list iterator
        QFileInfo fi;   // pointer for traversing

        while (it.hasNext())
        {
            fi = it.next();
            for(int i = 1; i < getCollection()->getSize(); ++i)
            {
                /*qDebug("Checking: %s",tempCollection->getTrack(i)->getFilename());*/
                if (tempCollection->getTrack(i))
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

	stopTiming();
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
