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
#include <QDirIterator>
#include <QFile>
#include <QMutexLocker>
#include <QString>
#include <QtDebug>
#include <QRegExp>

#include "trackinfoobject.h"

#include "controlobject.h"
#include "soundsourceproxy.h"
#include "xmlparse.h"
#include "track/beatfactory.h"
#include "track/keyfactory.h"
#include "track/keyutils.h"
#include "util/compatibility.h"
#include "util/cmdlineargs.h"
#include "util/time.h"
#include "util/math.h"
#include "waveform/waveform.h"
#include "library/coverartutils.h"

TrackInfoObject::TrackInfoObject(const QString& file,
                                 SecurityTokenPointer pToken,
                                 bool parseHeader, bool parseCoverArt)
        : m_fileInfo(file),
          m_pSecurityToken(pToken.isNull() ? Sandbox::openSecurityToken(
                  m_fileInfo, true) : pToken),
          m_qMutex(QMutex::Recursive),
          m_analyserProgress(-1) {
    initialize(parseHeader, parseCoverArt);
}

TrackInfoObject::TrackInfoObject(const QFileInfo& fileInfo,
                                 SecurityTokenPointer pToken,
                                 bool parseHeader, bool parseCoverArt)
        : m_fileInfo(fileInfo),
          m_pSecurityToken(pToken.isNull() ? Sandbox::openSecurityToken(
                  m_fileInfo, true) : pToken),
          m_qMutex(QMutex::Recursive),
          m_analyserProgress(-1) {
    initialize(parseHeader, parseCoverArt);
}

TrackInfoObject::TrackInfoObject(const QDomNode &nodeHeader)
        : m_qMutex(QMutex::Recursive),
          m_analyserProgress(-1) {
    QString filename = XmlParse::selectNodeQString(nodeHeader, "Filename");
    QString location = XmlParse::selectNodeQString(nodeHeader, "Filepath") + "/" +  filename;
    m_fileInfo = QFileInfo(location);
    m_pSecurityToken = Sandbox::openSecurityToken(m_fileInfo, true);

    // We don't call initialize() here because it would end up calling parse()
    // on the file. Plus those initializations weren't done before, so it might
    // cause subtle bugs. This constructor is only used for legacy importing so
    // I'm not going to do it. rryan 6/2010

    m_sTitle = XmlParse::selectNodeQString(nodeHeader, "Title");
    m_sArtist = XmlParse::selectNodeQString(nodeHeader, "Artist");
    m_sType = XmlParse::selectNodeQString(nodeHeader, "Type");
    m_sComment = XmlParse::selectNodeQString(nodeHeader, "Comment");
    m_iDuration = XmlParse::selectNodeQString(nodeHeader, "Duration").toInt();
    m_iSampleRate = XmlParse::selectNodeQString(nodeHeader, "SampleRate").toInt();
    m_iChannels = XmlParse::selectNodeQString(nodeHeader, "Channels").toInt();
    m_iBitrate = XmlParse::selectNodeQString(nodeHeader, "Bitrate").toInt();
    m_iTimesPlayed = XmlParse::selectNodeQString(nodeHeader, "TimesPlayed").toInt();
    m_fReplayGain = XmlParse::selectNodeQString(nodeHeader, "replaygain").toFloat();
    m_bHeaderParsed = false;
    m_bBpmLock = false;
    m_Rating = 0;

    // Mixxx <1.8 recorded track IDs in mixxxtrack.xml, but we are going to
    // ignore those. Tracks will get a new ID from the database.
    //m_iId = XmlParse::selectNodeQString(nodeHeader, "Id").toInt();
    m_iId = -1;

    m_fCuePoint = XmlParse::selectNodeQString(nodeHeader, "CuePoint").toFloat();
    m_bPlayed = false;
    m_bDirty = false;
    m_bLocationChanged = false;
}

void TrackInfoObject::initialize(bool parseHeader, bool parseCoverArt) {
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
    m_bHeaderParsed = false;
    m_iId = -1;
    m_iSampleRate = 0;
    m_iChannels = 0;
    m_fCuePoint = 0.0f;
    m_dateAdded = QDateTime::currentDateTime();
    m_Rating = 0;
    m_bBpmLock = false;
    m_sGrouping = "";
    m_sAlbumArtist = "";

    // parse() parses the metadata from file. This is not a quick operation!
    if (parseHeader) {
        parse(parseCoverArt);
    }
}

