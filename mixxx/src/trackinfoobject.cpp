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

#include <QDomNode>
#include <QDomDocument>
#include <QDomElement>
#include <QFileInfo>
#include <QMutexLocker>
#include <QString>
#include <QtDebug>
#include <QTime>

#include "trackinfoobject.h"

#include "bpm/bpmscheme.h"
#include "bpm/bpmreceiver.h"
#include "soundsourceproxy.h"
#include "xmlparse.h"
#include "controlobject.h"


TrackInfoObject::TrackInfoObject(const QString sLocation)
        : m_sLocation(sLocation),
          m_qMutex(QMutex::Recursive),
          m_chordData() {
    m_bDirty = false;
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
    m_iId = -1;
    m_pVisualWave = 0;
    m_iSampleRate = 0;
    m_iChannels = 0;
    m_fCuePoint = 0.0f;

    m_dVisualResampleRate = 0;
    QFileInfo fileInfo(sLocation);
    m_sFilename = fileInfo.fileName();

    //qDebug() << "new TrackInfoObject....";

    // Check that the file exists:
    checkFileExists();

    if (parse() == OK)
        m_bIsValid = true;
    else
        m_bIsValid = false;

    installEventFilter(this);

    //qDebug() << "done constructing  TrackInfoObject....";
}

TrackInfoObject::TrackInfoObject(const QDomNode &nodeHeader)
        : m_chordData(),
          m_qMutex(QMutex::Recursive) {

    m_sFilename = XmlParse::selectNodeQString(nodeHeader, "Filename");
    m_sLocation = XmlParse::selectNodeQString(nodeHeader, "Filepath") + "/" +  m_sFilename;
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
    m_iId = XmlParse::selectNodeQString(nodeHeader, "Id").toInt();
    m_fCuePoint = XmlParse::selectNodeQString(nodeHeader, "CuePoint").toFloat();

    m_pVisualWave = 0;
    m_dVisualResampleRate = 0;

    //m_pWave = XmlParse::selectNodeHexCharArray(nodeHeader, QString("WaveSummaryHex"));

    m_bIsValid = true;

    // Check that the actual file exists:
    checkFileExists();

    installEventFilter(this);

    m_bDirty = false;
}

TrackInfoObject::~TrackInfoObject() {
}

bool TrackInfoObject::isValid() const {
    QMutexLocker lock(&m_qMutex);
    return m_bIsValid;
}

bool TrackInfoObject::checkFileExists() {
    QFile fileTrack(getLocation());
    QMutexLocker lock(&m_qMutex);
    if (fileTrack.exists())
    {
        m_bExists = true;
        //qDebug() << "Track exists...";
    }
    else
    {

        m_bExists = false;
        //qDebug() << "The track %s was not found" << getLocation();
    }
    return m_bExists;
}

/*
    Writes information about the track to the xml file:
 */
void TrackInfoObject::writeToXML( QDomDocument &doc, QDomElement &header )
{
    QMutexLocker lock(&m_qMutex);

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

}

int TrackInfoObject::parse()
{
    // Add basic information derived from the filename:
    parseFilename();

    // Parse the using information stored in the sound file
    return SoundSourceProxy::ParseHeader(this);
}


void TrackInfoObject::parseFilename()
{
    QMutexLocker lock(&m_qMutex);

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
    setDirty(true);
}

void TrackInfoObject::setLength(int bytes)
{
    QMutexLocker lock(&m_qMutex);
    bool dirty = m_iLength != bytes;
    m_iLength = bytes;
    if (dirty)
        setDirty(true);
}

QString TrackInfoObject::getDurationStr() const
{
    QMutexLocker lock(&m_qMutex);
    int iDuration = m_iDuration;
    lock.unlock();

    // TODO(XXX) This whole thing should be pulled out into a generate MixxxUtil
    // class. The library has similar code littered throughout it.

    if (iDuration <=0) {
        return QString("?");
    } else {
        QTime t = QTime().addSecs(iDuration);
        if (t.hour() > 5)
            return QString("??");
        else if (t.hour() >= 1)
            return t.toString("h:mm:ss");
        else
            return t.toString("m:ss");
    }
}

