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

#include "trackinfoobject.h"

#include "soundsourceproxy.h"
#include "xmlparse.h"
#include "controlobject.h"
#include "waveform/waveform.h"
#include "track/beatfactory.h"

#include "mixxxutils.cpp"

TrackInfoObject::TrackInfoObject(const QString sLocation, bool parseHeader)
        : m_qMutex(QMutex::Recursive) {
    QFileInfo fileInfo(sLocation);
    populateLocation(fileInfo);
    initialize(parseHeader);
    m_waveform = new Waveform;
    m_waveformSummary = new Waveform;
}

TrackInfoObject::TrackInfoObject(const QFileInfo& fileInfo, bool parseHeader)
        : m_qMutex(QMutex::Recursive) {
    populateLocation(fileInfo);
    initialize(parseHeader);
    m_waveform = new Waveform;
    m_waveformSummary = new Waveform;
}

TrackInfoObject::TrackInfoObject(const QDomNode &nodeHeader)
        : m_qMutex(QMutex::Recursive) {
    m_sFilename = XmlParse::selectNodeQString(nodeHeader, "Filename");
    m_sLocation = XmlParse::selectNodeQString(nodeHeader, "Filepath") + "/" +  m_sFilename;
    QString create_date;

    // We don't call initialize() here because it would end up calling parse()
    // on the file. Plus those initializations weren't done before, so it might
    // cause subtle bugs. This constructor is only used for legacy importing so
    // I'm not going to do it. rryan 6/2010

    // Check the status of the file on disk.
    QFileInfo fileInfo(m_sLocation);
    populateLocation(fileInfo);

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
    m_fReplayGain = XmlParse::selectNodeQString(nodeHeader, "replaygain").toFloat();
    m_bHeaderParsed = false;
    create_date = XmlParse::selectNodeQString(nodeHeader, "CreateDate");
    if (create_date == "")
        m_dCreateDate = fileInfo.created();
    else
        m_dCreateDate = QDateTime::fromString(create_date);

    // Mixxx <1.8 recorded track IDs in mixxxtrack.xml, but we are going to
    // ignore those. Tracks will get a new ID from the database.
    //m_iId = XmlParse::selectNodeQString(nodeHeader, "Id").toInt();
    m_iId = -1;

    m_fCuePoint = XmlParse::selectNodeQString(nodeHeader, "CuePoint").toFloat();
    m_bPlayed = false;

    //m_pWave = XmlParse::selectNodeHexCharArray(nodeHeader, QString("WaveSummaryHex"));

    m_bIsValid = true;

    m_bDirty = false;
    m_bLocationChanged = false;

    m_waveform = new Waveform;
    m_waveformSummary = new Waveform;
}

void TrackInfoObject::populateLocation(const QFileInfo& fileInfo) {
    m_sFilename = fileInfo.fileName();
    m_sLocation = fileInfo.absoluteFilePath();
    m_sDirectory = fileInfo.absolutePath();
    m_iLength = fileInfo.size();
}

void TrackInfoObject::initialize(bool parseHeader) {
    m_bDirty = false;
    m_bLocationChanged = false;

    m_sArtist = "";
    m_sTitle = "";
    m_sType= "";
    m_sComment = "";
    m_sYear = "";
    m_sURL = "";
    m_iDuration = 0;
    m_iBitrate = 0;
    m_iTimesPlayed = 0;
    m_bPlayed = false;
    m_fReplayGain = 0.;
    m_bIsValid = false;
    m_bHeaderParsed = false;
    m_iId = -1;
    m_iSampleRate = 0;
    m_iChannels = 0;
    m_fCuePoint = 0.0f;
    m_dCreateDate = m_dateAdded = QDateTime::currentDateTime();
    m_Rating = 0;
    m_key = "";
    m_bBpmLock = false;

    // parse() parses the metadata from file. This is not a quick operation!
    if (parseHeader) {
        parse();
    }
}

TrackInfoObject::~TrackInfoObject() {
    //qDebug() << "~TrackInfoObject()" << m_iId << getInfo();
    delete m_waveform;
    delete m_waveformSummary;
}

void TrackInfoObject::doSave() {
    //qDebug() << "TIO::doSave()" << getInfo();
    emit(save(this));
}

bool TrackInfoObject::isValid() const {
    QMutexLocker lock(&m_qMutex);
    return m_bIsValid;
}

/*
    Writes information about the track to the xml file:
 */
