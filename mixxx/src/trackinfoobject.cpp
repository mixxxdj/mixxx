/***************************************************************************
                          trackinfoobject.cpp  -  description
                             -------------------
    begin                : 10 02 2003
    copyright            : (C) 2003 by Tue & Ken Haste Andersen
    email                : haste@diku.dk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qstring.h"
#include "qdom.h"
#include <qfileinfo.h>
#include "trackinfoobject.h"
#ifdef __WIN__
  #include "soundsourcesndfile.h"
#endif
#ifdef __UNIX__
  #include "soundsourceaudiofile.h"
#endif
#include "soundsourcemp3.h"
#include "soundsourceoggvorbis.h"
#include "wtracktable.h"
#include "wtracktableitem.h"
#include "xmlparse.h"

int TrackInfoObject::siMaxTimesPlayed = 0;

TrackInfoObject::TrackInfoObject(const QString sPath, const QString sFile) : m_sFilename(sFile), m_sFilepath(sPath)
{
    m_sArtist = "";
    m_sTitle = "";
    m_sType= "";
    m_sComment = "";
    m_iDuration = 0;
    m_iLength = 0;
    m_iBitrate = 0;
    m_iTimesPlayed = 0;
    m_fBpm = 0.;
    m_fBpmConfidence = 0.;
    m_iScore = 0;
    m_iId = 0;

    m_pTableItemScore = 0;
    m_pTableItemTitle = 0;
    m_pTableItemArtist = 0;
    m_pTableItemComment = 0;
    m_pTableItemType = 0;
    m_pTableItemDuration = 0;
    m_pTableItemBpm = 0;
    m_pTableItemBitrate = 0;

    m_pTableTrack = 0;

    // Check that the file exists:
    checkFileExists();

    parse();
}

TrackInfoObject::TrackInfoObject(const QDomNode &nodeHeader)
{
    m_sFilename = selectNodeStr( nodeHeader, "Filename");
    m_sFilepath = selectNodeStr( nodeHeader, "Filepath");
    m_sTitle = selectNodeStr( nodeHeader, "Title");
    m_sArtist = selectNodeStr( nodeHeader, "Artist");
    m_sType = selectNodeStr( nodeHeader, "Type");
    m_sComment = selectNodeStr( nodeHeader, "Comment");
    m_iDuration = selectNodeStr( nodeHeader, "Duration").toInt();
    m_iBitrate = selectNodeStr( nodeHeader, "Bitrate").toInt();
    m_iLength = selectNodeStr( nodeHeader, "Length").toInt();
    m_iTimesPlayed = selectNodeStr( nodeHeader, "TimesPlayed").toInt();
    m_fBpm = selectNodeStr( nodeHeader, "Bpm").toFloat();
    m_fBpmConfidence = selectNodeStr( nodeHeader, "BpmConfidence").toFloat();
    m_iScore = 0;
    m_iId = selectNodeStr( nodeHeader, "ID").toInt();
    m_pTableTrack = 0;

    m_pTableItemScore = 0;
    m_pTableItemTitle = 0;
    m_pTableItemArtist = 0;
    m_pTableItemComment = 0;
    m_pTableItemType = 0;
    m_pTableItemDuration = 0;
    m_pTableItemBpm = 0;
    m_pTableItemBitrate = 0;

    if (m_iTimesPlayed>siMaxTimesPlayed)
        siMaxTimesPlayed = m_iTimesPlayed;

    // Check that the actual file exists:
    checkFileExists();
}

QDomNode TrackInfoObject::selectNode( const QDomNode &nodeHeader, const QString sNode )
{
    QDomNode node = nodeHeader.firstChild();

    while ( !node.isNull() )
    {
        if (node.nodeName() == sNode)
            return node;
        node = node.nextSibling();
    }
    return node;
}

QString TrackInfoObject::selectNodeStr( const QDomNode &nodeHeader, const QString sNode )
{
    QString s = "";
    QDomNode node = selectNode(nodeHeader, sNode);
    if (!node.isNull())
        s = node.toElement().text();
    return s;
}

TrackInfoObject::~TrackInfoObject()
{
    removeFromTrackTable();
}

void TrackInfoObject::checkFileExists()
{
    QFile fileTrack(getLocation());
    if (fileTrack.exists())
        m_bExists = true;
    else
    {
        m_bExists = false;
        qDebug("The track %s was not found", getLocation().latin1());
    }
}

/*
    Writes information about the track to the xml file:
*/
void TrackInfoObject::writeToXML( QDomDocument &doc, QDomElement &header )
{
    XmlParse::addElement( doc, header, "Filename", m_sFilename );
    XmlParse::addElement( doc, header, "Filepath", m_sFilepath );
    XmlParse::addElement( doc, header, "Title", m_sTitle );
    XmlParse::addElement( doc, header, "Artist", m_sArtist );
    XmlParse::addElement( doc, header, "Type", m_sType );
    XmlParse::addElement( doc, header, "Comment", m_sComment);
    XmlParse::addElement( doc, header, "Duration", QString("%1").arg(m_iDuration));
    XmlParse::addElement( doc, header, "Bitrate", QString("%1").arg(m_iBitrate));
    XmlParse::addElement( doc, header, "Length", QString("%1").arg(m_iLength) );
    XmlParse::addElement( doc, header, "TimesPlayed", QString("%1").arg(m_iTimesPlayed) );
    XmlParse::addElement( doc, header, "Bpm", QString("%1").arg(m_fBpm) );
    XmlParse::addElement( doc, header, "BpmConfidence", QString("%1").arg(m_fBpmConfidence) );
    XmlParse::addElement( doc, header, "Id", QString("%1").arg(m_iId) );
}

