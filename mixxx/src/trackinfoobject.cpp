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
//Added by qt3to4:
#include <Q3ValueList>
#include <Q3MemArray>
#include <QMutexLocker>
#include <QtDebug>
#include "trackinfoobject.h"
// #include "bpm/bpmdetector.h"
#include "bpm/bpmscheme.h"
#include "bpm/bpmreceiver.h"
#include "soundsourceproxy.h"
//#include "wtracktable.h"
//#include "wtracktableitem.h"

#include "xmlparse.h"
#include <qdom.h>
#include "controlobject.h"

#ifdef QT3_SUPPORT
#include <QTime>
#endif

int TrackInfoObject::siMaxTimesPlayed = 1;

TrackInfoObject::TrackInfoObject(const QString sLocation)
	: m_sLocation(sLocation),
	m_chordData() {
    m_sArtist = "";
    m_sTitle = "";
    m_sType= "";
    m_sComment = "";
    m_sURL = "";
    m_iDuration = 0;
    m_iLength = 0;
    m_iBitrate = 0;
    m_iTimesPlayed = 0;
    m_fBpm = 0.;
    m_bBpmConfirm = false;
    m_bHeaderParsed = false;
    m_fBeatFirst = -1.;
    m_iScore = 0;
    m_iId = -1;
    m_pVisualWave = 0;
    m_pWave = 0;
    m_pSegmentation = 0;
    m_iSampleRate = 0;
    m_iChannels = 0;
    m_fCuePoint = 0.0f;

    m_dVisualResampleRate = 0;
    QFileInfo fileInfo(sLocation);
    m_sFilename = fileInfo.fileName();

    m_fBpmFactors = (float *)malloc(sizeof(float) * NumBpmFactors);
    generateBpmFactors();

    //m_pTableItemScore = 0;
    //m_pTableItemTitle = 0;
    //m_pTableItemArtist = 0;
    //m_pTableItemComment = 0;
    //m_pTableItemType = 0;
    //m_pTableItemDuration = 0;
    //m_pTableItemBpm = 0;
    //m_pTableItemBitrate = 0;

    //m_pTableTrack = 0;

    //qDebug() << "new TrackInfoObject....";

    // Check that the file exists:
    checkFileExists();

    if (parse()==OK)
        m_bIsValid = true;
    else
        m_bIsValid = false;

    installEventFilter(this);
    iTemp = 0;

    //qDebug() << "done constructing  TrackInfoObject....";
}

TrackInfoObject::TrackInfoObject(const QDomNode &nodeHeader)
	: m_chordData() {
    m_sFilename = XmlParse::selectNodeQString(nodeHeader, "Filename");
    //m_sLocation = XmlParse::selectNodeQString(nodeHeader, "Filepath");
    m_sTitle = XmlParse::selectNodeQString(nodeHeader, "Title");
    m_sArtist = XmlParse::selectNodeQString(nodeHeader, "Artist");
    m_sType = XmlParse::selectNodeQString(nodeHeader, "Type");
    m_sComment = XmlParse::selectNodeQString(nodeHeader, "Comment");
    m_iDuration = XmlParse::selectNodeQString(nodeHeader, "Duration").toInt();
    m_iSampleRate = XmlParse::selectNodeQString(nodeHeader, "SampleRate").toInt();
    m_iChannels = XmlParse::selectNodeQString(nodeHeader, "Channels").toInt();
    m_iBitrate = XmlParse::selectNodeQString(nodeHeader, "Bitrate").toInt();
    m_iLength = XmlParse::selectNodeQString(nodeHeader, "Length").toInt();
    m_iTimesPlayed = XmlParse::selectNodeQString(nodeHeader, "TimesPlayed").toInt();
    m_fBpm = XmlParse::selectNodeQString(nodeHeader, "Bpm").toFloat();
    m_bBpmConfirm = XmlParse::selectNodeQString(nodeHeader, "BpmConfirm").toInt();
    m_fBeatFirst = XmlParse::selectNodeQString(nodeHeader, "BeatFirst").toFloat();
    m_bHeaderParsed = false;
    m_iScore = 0;
    m_iId = XmlParse::selectNodeQString(nodeHeader, "Id").toInt();
    m_fCuePoint = XmlParse::selectNodeQString(nodeHeader, "CuePoint").toFloat();

    m_fBpmFactors = (float *)malloc(sizeof(float) * NumBpmFactors);
    generateBpmFactors();

    m_pVisualWave = 0;
    m_dVisualResampleRate = 0;

    //m_pWave = XmlParse::selectNodeHexCharArray(nodeHeader, QString("WaveSummaryHex"));

    m_pSegmentation = XmlParse::selectNodeLongList(nodeHeader, QString("SegmentationSummary"));
    //m_pTableTrack = 0;

    //m_pTableItemScore = 0;
    //m_pTableItemTitle = 0;
    //m_pTableItemArtist = 0;
    //m_pTableItemComment = 0;
    //m_pTableItemType = 0;
    //m_pTableItemDuration = 0;
    //m_pTableItemBpm = 0;
    //m_pTableItemBitrate = 0;

    m_bIsValid = true;

    if (m_iTimesPlayed>siMaxTimesPlayed)
        siMaxTimesPlayed = m_iTimesPlayed;

    // Check that the actual file exists:
    checkFileExists();

    installEventFilter(this);
}