void TrackInfoObject::writeToXML( QDomDocument &doc, QDomElement &header )
{
    QMutexLocker lock(&m_qMutex);

    QString create_date;
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
    XmlParse::addElement( doc, header, "replaygain", QString("%1").arg(m_fReplayGain) );
    XmlParse::addElement( doc, header, "Id", QString("%1").arg(m_iId) );
    XmlParse::addElement( doc, header, "CuePoint", QString::number(m_fCuePoint) );
    XmlParse::addElement( doc, header, "CreateDate", m_dCreateDate.toString() );
    //if (m_pWave) {
    //XmlParse::addHexElement(doc, header, "WaveSummaryHex", m_pWave);
    //}

}

int TrackInfoObject::parse()
{
    // Add basic information derived from the filename:
    parseFilename();

    // Parse the using information stored in the sound file
    bool result = SoundSourceProxy::ParseHeader(this);
    m_bIsValid = result == OK;
    return result;
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

    // Add no comment
    m_sComment = QString("");

    // Find the type
    m_sType = m_sFilename.section(".",-1).toLower().trimmed();
    setDirty(true);
}

QString TrackInfoObject::getDurationStr() const
{
    QMutexLocker lock(&m_qMutex);
    int iDuration = m_iDuration;
    lock.unlock();

    return MixxxUtils::secondsToMinutes(iDuration, true);
}

void TrackInfoObject::setLocation(QString location)
{
    QMutexLocker lock(&m_qMutex);
    QFileInfo fileInfo(location);
    // TODO(XXX) Can the file name change without m_sLocation changing?? The
    // extra test seems pointless.
    QString fileName = fileInfo.fileName();
    bool dirty = m_sLocation != location || fileName != m_sFilename;
    populateLocation(fileInfo);
    if (dirty) {
        m_bLocationChanged = true;
        setDirty(true);
    }
}

QString TrackInfoObject::getLocation() const
{
    QMutexLocker lock(&m_qMutex);
    return m_sLocation;
}

QString TrackInfoObject::getDirectory() const
{
    QMutexLocker lock(&m_qMutex);
    return m_sDirectory;
}

QString TrackInfoObject::getFilename()  const
{
    QMutexLocker lock(&m_qMutex);
    return m_sFilename;
}

QDateTime TrackInfoObject::getCreateDate() const
{
    QMutexLocker lock(&m_qMutex);
    QDateTime create_date = QDateTime(m_dCreateDate);
    return create_date;
}

bool TrackInfoObject::exists()  const
{
    QMutexLocker lock(&m_qMutex);
    // return here a fresh calculated value to be sure 
    // the file is not deleted or gone with an USB-Stick 
    // because it will probably stop the Auto-DJ
    return QFile::exists(m_sLocation);
}

float TrackInfoObject::getReplayGain() const
{
    QMutexLocker lock(&m_qMutex);
    return m_fReplayGain;
}

void TrackInfoObject::setReplayGain(float f)
{
    QMutexLocker lock(&m_qMutex);
    bool dirty = m_fReplayGain != f;
    m_fReplayGain = f;
    //qDebug() << "Reported ReplayGain value: " << m_fReplayGain;
    if (dirty)
        setDirty(true);
    lock.unlock();
    emit(ReplayGainUpdated(f));
}

float TrackInfoObject::getBpm() const {
    QMutexLocker lock(&m_qMutex);
    if (!m_pBeats) {
        return 0;
    }
    // getBpm() returns -1 when invalid.
    double bpm = m_pBeats->getBpm();
    if (bpm >= 0.0) {
        return bpm;
    }
    return 0;
}

void TrackInfoObject::setBpm(float f) {
    if (f < 0) {
        return;
    }

    QMutexLocker lock(&m_qMutex);
    // TODO(rryan): Assume always dirties.
    bool dirty = false;
    if (f == 0.0) {
        // If the user sets the BPM to 0, we assume they want to clear the
        // beatgrid.
        setBeats(BeatsPointer());
        dirty = true;
    } else if (!m_pBeats) {
        setBeats(BeatFactory::makeBeatGrid(this, f, 0));
        dirty = true;
    } else if (m_pBeats->getBpm() != f) {
        m_pBeats->setBpm(f);
        dirty = true;
    }

    if (dirty)
        setDirty(true);

    lock.unlock();
    // Tell the GUI to update the bpm label...
    //qDebug() << "TrackInfoObject signaling BPM update to" << f;
    emit(bpmUpdated(f));
}