TrackInfoObject::~TrackInfoObject() {
    //qDebug() << "~TrackInfoObject()" << m_iId << getInfo();

    // Notifies TrackDAO and other listeners that this track is about to be
    // deleted and should be saved to the database, removed from caches, etc.
    emit(deleted(this));
}

void TrackInfoObject::parse(bool parseCoverArt) {
    // Log parsing of header information in developer mode. This is useful for
    // tracking down corrupt files.
    const QString& canonicalLocation = m_fileInfo.canonicalFilePath();
    if (CmdlineArgs::Instance().getDeveloper()) {
        qDebug() << "TrackInfoObject::parse()" << canonicalLocation;
    }

    // Parse the information stored in the sound file.
    SoundSourceProxy proxy(canonicalLocation, m_pSecurityToken);
    Mixxx::SoundSourcePointer pSoundSource(proxy.getSoundSource());
    if (pSoundSource && pSoundSource->parseHeader() == OK) {

        // Dump the metadata extracted from the file into the track.

        // TODO(XXX): This involves locking the mutex for every setXXX
        // method. We should figure out an optimization where there are private
        // setters that don't lock the mutex.

        // If Artist, Title and Type fields are not blank, modify them.
        // Otherwise, keep their current values.
        // TODO(rryan): Should we re-visit this decision?
        if (!(pSoundSource->getArtist().isEmpty())) {
            setArtist(pSoundSource->getArtist());
        } else {
            parseArtist();
        }

        if (!(pSoundSource->getTitle().isEmpty())) {
            setTitle(pSoundSource->getTitle());
        } else {
            parseTitle();
        }

        if (!(pSoundSource->getType().isEmpty())) {
            setType(pSoundSource->getType());
        }

        setAlbum(pSoundSource->getAlbum());
        setAlbumArtist(pSoundSource->getAlbumArtist());
        setYear(pSoundSource->getYear());
        setGenre(pSoundSource->getGenre());
        setComposer(pSoundSource->getComposer());
        setGrouping(pSoundSource->getGrouping());
        setComment(pSoundSource->getComment());
        setTrackNumber(pSoundSource->getTrackNumber());
        float replayGain = pSoundSource->getReplayGain();
        if (replayGain != 0) {
            setReplayGain(replayGain);
        }
        setDuration(pSoundSource->getDuration());
        setBitrate(pSoundSource->getBitrate());
        setSampleRate(pSoundSource->getSampleRate());
        setChannels(pSoundSource->getChannels());

        // Need to set BPM after sample rate since beat grid creation depends on
        // knowing the sample rate. Bug #1020438.
        float bpm = pSoundSource->getBPM();
        if (bpm > 0) {
            // do not delete beat grid if bpm is not set in file
            setBpm(bpm);
        }

        QString key = pSoundSource->getKey();
        if (!key.isEmpty()) {
            setKeyText(key, mixxx::track::io::key::FILE_METADATA);
        }

        if (parseCoverArt) {
            m_coverArt.image = pSoundSource->parseCoverArt();
            if (!m_coverArt.image.isNull()) {
                m_coverArt.info.hash = CoverArtUtils::calculateHash(
                    m_coverArt.image);
                m_coverArt.info.coverLocation = QString();
                m_coverArt.info.type = CoverInfo::METADATA;
                m_coverArt.info.source = CoverInfo::GUESSED;
            }
        }

        setHeaderParsed(true);
    } else {
        qDebug() << "TrackInfoObject::parse() error at file"
                 << canonicalLocation;
        setHeaderParsed(false);

        // Add basic information derived from the filename:
        parseFilename();
    }
}

void TrackInfoObject::parseArtist() {
    QMutexLocker lock(&m_qMutex);
    QString filename = m_fileInfo.fileName();
    filename = filename.replace("_", " ");
    if (filename.count('-') == 1) {
        m_sArtist = filename.section('-', 0, 0).trimmed();
    }
    setDirty(true);
}

void TrackInfoObject::parseTitle() {
    QMutexLocker lock(&m_qMutex);
    QString filename = m_fileInfo.fileName();
    filename = filename.replace("_", " ");
    if (filename.count('-') == 1) {
        m_sTitle = filename.section('-', 1, 1).trimmed();
        // Remove the file type from m_sTitle
        m_sTitle = m_sTitle.section('.', 0, -2).trimmed();
    } else {
        m_sTitle = filename.section('.', 0, -2).trimmed();
    }
    setDirty(true);
}