void TrackInfoObject::setLocation(QString location)
{
    QMutexLocker lock(&m_qMutex);
    QFileInfo fileInfo(location);
    QString fileName = fileInfo.fileName();
    bool dirty = m_sLocation != location || fileName != m_sFilename;
    m_sLocation = location;
    m_sFilename = fileInfo.fileName();
    if (dirty)
        setDirty(true);
}

QString TrackInfoObject::getLocation() const
{
    QMutexLocker lock(&m_qMutex);
    return m_sLocation;
}

float TrackInfoObject::getBpm() const
{
    QMutexLocker lock(&m_qMutex);
    return m_fBpm;
}

void TrackInfoObject::setBpm(float f)
{
    QMutexLocker lock(&m_qMutex);
    bool dirty = m_fBpm != f;
    m_fBpm = f;
    if (dirty)
        setDirty(true);
    lock.unlock();

    //Tell the GUI to update the bpm label...
    qDebug() << "TrackInfoObject: emitting bpmUpdated signal!";
    emit(bpmUpdated(f));
}

QString TrackInfoObject::getBpmStr() const
{
    return QString("%1").arg(getBpm(), 3,'f',1);
}

bool TrackInfoObject::getBpmConfirm()  const
{
    QMutexLocker lock(&m_qMutex);
    return m_bBpmConfirm;
}

void TrackInfoObject::setBpmConfirm(bool confirm)
{
    QMutexLocker lock(&m_qMutex);
    m_bBpmConfirm = confirm;
}

bool TrackInfoObject::getHeaderParsed()  const
{
    QMutexLocker lock(&m_qMutex);
    return m_bHeaderParsed;
}

void TrackInfoObject::setHeaderParsed(bool parsed)
{
    QMutexLocker lock(&m_qMutex);
    m_bHeaderParsed = parsed;
}

QString TrackInfoObject::getInfo()  const
{
    QMutexLocker lock(&m_qMutex);
    QString artist = m_sArtist.trimmed() == "" ? "" : m_sArtist + ", ";
    QString sInfo = artist + m_sTitle;
    return sInfo;
}

int TrackInfoObject::getDuration()  const
{
    QMutexLocker lock(&m_qMutex);
    return m_iDuration;
}

void TrackInfoObject::setDuration(int i)
{
    QMutexLocker lock(&m_qMutex);
    bool dirty = m_iDuration != i;
    m_iDuration = i;
    if (dirty)
        setDirty(true);
}

QString TrackInfoObject::getTitle()  const
{
    QMutexLocker lock(&m_qMutex);
    return m_sTitle;
}

void TrackInfoObject::setTitle(QString s)
{
    QMutexLocker lock(&m_qMutex);
    s = s.trimmed();
    bool dirty = m_sTitle != s;
    m_sTitle = s;
    if (dirty)
        setDirty(true);
}

QString TrackInfoObject::getArtist()  const
{
    QMutexLocker lock(&m_qMutex);
    return m_sArtist;
}

void TrackInfoObject::setArtist(QString s)
{
    QMutexLocker lock(&m_qMutex);
    s = s.trimmed();
    bool dirty = m_sArtist != s;
    m_sArtist = s;
    if (dirty)
        setDirty(true);
}

QString TrackInfoObject::getAlbum()  const
{
    QMutexLocker lock(&m_qMutex);
    return m_sAlbum;
}

void TrackInfoObject::setAlbum(QString s)
{
    QMutexLocker lock(&m_qMutex);
    s = s.trimmed();
    bool dirty = m_sAlbum != s;
    m_sAlbum = s;
    if (dirty)
        setDirty(true);
}

QString TrackInfoObject::getYear()  const
{
    QMutexLocker lock(&m_qMutex);
    return m_sYear;
}

void TrackInfoObject::setYear(QString s)
{
    QMutexLocker lock(&m_qMutex);
    s = s.trimmed();
    bool dirty = m_sYear != s;
    m_sYear = s.trimmed();
    if (dirty)
        setDirty(true);
}

QString TrackInfoObject::getGenre()  const
{
    QMutexLocker lock(&m_qMutex);
    return m_sGenre;
}

void TrackInfoObject::setGenre(QString s)
{
    QMutexLocker lock(&m_qMutex);
    s = s.trimmed();
    bool dirty = m_sGenre != s;
    m_sGenre = s;
    if (dirty)
        setDirty(true);
}

QString TrackInfoObject::getTrackNumber()  const
{
    QMutexLocker lock(&m_qMutex);
    return m_sTrackNumber;
}