QString TrackInfoObject::getBpmStr() const
{
    return QString("%1").arg(getBpm(), 3,'f',1);
}

void TrackInfoObject::setBeats(BeatsPointer pBeats) {
    QMutexLocker lock(&m_qMutex);

    // This whole method is not so great. The fact that Beats is an ABC is
    // limiting with respect to QObject and signals/slots.

    QObject* pObject = NULL;
    if (m_pBeats) {
        pObject = dynamic_cast<QObject*>(m_pBeats.data());
        if (pObject) {
            disconnect(pObject, SIGNAL(updated()),
                       this, SLOT(slotBeatsUpdated()));
        }
    }
    m_pBeats = pBeats;
    double bpm = 0.0;
    if (m_pBeats) {
        bpm = m_pBeats->getBpm();
        pObject = dynamic_cast<QObject*>(m_pBeats.data());
        Q_ASSERT(pObject);
        if (pObject) {
            connect(pObject, SIGNAL(updated()),
                    this, SLOT(slotBeatsUpdated()));
        }
    }
    setDirty(true);
    lock.unlock();
    emit(bpmUpdated(bpm));
    emit(beatsUpdated());
}

BeatsPointer TrackInfoObject::getBeats() const {
    QMutexLocker lock(&m_qMutex);
    return m_pBeats;
}

void TrackInfoObject::slotBeatsUpdated() {
    QMutexLocker lock(&m_qMutex);
    setDirty(true);
    double bpm = m_pBeats->getBpm();
    lock.unlock();
    emit(bpmUpdated(bpm));
    emit(beatsUpdated());
}

bool TrackInfoObject::getHeaderParsed()  const
{
    QMutexLocker lock(&m_qMutex);
    return m_bHeaderParsed;
}

void TrackInfoObject::setHeaderParsed(bool parsed)
{
    QMutexLocker lock(&m_qMutex);
    bool dirty = m_bHeaderParsed != parsed;
    m_bHeaderParsed = parsed;
    if (dirty)
        setDirty(true);
}

QString TrackInfoObject::getInfo()  const
{
    QMutexLocker lock(&m_qMutex);
    QString artist = m_sArtist.trimmed() == "" ? "" : m_sArtist + ", ";
    QString sInfo = artist + m_sTitle;
    return sInfo;
}

QDateTime TrackInfoObject::getDateAdded() const {
    QMutexLocker lock(&m_qMutex);
    return m_dateAdded;
}