void TrackInfoObject::insertInTrackTableRow(WTrackTable *pTableTrack, int iRow)
{
    // Return if no WTrackTable is instantiated
    if (!pTableTrack)
        return;

    // Ensure the row that is requested for insert in the WTrackTable exists
    if (pTableTrack->numRows()<iRow+1)
        pTableTrack->setNumRows(iRow+1);

    // Update the score
    updateScore();

    // Construct elements to insert into the table, if they are not already allocated
    if (!m_pTableItemScore)
        m_pTableItemScore = new WTrackTableItem(this, pTableTrack,QTableItem::Never, getScoreStr(), typeNumber);
    if (!m_pTableItemTitle)
        m_pTableItemTitle = new WTrackTableItem(this, pTableTrack,QTableItem::Never, m_sTitle, typeText);
    if (!m_pTableItemArtist)
        m_pTableItemArtist = new WTrackTableItem(this, pTableTrack,QTableItem::Never, m_sArtist, typeText);
    if (!m_pTableItemComment)
        m_pTableItemComment = new WTrackTableItem(this, pTableTrack,QTableItem::WhenCurrent, m_sComment, typeText);
    if (!m_pTableItemType)
        m_pTableItemType = new WTrackTableItem(this, pTableTrack,QTableItem::Never, m_sType, typeText);
    if (!m_pTableItemDuration)
        m_pTableItemDuration = new WTrackTableItem(this, pTableTrack,QTableItem::Never, getDurationStr(), typeDuration);
    if (!m_pTableItemBpm)
        m_pTableItemBpm = new WTrackTableItem(this, pTableTrack,QTableItem::Never, getBpmStr(), typeNumber);
    if (!m_pTableItemBitrate)
        m_pTableItemBitrate = new WTrackTableItem(this, pTableTrack,QTableItem::Never, getBitrateStr(), typeNumber);

    //qDebug("inserting.. %p",pTableTrack->item(iRow, COL_SCORE));

    // Insert the elements into the table
    pTableTrack->setItem(iRow, COL_SCORE, m_pTableItemScore);
    pTableTrack->setItem(iRow, COL_TITLE, m_pTableItemTitle);
    pTableTrack->setItem(iRow, COL_ARTIST, m_pTableItemArtist);
    pTableTrack->setItem(iRow, COL_COMMENT, m_pTableItemComment);
    pTableTrack->setItem(iRow, COL_TYPE, m_pTableItemType);
    pTableTrack->setItem(iRow, COL_DURATION, m_pTableItemDuration);
    pTableTrack->setItem(iRow, COL_BPM, m_pTableItemBpm);
    pTableTrack->setItem(iRow, COL_BITRATE, m_pTableItemBitrate);

    m_pTableTrack = pTableTrack;
}