void TrackInfoObject::parseFilename() {
    // If the file name has the following form: "Artist - Title.type", extract
    // Artist, Title and type fields
    parseArtist();
    parseTitle();

    // Add no comment
    m_sComment.clear();

    // Find the type
    QString filename = m_fileInfo.fileName();
    m_sType = filename.section(".",-1).toLower().trimmed();
    setDirty(true);
}

QString TrackInfoObject::getDurationStr() const {
    QMutexLocker lock(&m_qMutex);
    int iDuration = m_iDuration;
    lock.unlock();

    return Time::formatSeconds(iDuration, true);
}

void TrackInfoObject::setLocation(const QString& location) {
    QMutexLocker lock(&m_qMutex);
    QFileInfo newFileInfo(location);
    if (newFileInfo != m_fileInfo) {
        m_fileInfo = newFileInfo;
        m_bLocationChanged = true;
        setDirty(true);
    }
}

QString TrackInfoObject::getLocation() const {
    QMutexLocker lock(&m_qMutex);
    return m_fileInfo.absoluteFilePath();
}

QString TrackInfoObject::getCanonicalLocation() const {
    QMutexLocker lock(&m_qMutex);
    return m_fileInfo.canonicalFilePath();
}

QFileInfo TrackInfoObject::getFileInfo() const {
    // No need for locking since we are passing a copy by value. Qt doesn't say
    // that QFileInfo is thread-safe but its copy constructor just copies the
    // d_ptr.
    return m_fileInfo;
}

SecurityTokenPointer TrackInfoObject::getSecurityToken() {
    return m_pSecurityToken;
}

QString TrackInfoObject::getDirectory() const {
    QMutexLocker lock(&m_qMutex);
    return m_fileInfo.absolutePath();
}

QString TrackInfoObject::getFilename() const {
    QMutexLocker lock(&m_qMutex);
    return m_fileInfo.fileName();
}

bool TrackInfoObject::exists() const {
    QMutexLocker lock(&m_qMutex);
    // return here a fresh calculated value to be sure
    // the file is not deleted or gone with an USB-Stick
    // because it will probably stop the Auto-DJ
    return QFile::exists(m_fileInfo.absoluteFilePath());
}

float TrackInfoObject::getReplayGain() const {
    QMutexLocker lock(&m_qMutex);
    return m_fReplayGain;
}

void TrackInfoObject::setReplayGain(float f) {
    QMutexLocker lock(&m_qMutex);
    //qDebug() << "Reported ReplayGain value: " << m_fReplayGain;
    if (m_fReplayGain != f) {
        m_fReplayGain = f;
        setDirty(true);
    }
    lock.unlock();
    emit(ReplayGainUpdated(f));
}