TrackInfoObject::~TrackInfoObject()
{
    //Update this track in the library. (Saves stuff.)



    //removeFromTrackTable();
    delete m_fBpmFactors;
}

bool TrackInfoObject::isValid() const
{
    return m_bIsValid;
}

bool TrackInfoObject::checkFileExists()
{
    QFile fileTrack(getLocation());
    if (fileTrack.exists())
    {
        //m_qMutex.lock();
        m_bExists = true;
        //qDebug() << "Track exists...";
        //m_qMutex.unlock();
    }
    else
    {
        //m_qMutex.lock();
        m_bExists = false;
        //qDebug() << "The track %s was not found" << getLocation();
        //m_qMutex.unlock();
    }
    return m_bExists;
}

/*
    Writes information about the track to the xml file:
 */
void TrackInfoObject::writeToXML( QDomDocument &doc, QDomElement &header )
{
    m_qMutex.lock();

    XmlParse::addElement( doc, header, "Filename", m_sFilename );
    //XmlParse::addElement( doc, header, "Filepath", m_sFilepath );
    XmlParse::addElement( doc, header, "Title", m_sTitle );
    XmlParse::addElement( doc, header, "Artist", m_sArtist );
    XmlParse::addElement( doc, header, "Type", m_sType );
    XmlParse::addElement( doc, header, "Comment", m_sComment);
    XmlParse::addElement( doc, header, "Duration", QString("%1").arg(m_iDuration));
    XmlParse::addElement( doc, header, "SampleRate", QString("%1").arg(m_iSampleRate));
    XmlParse::addElement( doc, header, "Channels", QString("%1").arg(m_iChannels));
    XmlParse::addElement( doc, header, "Bitrate", QString("%1").arg(m_iBitrate));
    XmlParse::addElement( doc, header, "Length", QString("%1").arg(m_iLength) );
    XmlParse::addElement( doc, header, "TimesPlayed", QString("%1").arg(m_iTimesPlayed) );
    XmlParse::addElement( doc, header, "Bpm", QString("%1").arg(m_fBpm) );
    XmlParse::addElement( doc, header, "BpmConfirm", QString("%1").arg(m_bBpmConfirm) );
    XmlParse::addElement( doc, header, "BeatFirst", QString("%1").arg(m_fBeatFirst) );
    XmlParse::addElement( doc, header, "Id", QString("%1").arg(m_iId) );
    XmlParse::addElement( doc, header, "CuePoint", QString::number(m_fCuePoint) );
    //if (m_pWave) {
        //XmlParse::addHexElement(doc, header, "WaveSummaryHex", m_pWave);
    //}
    if (m_pSegmentation)
        XmlParse::addElement(doc, header, "SegmentationSummary", m_pSegmentation);

    m_qMutex.unlock();
}