void TrackInfoObject::setTrackNumber(QString s)
{
    QMutexLocker lock(&m_qMutex);
    s = s.trimmed();
    bool dirty = m_sTrackNumber != s;
    m_sTrackNumber = s;
    if (dirty)
        setDirty(true);
}

QString TrackInfoObject::getFilename()  const
{
    QMutexLocker lock(&m_qMutex);
    return m_sFilename;
}

bool TrackInfoObject::exists()  const
{
    QMutexLocker lock(&m_qMutex);
    return m_bExists;
}

int TrackInfoObject::getTimesPlayed()  const
{
    QMutexLocker lock(&m_qMutex);
    return m_iTimesPlayed;
}

void TrackInfoObject::incTimesPlayed()
{
    QMutexLocker lock(&m_qMutex);
    ++m_iTimesPlayed;
    setDirty(true);
}

QString TrackInfoObject::getComment() const
{
    QMutexLocker lock(&m_qMutex);
    return m_sComment;
}

void TrackInfoObject::setComment(QString s)
{
    QMutexLocker lock(&m_qMutex);
    bool dirty = s != m_sComment;
    m_sComment = s;
    if (dirty)
        setDirty(true);
}

QString TrackInfoObject::getType() const
{
    QMutexLocker lock(&m_qMutex);
    return m_sType;
}

void TrackInfoObject::setType(QString s)
{
    QMutexLocker lock(&m_qMutex);
    bool dirty = s != m_sType;
    m_sType = s;
    if (dirty)
        setDirty(true);
}

void TrackInfoObject::setSampleRate(int iSampleRate)
{
    QMutexLocker lock(&m_qMutex);
    bool dirty = m_iSampleRate != iSampleRate;
    m_iSampleRate = iSampleRate;
    if (dirty)
        setDirty(true);
}

int TrackInfoObject::getSampleRate() const
{
    QMutexLocker lock(&m_qMutex);
    return m_iSampleRate;
}

void TrackInfoObject::setChannels(int iChannels)
{
    QMutexLocker lock(&m_qMutex);
    bool dirty = m_iChannels != iChannels;
    m_iChannels = iChannels;
    if (dirty)
        setDirty(true);
}

int TrackInfoObject::getChannels() const
{
    QMutexLocker lock(&m_qMutex);
    return m_iChannels;
}

int TrackInfoObject::getLength() const
{
    QMutexLocker lock(&m_qMutex);
    return m_iLength;
}

int TrackInfoObject::getBitrate() const
{
    QMutexLocker lock(&m_qMutex);
    return m_iBitrate;
}

QString TrackInfoObject::getBitrateStr() const
{
    return QString("%1").arg(getBitrate());
}

void TrackInfoObject::setBitrate(int i)
{
    QMutexLocker lock(&m_qMutex);
    bool dirty = m_iBitrate != i;
    m_iBitrate = i;
    if (dirty)
        setDirty(true);
}

void TrackInfoObject::setBeatFirst(float fBeatFirstPos)
{
    QMutexLocker lock(&m_qMutex);
    bool dirty = m_fBeatFirst != fBeatFirstPos;
    m_fBeatFirst = fBeatFirstPos;
    if (dirty)
        setDirty(true);
}

float TrackInfoObject::getBeatFirst() const
{
    QMutexLocker lock(&m_qMutex);
    return m_fBeatFirst;
}

int TrackInfoObject::getId() const {
    QMutexLocker lock(&m_qMutex);
    return m_iId;
}

void TrackInfoObject::setId(int iId) {
    QMutexLocker lock(&m_qMutex);
    bool dirty = m_iId != iId;
    m_iId = iId;
    if (dirty)
        setDirty(true);
}

QVector<float> * TrackInfoObject::getVisualWaveform() {
    QMutexLocker lock(&m_qMutex);
    return m_pVisualWave;
}

void TrackInfoObject::setVisualResampleRate(double dVisualResampleRate) {
    // Temporary, shared value that should not be saved. The only reason it
    // exists on the TIO is a temporary hack, so it does not dirty the TIO.
    QMutexLocker lock(&m_qMutex);
    m_dVisualResampleRate = dVisualResampleRate;
}

double TrackInfoObject::getVisualResampleRate() {
    QMutexLocker lock(&m_qMutex);
    return m_dVisualResampleRate;
}