void TrackInfoObject::removeFromTrackTable()
{
    //qDebug("remove");
    if (m_pTableTrack)
    {
        // Remove the row from the table, and delete the table items
        m_pTableTrack->removeRow(m_pTableItemScore->row());

        // Reset pointers
        m_pTableItemScore = 0;
        m_pTableItemTitle = 0;
        m_pTableItemArtist = 0;
        m_pTableItemComment = 0;
        m_pTableItemType = 0;
        m_pTableItemDuration = 0;
        m_pTableItemBpm = 0;
        m_pTableItemBitrate = 0;

        m_pTableTrack = 0;
    }
}

int TrackInfoObject::parse()
{
    // Add basic information derived from the filename:
    parseFilename();

    // Parse the using information stored in the sound file
    int iResult = ERR;
    if (m_sType == "wav")
#ifdef __WIN__
        iResult = SoundSourceSndFile::ParseHeader(this);
#endif
#ifdef __UNIX__
        iResult = SoundSourceAudioFile::ParseHeader(this);
#endif
    else if (m_sType == "mp3")
        iResult = SoundSourceMp3::ParseHeader(this);
    else if (m_sType == "ogg")
        iResult = SoundSourceOggVorbis::ParseHeader(this);

    return iResult;
}


void TrackInfoObject::parseFilename()
{
    if (m_sFilename.find('-') != -1)
    {
        m_sArtist = m_sFilename.section('-',0,0); // Get the first part
        m_sTitle = m_sFilename.section('-',1,1); // Get the second part
        m_sTitle = m_sTitle.section('.',0,-2); // Remove the ending
        m_sType = m_sFilename.section('.',-1); // Get the ending
    }
    else
    {
        m_sTitle = m_sFilename.section('.',0,-2); // Remove the ending;
        m_sType = m_sFilename.section('.',-1); // Get the ending
    }

    // Remove spaces from start and end of title and artist
    while (m_sArtist.startsWith(" "))
        m_sArtist = m_sArtist.right(m_sArtist.length()-1);
    while (m_sArtist.endsWith(" "))
            m_sArtist = m_sArtist.left(m_sArtist.length()-1);
    while (m_sTitle.startsWith(" "))
            m_sTitle = m_sTitle.right(m_sTitle.length()-1);
    while (m_sTitle.endsWith(" "))
            m_sTitle = m_sTitle.left(m_sTitle.length()-1);


    // Sort out obviously wrong parsings:
    if ((m_sArtist.length() < 3) || (m_sTitle < 3))
    {
        m_sTitle = m_sFilename.section('.',0,-2);
        m_sArtist = "";
    }

    // Find the length:
    m_iLength = QFileInfo(m_sFilepath + '/' + m_sFilename).size();

    // Add no comment
    m_sComment = QString("");

    // Find the type
    m_sType = m_sFilename.section(".",-1).lower();

}

QString TrackInfoObject::getDurationStr()
{
    if (m_iDuration <=0)
        return QString("?");
    else
    {
        int iHours = m_iDuration/3600;
        int iMinutes = (m_iDuration - 3600*iHours)/60;
        int iSeconds = m_iDuration%60;

        // Sort out obviously wrong results:
        if (iHours > 5)
            return QString("??");
        if (iHours >= 1)
            return QString().sprintf("%2d:%2d:%02d", iHours, iMinutes, iSeconds);
        else
            return QString().sprintf("%2d:%02d", iMinutes, iSeconds);
    }
}

QString TrackInfoObject::getLocation()
{
    return m_sFilepath + "/" + m_sFilename;
}

float TrackInfoObject::getBpm()
{
    return m_fBpm;
}

void TrackInfoObject::setBpm(float f)
{
    m_fBpm = f;

    if (m_pTableItemBpm)
    {
	    qDebug("update cell");
        m_pTableItemBpm->setText(getBpmStr());
        m_pTableItemBpm->table()->updateCell(m_pTableItemBpm->row(), m_pTableItemBpm->col());
    }
}

QString TrackInfoObject::getBpmStr()
{
    return QString("%1").arg(m_fBpm, 3,'f',1);
}

float TrackInfoObject::getBpmConfidence()
{
    return m_fBpmConfidence;
}

void TrackInfoObject::setBpmConfidence(float f)
{
    m_fBpmConfidence = f;
}