/*
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
        m_pTableItemScore = new WTrackTableItem(this, pTableTrack,Q3TableItem::Never, getScoreStr(), typeNumber);
    if (!m_pTableItemTitle)
        m_pTableItemTitle = new WTrackTableItem(this, pTableTrack,Q3TableItem::Never, getTitle(), typeText);
    if (!m_pTableItemArtist)
        m_pTableItemArtist = new WTrackTableItem(this, pTableTrack,Q3TableItem::Never, getArtist(), typeText);
    if (!m_pTableItemComment)
        m_pTableItemComment = new WTrackTableItem(this, pTableTrack,Q3TableItem::WhenCurrent, getComment(), typeText);
    if (!m_pTableItemType)
        m_pTableItemType = new WTrackTableItem(this, pTableTrack,Q3TableItem::Never, getType(), typeText);
    if (!m_pTableItemDuration)
        m_pTableItemDuration = new WTrackTableItem(this, pTableTrack,Q3TableItem::Never, getDurationStr(), typeDuration);
    if (!m_pTableItemBpm)
                        m_pTableItemBpm = new WTrackTableItem(this, pTableTrack,Q3TableItem::Never, getBpmStr(), typeNumber); // Force use of BPM tapper dialog

                //m_pTableItemBpm = new WTrackTableItem(this, pTableTrack,Q3TableItem::WhenCurrent, getBpmStr(), typeNumber); // Use old-style keyboard entry for BPM

    if (!m_pTableItemBitrate)
        m_pTableItemBitrate = new WTrackTableItem(this, pTableTrack,Q3TableItem::Never, getBitrateStr(), typeNumber);

    //qDebug() << "inserting.. " << pTableTrack->item(iRow;

    // Insert the elements into the table
    pTableTrack->setItem(iRow, WTrackTableModel::SCORE, m_pTableItemScore);
    pTableTrack->setItem(iRow, WTrackTableModel::TITLE, m_pTableItemTitle);
    pTableTrack->setItem(iRow, WTrackTableModel::ARTIST, m_pTableItemArtist);
    pTableTrack->setItem(iRow, WTrackTableModel::COMMENT, m_pTableItemComment);
    pTableTrack->setItem(iRow, WTrackTableModel::TYPE, m_pTableItemType);
    pTableTrack->setItem(iRow, WTrackTableModel::DURATION, m_pTableItemDuration);
    pTableTrack->setItem(iRow, WTrackTableModel::BPM, m_pTableItemBpm);
    pTableTrack->setItem(iRow, WTrackTableModel::BITRATE, m_pTableItemBitrate);

    m_pTableTrack = pTableTrack;

   }

   void TrackInfoObject::removeFromTrackTable()
   {
    if (m_pTableTrack)
    {
        // Remove the row from the table, and delete the table items
        int row = m_pTableTrack->currentRow();
                //qDebug() << "remove from row " << row;
        m_pTableTrack->removeRow(m_pTableItemScore->row());

        // Set a new active row
        if (row < m_pTableTrack->numRows())
            m_pTableTrack->setCurrentCell(row, 0);
        else if (m_pTableTrack->numRows())
            m_pTableTrack->setCurrentCell(m_pTableTrack->numRows()-1, 0);

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
   void TrackInfoObject::clearTrackTableRow()
   {
        if (m_pTableTrack)
    {
        // Remove all contents of first row
        //int row = m_pTableTrack->currentRow();
                //qDebug() << "remove from row " << row;
        m_pTableTrack->removeRow(1);//m_pTableItemScore->row());

        // Set a new active row
        if (1 < m_pTableTrack->numRows())
            m_pTableTrack->setCurrentCell(1, 0);
        else if (m_pTableTrack->numRows())
            m_pTableTrack->setCurrentCell(m_pTableTrack->numRows()-1, 0);

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
   }*/