double TrackInfoObject::getBpm() const {
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

void TrackInfoObject::setBpm(double f) {
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

    if (dirty) {
        setDirty(true);
    }

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
    if (m_bHeaderParsed != parsed) {
        m_bHeaderParsed = parsed;
        setDirty(true);
    }
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

void TrackInfoObject::setDateAdded(const QDateTime& dateAdded) {
    QMutexLocker lock(&m_qMutex);
    m_dateAdded = dateAdded;
}

QDateTime TrackInfoObject::getFileModifiedTime() const {
    QMutexLocker lock(&m_qMutex);
    return m_fileInfo.lastModified();
}

QDateTime TrackInfoObject::getFileCreationTime() const {
    QMutexLocker lock(&m_qMutex);
    return m_fileInfo.created();
}

int TrackInfoObject::getDuration()  const {
    QMutexLocker lock(&m_qMutex);
    return m_iDuration;
}

void TrackInfoObject::setDuration(int i) {
    QMutexLocker lock(&m_qMutex);
    if (m_iDuration != i) {
        m_iDuration = i;
        setDirty(true);
    }
}

QString TrackInfoObject::getTitle() const {
    QMutexLocker lock(&m_qMutex);
    return m_sTitle;
}

void TrackInfoObject::setTitle(const QString& s) {
    QMutexLocker lock(&m_qMutex);
    QString title = s.trimmed();
    if (m_sTitle != title) {
        m_sTitle = title;
        setDirty(true);
    }
}

QString TrackInfoObject::getArtist() const {
    QMutexLocker lock(&m_qMutex);
    return m_sArtist;
}

void TrackInfoObject::setArtist(const QString& s) {
    QMutexLocker lock(&m_qMutex);
    QString artist = s.trimmed();
    if (m_sArtist != artist) {
        m_sArtist = artist;
        setDirty(true);
    }
}

QString TrackInfoObject::getAlbum() const {
    QMutexLocker lock(&m_qMutex);
    return m_sAlbum;
}

void TrackInfoObject::setAlbum(const QString& s) {
    QMutexLocker lock(&m_qMutex);
    QString album = s.trimmed();
    if (m_sAlbum != album) {
        m_sAlbum = album;
        setDirty(true);
    }
}

QString TrackInfoObject::getAlbumArtist()  const {
    QMutexLocker lock(&m_qMutex);
    return m_sAlbumArtist;
}

void TrackInfoObject::setAlbumArtist(const QString& s) {
    QMutexLocker lock(&m_qMutex);
    QString st = s.trimmed();
    if (m_sAlbumArtist != st) {
        m_sAlbumArtist = st;
        setDirty(true);
    }
}

QString TrackInfoObject::getYear()  const {
    QMutexLocker lock(&m_qMutex);
    return m_sYear;
}

void TrackInfoObject::setYear(const QString& s) {
    QMutexLocker lock(&m_qMutex);
    QString year = s.trimmed();
    if (m_sYear != year) {
        m_sYear = year;
        setDirty(true);
    }
}

QString TrackInfoObject::getGenre() const {
    QMutexLocker lock(&m_qMutex);
    return m_sGenre;
}

void TrackInfoObject::setGenre(const QString& s) {
    QMutexLocker lock(&m_qMutex);
    QString genre = s.trimmed();
    if (m_sGenre != genre) {
        m_sGenre = genre;
        setDirty(true);
    }
}

QString TrackInfoObject::getComposer() const {
    QMutexLocker lock(&m_qMutex);
    return m_sComposer;
}

void TrackInfoObject::setComposer(const QString& s) {
    QMutexLocker lock(&m_qMutex);
    QString composer = s.trimmed();
    if (m_sComposer != composer) {
        m_sComposer = composer;
        setDirty(true);
    }
}

QString TrackInfoObject::getGrouping()  const {
    QMutexLocker lock(&m_qMutex);
    return m_sGrouping;
}

void TrackInfoObject::setGrouping(const QString& s) {
    QMutexLocker lock(&m_qMutex);
    QString grouping = s.trimmed();
    if (m_sGrouping != grouping) {
        m_sGrouping = grouping;
        setDirty(true);
    }
}

QString TrackInfoObject::getTrackNumber()  const {
    QMutexLocker lock(&m_qMutex);
    return m_sTrackNumber;
}

void TrackInfoObject::setTrackNumber(const QString& s) {
    QMutexLocker lock(&m_qMutex);
    QString tn = s.trimmed();
    if (m_sTrackNumber != tn) {
        m_sTrackNumber = tn;
        setDirty(true);
    }
}

int TrackInfoObject::getTimesPlayed() const {
    QMutexLocker lock(&m_qMutex);
    return m_iTimesPlayed;
}

void TrackInfoObject::setTimesPlayed(int t) {
    QMutexLocker lock(&m_qMutex);
    if (t != m_iTimesPlayed) {
        m_iTimesPlayed = t;
        setDirty(true);
    }
}

void TrackInfoObject::incTimesPlayed() {
    setPlayedAndUpdatePlaycount(true);
}

bool TrackInfoObject::getPlayed() const {
    QMutexLocker lock(&m_qMutex);
    bool bPlayed = m_bPlayed;
    return bPlayed;
}

void TrackInfoObject::setPlayedAndUpdatePlaycount(bool bPlayed) {
    QMutexLocker lock(&m_qMutex);
    if (bPlayed) {
        ++m_iTimesPlayed;
        setDirty(true);
    }
    else if (m_bPlayed && !bPlayed) {
        m_iTimesPlayed = math_max(0, m_iTimesPlayed - 1);
        setDirty(true);
    }
    m_bPlayed = bPlayed;
}

void TrackInfoObject::setPlayed(bool bPlayed) {
    QMutexLocker lock(&m_qMutex);
    if (bPlayed != m_bPlayed) {
        m_bPlayed = bPlayed;
        setDirty(true);
    }
}

QString TrackInfoObject::getComment() const {
    QMutexLocker lock(&m_qMutex);
    return m_sComment;
}

void TrackInfoObject::setComment(const QString& s) {
    QMutexLocker lock(&m_qMutex);
    if (s != m_sComment) {
        m_sComment = s;
        setDirty(true);
    }
}

QString TrackInfoObject::getType() const {
    QMutexLocker lock(&m_qMutex);
    return m_sType;
}

void TrackInfoObject::setType(const QString& s) {
    QMutexLocker lock(&m_qMutex);
    if (s != m_sType) {
        m_sType = s;
        setDirty(true);
    }
}

void TrackInfoObject::setSampleRate(int iSampleRate) {
    QMutexLocker lock(&m_qMutex);
    if (m_iSampleRate != iSampleRate) {
        m_iSampleRate = iSampleRate;
        setDirty(true);
    }
}

int TrackInfoObject::getSampleRate() const {
    QMutexLocker lock(&m_qMutex);
    return m_iSampleRate;
}

void TrackInfoObject::setChannels(int iChannels) {
    QMutexLocker lock(&m_qMutex);
    if (m_iChannels != iChannels) {
        m_iChannels = iChannels;
        setDirty(true);
    }
}

int TrackInfoObject::getChannels() const {
    QMutexLocker lock(&m_qMutex);
    return m_iChannels;
}

int TrackInfoObject::getLength() const {
    QMutexLocker lock(&m_qMutex);
    return m_fileInfo.size();
}

int TrackInfoObject::getBitrate() const {
    QMutexLocker lock(&m_qMutex);
    return m_iBitrate;
}

QString TrackInfoObject::getBitrateStr() const {
    return QString("%1").arg(getBitrate());
}

void TrackInfoObject::setBitrate(int i) {
    QMutexLocker lock(&m_qMutex);
    if (m_iBitrate != i) {
        m_iBitrate = i;
        setDirty(true);
    }
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

void TrackInfoObject::setURL(const QString& url) {
    QMutexLocker lock(&m_qMutex);
    if (m_sURL != url) {
        m_sURL = url;
        setDirty(true);
    }
}

QString TrackInfoObject::getURL() {
    QMutexLocker lock(&m_qMutex);
    return m_sURL;
}

ConstWaveformPointer TrackInfoObject::getWaveform() {
    return m_waveform;
}

void TrackInfoObject::setWaveform(ConstWaveformPointer pWaveform) {
    m_waveform = pWaveform;
    emit(waveformUpdated());
}

ConstWaveformPointer TrackInfoObject::getWaveformSummary() const {
    return m_waveformSummary;
}

void TrackInfoObject::setWaveformSummary(ConstWaveformPointer pWaveform) {
    m_waveformSummary = pWaveform;
    emit(waveformSummaryUpdated());
}

void TrackInfoObject::setAnalyserProgress(int progress) {
    // progress in 0 .. 1000. QAtomicInt so no need for lock.
    if (progress != load_atomic(m_analyserProgress)) {
        m_analyserProgress = progress;
        emit(analyserProgress(progress));
    }
}

int TrackInfoObject::getAnalyserProgress() const {
    // QAtomicInt so no need for lock.
    return load_atomic(m_analyserProgress);
}

void TrackInfoObject::setCuePoint(float cue) {
    QMutexLocker lock(&m_qMutex);
    if (m_fCuePoint != cue) {
        m_fCuePoint = cue;
        setDirty(true);
    }
}

float TrackInfoObject::getCuePoint() {
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
    // TODO(XXX): Delete the cue point.
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

void TrackInfoObject::setDirty(bool bDirty) {

    QMutexLocker lock(&m_qMutex);
    bool change = m_bDirty != bDirty;
    m_bDirty = bDirty;
    lock.unlock();
    // qDebug() << "Track" << m_iId << getInfo() << (change? "changed" : "unchanged")
    //          << "set" << (bDirty ? "dirty" : "clean");
    if (change) {
        if (m_bDirty) {
            emit(dirty(this));
        } else {
            emit(clean(this));
        }
    }
    // Emit a changed signal regardless if this attempted to set us dirty.
    if (bDirty) {
        emit(changed(this));
    }

    //qDebug() << QString("TrackInfoObject %1 %2 set to %3").arg(QString::number(m_iId), m_fileInfo.absoluteFilePath(), m_bDirty ? "dirty" : "clean");
}

bool TrackInfoObject::isDirty() {
    QMutexLocker lock(&m_qMutex);
    return m_bDirty;
}

bool TrackInfoObject::locationChanged() {
    QMutexLocker lock(&m_qMutex);
    return m_bLocationChanged;
}

int TrackInfoObject::getRating() const {
    QMutexLocker lock(&m_qMutex);
    return m_Rating;
}

void TrackInfoObject::setRating (int rating) {
    QMutexLocker lock(&m_qMutex);
    if (rating != m_Rating) {
        m_Rating = rating;
        setDirty(true);
    }
}

void TrackInfoObject::setKeys(Keys keys) {
    QMutexLocker lock(&m_qMutex);
    setDirty(true);
    m_keys = keys;
    // Might be INVALID. We don't care.
    mixxx::track::io::key::ChromaticKey newKey = m_keys.getGlobalKey();
    lock.unlock();
    emit(keyUpdated(KeyUtils::keyToNumericValue(newKey)));
    emit(keysUpdated());
}

const Keys& TrackInfoObject::getKeys() const {
    QMutexLocker lock(&m_qMutex);
    return m_keys;
}

mixxx::track::io::key::ChromaticKey TrackInfoObject::getKey() const {
    QMutexLocker lock(&m_qMutex);
    if (!m_keys.isValid()) {
        return mixxx::track::io::key::INVALID;
    }
    return m_keys.getGlobalKey();
}

void TrackInfoObject::setKey(mixxx::track::io::key::ChromaticKey key,
                             mixxx::track::io::key::Source source) {
    QMutexLocker lock(&m_qMutex);
    bool dirty = false;
    if (key == mixxx::track::io::key::INVALID) {
        m_keys = Keys();
        dirty = true;
    } else if (m_keys.getGlobalKey() != key) {
        m_keys = KeyFactory::makeBasicKeys(key, source);
    }

    if (dirty) {
        setDirty(true);
    }

    // Might be INVALID. We don't care.
    mixxx::track::io::key::ChromaticKey newKey = m_keys.getGlobalKey();
    lock.unlock();
    emit(keyUpdated(KeyUtils::keyToNumericValue(newKey)));
    emit(keysUpdated());
}

void TrackInfoObject::setKeyText(QString key,
                                 mixxx::track::io::key::Source source) {
    QMutexLocker lock(&m_qMutex);

    Keys newKeys = KeyFactory::makeBasicKeysFromText(key, source);

    // We treat this as dirtying if it is parsed to a different key or if we
    // fail to parse the key, if the text value is different from the current
    // text value.
    bool dirty = newKeys.getGlobalKey() != m_keys.getGlobalKey() ||
            (newKeys.getGlobalKey() == mixxx::track::io::key::INVALID &&
             newKeys.getGlobalKeyText() != m_keys.getGlobalKeyText());
    if (dirty) {
        m_keys = newKeys;
        setDirty(true);
        // Might be INVALID. We don't care.
        mixxx::track::io::key::ChromaticKey newKey = m_keys.getGlobalKey();
        lock.unlock();
        emit(keyUpdated(KeyUtils::keyToNumericValue(newKey)));
        emit(keysUpdated());
    }
}

QString TrackInfoObject::getKeyText() const {
    QMutexLocker lock(&m_qMutex);

    mixxx::track::io::key::ChromaticKey key = m_keys.getGlobalKey();
    if (key != mixxx::track::io::key::INVALID) {
        return KeyUtils::keyToString(key);
    }

    // Fall back on text global name.
    QString keyText = m_keys.getGlobalKeyText();
    return keyText;
}

void TrackInfoObject::setBpmLock(bool bpmLock) {
    QMutexLocker lock(&m_qMutex);
    if (bpmLock != m_bBpmLock) {
        m_bBpmLock = bpmLock;
        setDirty(true);
    }
}

bool TrackInfoObject::hasBpmLock() const {
    QMutexLocker lock(&m_qMutex);
    return m_bBpmLock;
}

void TrackInfoObject::setCoverInfo(const CoverInfo& info) {
    QMutexLocker lock(&m_qMutex);
    if (info != m_coverArt.info) {
        m_coverArt = CoverArt();
        m_coverArt.info = info;
        setDirty(true);
        lock.unlock();
        emit(coverArtUpdated());
    }
}

CoverInfo TrackInfoObject::getCoverInfo() const {
    QMutexLocker lock(&m_qMutex);
    return m_coverArt.info;
}

void TrackInfoObject::setCoverArt(const CoverArt& cover) {
    QMutexLocker lock(&m_qMutex);
    if (cover != m_coverArt) {
        m_coverArt = cover;
        setDirty(true);
        lock.unlock();
        emit(coverArtUpdated());
    }
}

CoverArt TrackInfoObject::getCoverArt() const {
    QMutexLocker lock(&m_qMutex);
    return m_coverArt;
}