QString TrackInfoObject::getInfo()
{
    return QString("" + m_sArtist + "\n" +
                   "" + m_sTitle + "\n");
//                   "Type   : " + m_sType  + "\n" +
//                   "Bitrate: " + m_sBitrate );
}

int TrackInfoObject::getDuration()
{
    return m_iDuration;
}

void TrackInfoObject::setDuration(int i)
{
    m_iDuration = i;

    if (m_pTableItemDuration)
    {
        m_pTableItemDuration->setText(getDurationStr());
        m_pTableItemDuration->table()->updateCell(m_pTableItemDuration->row(), m_pTableItemDuration->col());
    }
}

QString TrackInfoObject::getTitle()
{
    return m_sTitle;
}

void TrackInfoObject::setTitle(QString s)
{
    m_sTitle = s;

    if (m_pTableItemTitle)
    {
        m_pTableItemTitle->setText(m_sTitle);
        m_pTableItemTitle->table()->updateCell(m_pTableItemTitle->row(), m_pTableItemTitle->col());
    }
}

QString TrackInfoObject::getArtist()
{
    return m_sArtist;
}

void TrackInfoObject::setArtist(QString s)
{
    m_sArtist = s;

    if (m_pTableItemArtist)
    {
        m_pTableItemArtist->setText(m_sArtist);
        m_pTableItemArtist->table()->updateCell(m_pTableItemArtist->row(), m_pTableItemArtist->col());
    }

}

QString TrackInfoObject::getFilename()
{
    return m_sFilename;
}

bool TrackInfoObject::exists()
{
    return m_bExists;
}

int TrackInfoObject::getTimesPlayed()
{
    return m_iTimesPlayed;
}

void TrackInfoObject::incTimesPlayed()
{
    ++m_iTimesPlayed;
    if (m_iTimesPlayed>siMaxTimesPlayed)
        siMaxTimesPlayed = m_iTimesPlayed;
}

void TrackInfoObject::setFilepath(QString s)
{
    m_sFilepath = s;
}

QString TrackInfoObject::getComment()
{
    return m_sComment;
}

void TrackInfoObject::setComment(QString s)
{
    m_sComment = s;

    if (m_pTableItemComment)
    {
        m_pTableItemComment->setText(m_sComment);
        m_pTableItemComment->table()->updateCell(m_pTableItemComment->row(), m_pTableItemComment->col());
    }

}

QString TrackInfoObject::getType()
{
    return m_sType;
}

void TrackInfoObject::setType(QString s)
{
    m_sType = s;

    if (m_pTableItemType)
    {
        m_pTableItemType->setText(m_sType);
        m_pTableItemType->table()->updateCell(m_pTableItemType->row(), m_pTableItemType->col());
    }
}

int TrackInfoObject::getLength()
{
    return m_iLength;
}

int TrackInfoObject::getBitrate()
{
    return m_iBitrate;
}

QString TrackInfoObject::getBitrateStr()
{
    return QString("%1").arg(m_iBitrate);
}

void TrackInfoObject::setBitrate(int i)
{
    m_iBitrate = i;

    if (m_pTableItemBitrate)
    {
        m_pTableItemBitrate->setText(getBitrateStr());
        m_pTableItemBitrate->table()->updateCell(m_pTableItemBitrate->row(), m_pTableItemBitrate->col());
    }
}

QString TrackInfoObject::getScoreStr()
{
    return QString("%1").arg(m_iScore);
}

void TrackInfoObject::updateScore()
{
    m_iScore = 99*m_iTimesPlayed/siMaxTimesPlayed;

    if (m_pTableItemScore)
    {
        m_pTableItemScore->setText(getScoreStr());
        m_pTableItemScore->table()->updateCell(m_pTableItemScore->row(), m_pTableItemScore->col());
    }
}

int TrackInfoObject::getId()
{
    return m_iId;
}

void TrackInfoObject::setId(int iId)
{
    m_iId = iId;
}

TrackInfoObject *TrackInfoObject::getNext()
{
    if (m_pTableTrack)
    {
        WTrackTableItem *p = (WTrackTableItem *)m_pTableTrack->item(m_pTableItemArtist->row()+1,0);
        if (p)
            return p->getTrackInfoObject();
    }
    return 0;
}