int TrackInfoObject::parse()
{
    // Add basic information derived from the filename:
    parseFilename();

    // Parse the using information stored in the sound file
    return SoundSourceProxy::ParseHeader(this);
}


void TrackInfoObject::parseFilename()
{
    m_qMutex.lock();

    if (m_sFilename.indexOf('-') != -1)
    {
        m_sArtist = m_sFilename.section('-',0,0).trimmed(); // Get the first part
        m_sTitle = m_sFilename.section('-',1,1); // Get the second part
        m_sTitle = m_sTitle.section('.',0,-2).trimmed(); // Remove the ending
    }
    else
    {
        m_sTitle = m_sFilename.section('.',0,-2).trimmed(); // Remove the ending;
        m_sType = m_sFilename.section('.',-1).trimmed(); // Get the ending
    }

    if (m_sTitle.length() == 0) {
        m_sTitle = m_sFilename.section('.',0,-2).trimmed();
    }

    // Find the length:
    m_iLength = QFileInfo(m_sLocation).size();

    // Add no comment
    m_sComment = QString("");

    // Find the type
    m_sType = m_sFilename.section(".",-1).toLower().trimmed();

    m_qMutex.unlock();
}

void TrackInfoObject::setLength(int bytes)
{
	m_iLength = bytes;
}

QString TrackInfoObject::getDurationStr() const
{
    m_qMutex.lock();
    int iDuration = m_iDuration;
    m_qMutex.unlock();

    if (iDuration <=0)
        return QString("?");
    else
    {
#if 0
        int iHours = iDuration/3600;
        int iMinutes = (iDuration - 3600*iHours)/60;
        int iSeconds = iDuration%60;

        // Sort out obviously wrong results:
        if (iHours > 5)
            return QString("??");
        if (iHours >= 1)
            return QString().sprintf("%d:%02d:%02d", iHours, iMinutes, iSeconds);
        else
            return QString().sprintf("%d:%02d", iMinutes, iSeconds);
#else
        QTime t = QTime().addSecs(iDuration);
        if (t.hour() > 5)
            return QString("??");

        if (t.hour() >= 1)
            return t.toString("h:mm:ss");
        else
            return t.toString("m:ss");
#endif
    }
}

void TrackInfoObject::setLocation(QString location)
{
	m_sLocation = location;
	QFileInfo fileInfo(location);
	m_sFilename = fileInfo.fileName();
}

QString TrackInfoObject::getLocation() const
{
    return m_sLocation;
}

float TrackInfoObject::getBpm() const
{
    m_qMutex.lock();
    float fBpm = m_fBpm;
    m_qMutex.unlock();

    return fBpm;
}

void TrackInfoObject::setBpm(float f)
{
    m_qMutex.lock();
    m_fBpm = f;
    m_qMutex.unlock();

    generateBpmFactors();

    //Tell the GUI to update the bpm label...
    qDebug() << "TrackInfoObject: emitting bpmUpdated signal!";
    emit(bpmUpdated(f));
}

void TrackInfoObject::generateBpmFactors()
{
    m_qMutex.lock();
    for(int i = 0; i < NumBpmFactors; i++)
    {
        m_fBpmFactors[i] = m_fBpm * Factors[i];
    }
    m_qMutex.unlock();
}

void TrackInfoObject::getBpmFactors(float * f) const
{
    m_qMutex.lock();
    if(f)
    {
        for(int i = 0; i < NumBpmFactors; i++)
        {
            f[i] = m_fBpmFactors[i];
        }
    }

    m_qMutex.unlock();
}

QString TrackInfoObject::getBpmStr() const
{
    m_qMutex.lock();
    float fBpm = m_fBpm;
    m_qMutex.unlock();

    return QString("%1").arg(fBpm, 3,'f',1);
}

bool TrackInfoObject::getBpmConfirm()  const
{
    m_qMutex.lock();
    bool bBpmConfirm = m_bBpmConfirm;
    m_qMutex.unlock();

    return bBpmConfirm;
}