void TrackInfoObject::setDateAdded(QDateTime dateAdded) {
    QMutexLocker lock(&m_qMutex);
    m_dateAdded = dateAdded;
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

QString TrackInfoObject::getComposer()  const
{
    QMutexLocker lock(&m_qMutex);
    return m_sComposer;
}

void TrackInfoObject::setComposer(QString s)
{
    QMutexLocker lock(&m_qMutex);
    s = s.trimmed();
    bool dirty = m_sComposer != s;
    m_sComposer = s;
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

int TrackInfoObject::getTimesPlayed()  const
{
    QMutexLocker lock(&m_qMutex);
    return m_iTimesPlayed;
}

void TrackInfoObject::setTimesPlayed(int t)
{
    QMutexLocker lock(&m_qMutex);
    bool dirty = t != m_iTimesPlayed;
    m_iTimesPlayed = t;
    if (dirty)
        setDirty(true);
}

void TrackInfoObject::incTimesPlayed()
{
    setPlayedAndUpdatePlaycount(true);
}

bool TrackInfoObject::getPlayed() const
{
    QMutexLocker lock(&m_qMutex);
    bool bPlayed = m_bPlayed;
    return bPlayed;
}

void TrackInfoObject::setPlayedAndUpdatePlaycount(bool bPlayed)
{
    QMutexLocker lock(&m_qMutex);
    if (bPlayed) {
        ++m_iTimesPlayed;
        setDirty(true);
    }
    else if (m_bPlayed && !bPlayed) {
        --m_iTimesPlayed;
        setDirty(true);
    }
    m_bPlayed = bPlayed;
}

void TrackInfoObject::setPlayed(bool bPlayed)
{
    QMutexLocker lock(&m_qMutex);
    if (bPlayed != m_bPlayed) {
        m_bPlayed = bPlayed;
        setDirty(true);
    }
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

int TrackInfoObject::getId() const {
    QMutexLocker lock(&m_qMutex);
    return m_iId;
}

void TrackInfoObject::setId(int iId) {
    QMutexLocker lock(&m_qMutex);
    // changing the Id does not make the track drity because the Id is always
    // generated by the Database itself
    m_iId = iId;
}


//TODO (vrince) remove clen-up when new summary is ready
/*
const QByteArray *TrackInfoObject::getWaveSummary()
{
    QMutexLocker lock(&m_qMutex);
    return &m_waveSummary;
}

void TrackInfoObject::setWaveSummary(const QByteArray* pWave, bool updateUI)
{
    QMutexLocker lock(&m_qMutex);
    m_waveSummary = *pWave; //_Copy_ the bytes
    setDirty(true);
    lock.unlock();
    emit(wavesummaryUpdated(this));
}*/

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

Waveform* TrackInfoObject::getWaveform() {
    QMutexLocker lock(&m_qMutex);
    return m_waveform;
}

const Waveform* TrackInfoObject::getWaveform() const {
    QMutexLocker lock(&m_qMutex);
    return m_waveform;
}

void TrackInfoObject::setWaveform(Waveform* pWaveform) {
    QMutexLocker lock(&m_qMutex);
    if (m_waveform) {
        delete m_waveform;
    }
    m_waveform = pWaveform;
    lock.unlock();
    emit(waveformUpdated());
}

Waveform* TrackInfoObject::getWaveformSummary() {
    QMutexLocker lock(&m_qMutex);
    return m_waveformSummary;
}

const Waveform* TrackInfoObject::getWaveformSummary() const {
    QMutexLocker lock(&m_qMutex);
    return m_waveformSummary;
}

void TrackInfoObject::setWaveformSummary(Waveform* pWaveformSummary) {
    QMutexLocker lock(&m_qMutex);
    if (m_waveformSummary) {
        delete m_waveformSummary;
    }
    m_waveformSummary = pWaveformSummary;
    lock.unlock();
    emit(waveformSummaryUpdated());
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
    //qDebug() << "TrackInfoObject::addCue()";
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
    m_cuePoints.removeOne(cue);
    setDirty(true);
    lock.unlock();
    emit(cuesUpdated());
}

const QList<Cue*>& TrackInfoObject::getCuePoints() {
    QMutexLocker lock(&m_qMutex);
    return m_cuePoints;
}

void TrackInfoObject::setCuePoints(QList<Cue*> cuePoints) {
    //qDebug() << "setCuePoints" << cuePoints.length();
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
    // qDebug() << "Track" << m_iId << getInfo() << (change? "changed" : "unchanged")
    //          << "set" << (bDirty ? "dirty" : "clean");
    if (change) {
        if (m_bDirty)
            emit(dirty(this));
        else
            emit(clean(this));
    }
    // Emit a changed signal regardless if this attempted to set us dirty.
    if (bDirty)
        emit(changed(this));

    //qDebug() << QString("TrackInfoObject %1 %2 set to %3").arg(QString::number(m_iId), m_sLocation, m_bDirty ? "dirty" : "clean");
}

bool TrackInfoObject::isDirty() {
    QMutexLocker lock(&m_qMutex);
    return m_bDirty;
}

bool TrackInfoObject::locationChanged() {
    QMutexLocker lock(&m_qMutex);
    return m_bLocationChanged;
}

int TrackInfoObject::getRating() const{
    QMutexLocker lock(&m_qMutex);
    return m_Rating;
}

void TrackInfoObject::setRating (int rating){
    QMutexLocker lock(&m_qMutex);
    bool dirty = rating != m_Rating;
    m_Rating = rating;
    if (dirty)
        setDirty(true);
}

QString TrackInfoObject::getKey() const{
    QMutexLocker lock(&m_qMutex);
    return m_key;
}

void TrackInfoObject::setKey(QString key){
    QMutexLocker lock(&m_qMutex);
    bool dirty = key != m_key;
    m_key = key;
    if (dirty)
        setDirty(true);
}

void TrackInfoObject::setBpmLock(bool bpmLock) {
    QMutexLocker lock(&m_qMutex);
    bool dirty = bpmLock != m_bBpmLock;
    m_bBpmLock = bpmLock;
    if (dirty)
        setDirty(true);
}

bool TrackInfoObject::hasBpmLock() const {
    QMutexLocker lock(&m_qMutex);
    return m_bBpmLock;
}