const QByteArray *TrackInfoObject::getWaveSummary()
{
    QMutexLocker lock(&m_qMutex);
    return &m_waveSummary;
}

void TrackInfoObject::setVisualWaveform(QVector<float> *pWave) {
    // The visual waveform is not serialized currently so it does not dirty a
    // TIO.
    QMutexLocker lock(&m_qMutex);
    m_pVisualWave = pWave;
}

void TrackInfoObject::setWaveSummary(const QByteArray* pWave, bool updateUI)
{
    QMutexLocker lock(&m_qMutex);
    m_waveSummary = *pWave; //_Copy_ the bytes
    setDirty(true);
    lock.unlock();
    emit(wavesummaryUpdated(this));
}

void TrackInfoObject::setURL(QString url)
{
    QMutexLocker lock(&m_qMutex);
    bool dirty = m_sURL != url;
    m_sURL = url;
    if (dirty)
        setDirty(true);
}

QString TrackInfoObject::getURL()
{
    QMutexLocker lock(&m_qMutex);
    return m_sURL;
}

void TrackInfoObject::setCuePoint(float cue)
{
    QMutexLocker lock(&m_qMutex);
    bool dirty = m_fCuePoint != cue;
    m_fCuePoint = cue;
    if (dirty)
        setDirty(true);
}

float TrackInfoObject::getCuePoint()
{
    QMutexLocker lock(&m_qMutex);
    return m_fCuePoint;
}

void TrackInfoObject::slotCueUpdated() {
    setDirty(true);
    emit(cuesUpdated());
}

Cue* TrackInfoObject::addCue() {
    qDebug() << "TrackInfoObject::addCue()";
    QMutexLocker lock(&m_qMutex);
    Cue* cue = new Cue(m_iId);
    connect(cue, SIGNAL(updated()),
            this, SLOT(slotCueUpdated()));
    m_cuePoints.push_back(cue);
    setDirty(true);
    lock.unlock();
    emit(cuesUpdated());
    return cue;
}

void TrackInfoObject::removeCue(Cue* cue) {
    QMutexLocker lock(&m_qMutex);
    disconnect(cue, 0, this, 0);
    m_cuePoints.remove(cue);
    setDirty(true);
    lock.unlock();
    emit(cuesUpdated());
}

const QList<Cue*>& TrackInfoObject::getCuePoints() {
    QMutexLocker lock(&m_qMutex);
    return m_cuePoints;
}

void TrackInfoObject::setCuePoints(QList<Cue*> cuePoints) {
    qDebug() << "setCuePoints" << cuePoints.length();
    QMutexLocker lock(&m_qMutex);
    QListIterator<Cue*> it(m_cuePoints);
    while (it.hasNext()) {
        Cue* cue = it.next();
        disconnect(cue, 0, this, 0);
    }
    m_cuePoints = cuePoints;
    it = QListIterator<Cue*>(m_cuePoints);
    while (it.hasNext()) {
        Cue* cue = it.next();
        connect(cue, SIGNAL(updated()),
            this, SLOT(slotCueUpdated()));
    }
    setDirty(true);
    lock.unlock();
    emit(cuesUpdated());
}

const Segmentation<QString>* TrackInfoObject::getChordData() {
    QMutexLocker lock(&m_qMutex);
    return &m_chordData;
}

void TrackInfoObject::setChordData(Segmentation<QString> cd) {
    QMutexLocker lock(&m_qMutex);
    m_chordData = cd;
    setDirty(true);
}

void TrackInfoObject::setDirty(bool bDirty) {
    QMutexLocker lock(&m_qMutex);
    bool change = m_bDirty != bDirty;
    m_bDirty = bDirty;
    lock.unlock();
    if (change) {
        qDebug() << "Track" << m_iId << "set" << (bDirty ? "dirty" : "clean");
        if (m_bDirty)
            emit(dirty());
        else
            emit(clean());
    }
    // Emit a changed signal regardless if this attempted to set us dirty.
    if (bDirty)
        emit(changed());


    //qDebug() << QString("TrackInfoObject %1 %2 set to %3").arg(m_iId).arg(m_sLocation).arg(m_bDirty ? "dirty" : "clean");
}

bool TrackInfoObject::isDirty() {
    QMutexLocker lock(&m_qMutex);
    return m_bDirty;
}