void TrackInfoObject::setBpmConfirm(bool confirm)
{
    m_qMutex.lock();
    m_bBpmConfirm = confirm;
    m_qMutex.unlock();
}

bool TrackInfoObject::getHeaderParsed()  const
{
    m_qMutex.lock();
    bool bParsed = m_bHeaderParsed;
    m_qMutex.unlock();

    return bParsed;
}

void TrackInfoObject::setHeaderParsed(bool parsed)
{
    m_qMutex.lock();
    m_bHeaderParsed = parsed;
    m_qMutex.unlock();
}

QString TrackInfoObject::getInfo()  const
{
    m_qMutex.lock();
    QString artist = m_sArtist.trimmed() == "" ? "" : m_sArtist + ", ";
    QString sInfo = artist + m_sTitle;
    m_qMutex.unlock();

    return sInfo;
}

int TrackInfoObject::getDuration()  const
{
    m_qMutex.lock();
    int iDuration = m_iDuration;
    m_qMutex.unlock();

    return iDuration;
}

void TrackInfoObject::setDuration(int i)
{
    m_qMutex.lock();
    m_iDuration = i;
    m_qMutex.unlock();
/*
    if (m_pTableItemDuration)
    {
        m_pTableItemDuration->setText(getDurationStr());
        m_pTableItemDuration->table()->updateCell(m_pTableItemDuration->row(), m_pTableItemDuration->col());
    }
 */
}

QString TrackInfoObject::getTitle()  const
{
    m_qMutex.lock();
    QString sTitle = m_sTitle;
    m_qMutex.unlock();

    return sTitle;
}

void TrackInfoObject::setTitle(QString s)
{
    m_qMutex.lock();
    m_sTitle = s.trimmed();
    m_qMutex.unlock();
/*
    if (m_pTableItemTitle)
    {
        m_pTableItemTitle->setText(s);
        m_pTableItemTitle->table()->updateCell(m_pTableItemTitle->row(), m_pTableItemTitle->col());
    }
 */
}

QString TrackInfoObject::getArtist()  const
{
    m_qMutex.lock();
    QString sArtist = m_sArtist;
    m_qMutex.unlock();

    return sArtist;
}

void TrackInfoObject::setArtist(QString s)
{
    m_qMutex.lock();
    m_sArtist = s.trimmed();
    m_qMutex.unlock();
/*
    if (m_pTableItemArtist)
    {
        m_pTableItemArtist->setText(s);
        m_pTableItemArtist->table()->updateCell(m_pTableItemArtist->row(), m_pTableItemArtist->col());
    }
 */
}

QString TrackInfoObject::getAlbum()  const
{
    m_qMutex.lock();
    QString sAlbum = m_sAlbum;
    m_qMutex.unlock();

    return sAlbum;
}

void TrackInfoObject::setAlbum(QString s)
{
    m_qMutex.lock();
    m_sAlbum = s.trimmed();
    m_qMutex.unlock();
}

QString TrackInfoObject::getYear()  const
{
    m_qMutex.lock();
    QString sYear = m_sYear;
    m_qMutex.unlock();

    return sYear;
}

void TrackInfoObject::setYear(QString s)
{
    m_qMutex.lock();
    m_sYear = s.trimmed();
    m_qMutex.unlock();
}

QString TrackInfoObject::getGenre()  const
{
    m_qMutex.lock();
    QString sGenre = m_sGenre;
    m_qMutex.unlock();

    return sGenre;
}

void TrackInfoObject::setGenre(QString s)
{
    m_qMutex.lock();
    m_sGenre = s.trimmed();
    m_qMutex.unlock();
}

QString TrackInfoObject::getTrackNumber()  const
{
    m_qMutex.lock();
    QString sTrackNumber = m_sTrackNumber;
    m_qMutex.unlock();

    return sTrackNumber;
}

void TrackInfoObject::setTrackNumber(QString s)
{
    m_qMutex.lock();
    m_sTrackNumber = s.trimmed();
    m_qMutex.unlock();
}

QString TrackInfoObject::getFilename()  const
{
    m_qMutex.lock();
    QString sFilename = m_sFilename;
    m_qMutex.unlock();

    return sFilename;
}

bool TrackInfoObject::exists()  const
{
    m_qMutex.lock();
    bool bExists = m_bExists;
    m_qMutex.unlock();

    return bExists;
}

int TrackInfoObject::getTimesPlayed()  const
{
    m_qMutex.lock();
    int iTimesPlayed = m_iTimesPlayed;
    m_qMutex.unlock();

    return iTimesPlayed;
}

void TrackInfoObject::incTimesPlayed()
{
    m_qMutex.lock();
    ++m_iTimesPlayed;
    if (m_iTimesPlayed>siMaxTimesPlayed)
        siMaxTimesPlayed = m_iTimesPlayed;
    m_qMutex.unlock();
}

QString TrackInfoObject::getComment() const
{
    m_qMutex.lock();
    QString sComment = m_sComment;
    m_qMutex.unlock();

    return sComment;
}

void TrackInfoObject::setComment(QString s)
{
    m_qMutex.lock();
    m_sComment = s;
    m_qMutex.unlock();
/*
    if (m_pTableItemComment)
    {
        m_pTableItemComment->setText(s);
        m_pTableItemComment->table()->updateCell(m_pTableItemComment->row(), m_pTableItemComment->col());
    }
 */
}

QString TrackInfoObject::getType() const
{
    m_qMutex.lock();
    QString sType = m_sType;
    m_qMutex.unlock();

    return sType;
}

void TrackInfoObject::setType(QString s)
{
    m_qMutex.lock();
    m_sType = s;
    m_qMutex.unlock();
/*
    if (m_pTableItemType)
    {
        m_pTableItemType->setText(s);
        m_pTableItemType->table()->updateCell(m_pTableItemType->row(), m_pTableItemType->col());
    }
 */
}

void TrackInfoObject::setSampleRate(int iSampleRate)
{
    m_qMutex.lock();
    m_iSampleRate = iSampleRate;
    m_qMutex.unlock();

}

int TrackInfoObject::getSampleRate() const
{
    m_qMutex.lock();
    int iSampleRate = m_iSampleRate;
    m_qMutex.unlock();

    return iSampleRate;
}

void TrackInfoObject::setChannels(int iChannels)
{
    m_qMutex.lock();
    m_iChannels = iChannels;
    m_qMutex.unlock();

}

int TrackInfoObject::getChannels() const
{
    m_qMutex.lock();
    int iChannels = m_iChannels;
    m_qMutex.unlock();

    return iChannels;
}

int TrackInfoObject::getLength() const
{
    m_qMutex.lock();
    int iLength = m_iLength;
    m_qMutex.unlock();

    return iLength;
}

int TrackInfoObject::getBitrate() const
{
    m_qMutex.lock();
    int iBitrate = m_iBitrate;
    m_qMutex.unlock();

    return iBitrate;
}

QString TrackInfoObject::getBitrateStr() const
{
    return QString("%1").arg(getBitrate());
}

void TrackInfoObject::setBitrate(int i)
{
    m_qMutex.lock();
    m_iBitrate = i;
    m_qMutex.unlock();
/*
    if (m_pTableItemBitrate)
    {
        m_pTableItemBitrate->setText(getBitrateStr());
        m_pTableItemBitrate->table()->updateCell(m_pTableItemBitrate->row(), m_pTableItemBitrate->col());
    }
 */
}

void TrackInfoObject::setBeatFirst(float fBeatFirstPos)
{
    m_qMutex.lock();
    m_fBeatFirst = fBeatFirstPos;
    m_qMutex.unlock();
}

float TrackInfoObject::getBeatFirst() const
{
    m_qMutex.lock();
    float fBeatFirst = m_fBeatFirst;
    m_qMutex.unlock();

    return fBeatFirst;
}

int TrackInfoObject::getScore() const
{
    m_qMutex.lock();
    int iScore = m_iScore;
    m_qMutex.unlock();

    return iScore;
}

QString TrackInfoObject::getScoreStr() const
{
    m_qMutex.lock();
    int iScore = m_iScore;
    m_qMutex.unlock();

    return QString("%1").arg(iScore);
}

void TrackInfoObject::updateScore()
{
    m_qMutex.lock();
    Q_ASSERT(siMaxTimesPlayed!=0);
    m_iScore = 99*m_iTimesPlayed/siMaxTimesPlayed;
    m_qMutex.unlock();
/*
    if (m_pTableItemScore)
    {
        m_pTableItemScore->setText(getScoreStr());
        m_pTableItemScore->table()->updateCell(m_pTableItemScore->row(), m_pTableItemScore->col());
    }
 */
}

int TrackInfoObject::getId() const
{
    m_qMutex.lock();
    int iId = m_iId;
    m_qMutex.unlock();

    return iId;
}

void TrackInfoObject::setId(int iId)
{
    m_qMutex.lock();
    m_iId = iId;
    m_qMutex.unlock();
}

QVector<float> * TrackInfoObject::getVisualWaveform() {
    m_qMutex.lock();
    QVector<float> *pVisualWaveform = m_pVisualWave;
    m_qMutex.unlock();
    return pVisualWaveform;
}

void TrackInfoObject::setVisualResampleRate(double dVisualResampleRate) {
    m_qMutex.lock();
    m_dVisualResampleRate = dVisualResampleRate;
    m_qMutex.unlock();
}

double TrackInfoObject::getVisualResampleRate() {
    double rate;
    m_qMutex.lock();
    rate = m_dVisualResampleRate;
    m_qMutex.unlock();
    return rate;
}

QByteArray *TrackInfoObject::getWaveSummary()
{
    m_qMutex.lock();
    QByteArray *pWaveSummary = m_pWave;
    m_qMutex.unlock();

    return pWaveSummary;
}

Q3ValueList<long> * TrackInfoObject::getSegmentationSummary()
{
    m_qMutex.lock();
    Q3ValueList<long> *pSegmentationSummary = m_pSegmentation;
    m_qMutex.unlock();

    return pSegmentationSummary;
}

void TrackInfoObject::setVisualWaveform(QVector<float> *pWave) {
    m_qMutex.lock();
    m_pVisualWave = pWave;
    m_qMutex.unlock();
}

void TrackInfoObject::setWaveSummary(QByteArray* pWave, Q3ValueList<long> * pSegmentation, bool updateUI)
{
    m_qMutex.lock();
    m_pWave = pWave;
    m_pSegmentation = pSegmentation;
    m_qMutex.unlock();

    //if (updateUI) setOverviewWidget(m_pOverviewWidget);
    emit(wavesummaryUpdated(this));
}


/** Set URL for track*/
void TrackInfoObject::setURL(QString url)
{
    m_sURL = url;
}

/** Get URL for track */
QString TrackInfoObject::getURL()
{
    return m_sURL;
}

void TrackInfoObject::setCuePoint(float cue)
{
    m_qMutex.lock();
    m_fCuePoint = cue;
    m_qMutex.unlock();
}

float TrackInfoObject::getCuePoint()
{
    return m_fCuePoint;
}

Cue* TrackInfoObject::addCue() {
    QMutexLocker lock(&m_qMutex);
    // TODO(XXX) when TIO's know their own id, replace -1 here.
    Cue* cue = new Cue(-1);
    m_cuePoints.push_back(cue);
    return cue;
}

void TrackInfoObject::removeCue(Cue* cue) {
    QMutexLocker lock(&m_qMutex);
    m_cuePoints.remove(cue);
}

const QList<Cue*>& TrackInfoObject::getCuePoints() {
    return m_cuePoints;
}

void TrackInfoObject::setCuePoints(QList<Cue*> cuePoints) {
    QMutexLocker lock(&m_qMutex);
    m_cuePoints = cuePoints;
}
