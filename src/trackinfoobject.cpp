#include <QDirIterator>
#include <QFile>
#include <QMutexLocker>
#include <QtDebug>
#include <QRegExp>

#include "trackinfoobject.h"

#include "controlobject.h"
#include "soundsourceproxy.h"
#include "library/coverartutils.h"
#include "metadata/trackmetadata.h"
#include "track/beatfactory.h"
#include "track/keyfactory.h"
#include "track/keyutils.h"
#include "util/compatibility.h"
#include "util/cmdlineargs.h"
#include "util/time.h"
#include "util/math.h"
#include "util/assert.h"
#include "util/xml.h"


TrackInfoObject::TrackInfoObject(
        const QFileInfo& fileInfo,
        const SecurityTokenPointer& pToken,
        TrackId trackId)
        : m_fileInfo(fileInfo),
          m_pSecurityToken(pToken.isNull() ? Sandbox::openSecurityToken(
                  m_fileInfo, true) : pToken),
          m_bDeleteOnReferenceExpiration(false),
          m_qMutex(QMutex::Recursive),
          m_id(std::move(trackId)) {
    m_analyserProgress = -1;

    m_bDirty = false;
    m_bBpmLocked = false;
    m_bHeaderParsed = false;

    m_iDuration = 0;
    m_iBitrate = 0;
    m_fReplayGain = 0.0f;
    m_bHeaderParsed = false;
    m_iSampleRate = 0;
    m_iChannels = 0;
    m_fCuePoint = 0.0f;
    m_dateAdded = QDateTime::currentDateTime();
    m_Rating = 0;

    m_bHeaderParsed = false;
}

//static
TrackPointer TrackInfoObject::newTemporary(
        const QFileInfo& fileInfo,
        const SecurityTokenPointer& pSecurityToken) {
    return TrackPointer(
            new TrackInfoObject(
                    fileInfo,
                    pSecurityToken,
                    TrackId()),
            &QObject::deleteLater);
}

//static
TrackPointer TrackInfoObject::newTemporaryForSameFile(
        const TrackPointer& pTrack) {
    QMutexLocker lock(&pTrack->m_qMutex);
    return newTemporary(
                pTrack->m_fileInfo,
                pTrack->m_pSecurityToken);
}

//static
TrackPointer TrackInfoObject::newDummy(
        const QFileInfo& fileInfo,
        TrackId trackId) {
    return TrackPointer(
            new TrackInfoObject(
                    fileInfo,
                    SecurityTokenPointer(),
                    trackId),
            &QObject::deleteLater);
}

// static
void TrackInfoObject::onTrackReferenceExpired(TrackInfoObject* pTrack) {
    DEBUG_ASSERT_AND_HANDLE(pTrack != NULL) {
        return;
    }
    //qDebug() << "TrackInfoObject::onTrackReferenceExpired"
    //         << pTrack << pTrack->getId() << pTrack->getInfo();
    if (pTrack->m_bDeleteOnReferenceExpiration) {
        delete pTrack;
    } else {
        emit(pTrack->referenceExpired(pTrack));
    }
}

void TrackInfoObject::setDeleteOnReferenceExpiration(bool deleteOnReferenceExpiration) {
    m_bDeleteOnReferenceExpiration = deleteOnReferenceExpiration;
}

namespace {
    // Parses artist/title from the file name and returns the file type.
    // Assumes that the file name is written like: "artist - title.xxx"
    // or "artist_-_title.xxx",
    void parseMetadataFromFileName(Mixxx::TrackMetadata& trackMetadata, QString fileName) {
        fileName.replace("_", " ");
        QString titleWithFileType;
        if (fileName.count('-') == 1) {
            const QString artist(fileName.section('-', 0, 0).trimmed());
            if (!artist.isEmpty()) {
                trackMetadata.setArtist(artist);
            }
            titleWithFileType = fileName.section('-', 1, 1).trimmed();
        } else {
            titleWithFileType = fileName.trimmed();
        }
        const QString title(titleWithFileType.section('.', 0, -2).trimmed());
        if (!title.isEmpty()) {
            trackMetadata.setTitle(title);
        }
    }
}

void TrackInfoObject::setMetadata(const Mixxx::TrackMetadata& trackMetadata) {
    // TODO(XXX): This involves locking the mutex for every setXXX
    // method. We should figure out an optimization where there are private
    // setters that don't lock the mutex.
    setArtist(trackMetadata.getArtist());
    setTitle(trackMetadata.getTitle());
    setAlbum(trackMetadata.getAlbum());
    setAlbumArtist(trackMetadata.getAlbumArtist());
    setYear(trackMetadata.getYear());
    setGenre(trackMetadata.getGenre());
    setComposer(trackMetadata.getComposer());
    setGrouping(trackMetadata.getGrouping());
    setComment(trackMetadata.getComment());
    setTrackNumber(trackMetadata.getTrackNumber());
    setChannels(trackMetadata.getChannels());
    setSampleRate(trackMetadata.getSampleRate());
    setDuration(trackMetadata.getDuration());
    setBitrate(trackMetadata.getBitrate());

    if (trackMetadata.isReplayGainValid()) {
        setReplayGain(trackMetadata.getReplayGain());
    }

    // Need to set BPM after sample rate since beat grid creation depends on
    // knowing the sample rate. Bug #1020438.
    if (trackMetadata.isBpmValid()) {
        setBpm(trackMetadata.getBpm());
    }

    const QString key(trackMetadata.getKey());
    if (!key.isEmpty()) {
        setKeyText(key, mixxx::track::io::key::FILE_METADATA);
    }
}

void TrackInfoObject::getMetadata(Mixxx::TrackMetadata* pTrackMetadata) {
    // TODO(XXX): This involves locking the mutex for every setXXX
    // method. We should figure out an optimization where there are private
    // getters that don't lock the mutex.
    pTrackMetadata->setArtist(getArtist());
    pTrackMetadata->setTitle(getTitle());
    pTrackMetadata->setAlbum(getAlbum());
    pTrackMetadata->setAlbumArtist(getAlbumArtist());
    pTrackMetadata->setYear(Mixxx::TrackMetadata::reformatYear(getYear()));
    pTrackMetadata->setGenre(getGenre());
    pTrackMetadata->setComposer(getComposer());
    pTrackMetadata->setGrouping(getGrouping());
    pTrackMetadata->setComment(getComment());
    pTrackMetadata->setTrackNumber(getTrackNumber());
    pTrackMetadata->setChannels(getChannels());
    pTrackMetadata->setSampleRate(getSampleRate());
    pTrackMetadata->setDuration(getDuration());
    pTrackMetadata->setBitrate(getBitrate());
    pTrackMetadata->setReplayGain(getReplayGain());
    pTrackMetadata->setBpm(getBpm());
    pTrackMetadata->setKey(getKeyText());
}

void TrackInfoObject::parseTrackMetadata(
        const SoundSourceProxy& proxy,
        bool parseCoverArt,
        bool reloadFromFile) {
    DEBUG_ASSERT(this == proxy.getTrack().data());

    if (proxy.getFilePath().isEmpty()) {
        qWarning() << "Failed to parse track metadata from file"
                << getLocation()
                << ": File is inaccessible or missing";
        setHeaderParsed(false);
        return;
    }

    const QString canonicalLocation(getCanonicalLocation());
    DEBUG_ASSERT_AND_HANDLE(proxy.getFilePath() == canonicalLocation) {
            qWarning() << "Failed to parse track metadata from file"
                    << getLocation()
                    << ": Mismatching file paths"
                    << proxy.getFilePath()
                    << "<>"
                    << canonicalLocation;
        setHeaderParsed(false);
        return;
    }

    if (proxy.getType().isEmpty()) {
        qWarning() << "Failed to parse track metadata from file"
                << getLocation()
                << ": Unsupported file type";
        setHeaderParsed(false);
        return;
    }
    setType(proxy.getType());

    bool parsedFromFile = getHeaderParsed();
    if (parsedFromFile && !reloadFromFile) {
        return; // do not reload from file
    }
    Mixxx::TrackMetadata trackMetadata;
    // Use the existing trackMetadata as default values. Otherwise
    // existing values in the library will be overwritten with
    // empty values if the corresponding file tags are missing.
    // Depending on the file type some kind of tags might even
    // not be supported at all and those would get lost!
    getMetadata(&trackMetadata);
    QImage coverArt;
    // If parsing of the cover art image should be omitted the
    // 2nd output parameter must be set to nullptr. Cover art
    // is not reloaded from file once the metadata has been parsed!
    QImage* pCoverArt = (parseCoverArt && !parsedFromFile) ? &coverArt : nullptr;
    // Parse the tags stored in the audio file.
    if (proxy.parseTrackMetadataAndCoverArt(&trackMetadata, pCoverArt) == OK) {
        parsedFromFile = true;
    } else {
        qWarning() << "Failed to parse tags from audio file"
                 << canonicalLocation;
    }

    // If Artist or title fields are blank try to parse them
    // from the file name.
    // TODO(rryan): Should we re-visit this decision?
    if (trackMetadata.getArtist().isEmpty() || trackMetadata.getTitle().isEmpty()) {
        parseMetadataFromFileName(trackMetadata, m_fileInfo.fileName());
    }

    // Dump the trackMetadata extracted from the file back into the track.
    setMetadata(trackMetadata);
    if (pCoverArt && !pCoverArt->isNull()) {
        CoverArt coverArt;
        coverArt.image = *pCoverArt;
        coverArt.info.hash = CoverArtUtils::calculateHash(
                coverArt.image);
        coverArt.info.coverLocation = QString();
        coverArt.info.type = CoverInfo::METADATA;
        coverArt.info.source = CoverInfo::GUESSED;
        setCoverArt(coverArt);
    }
    setHeaderParsed(parsedFromFile);
}

QString TrackInfoObject::getDurationStr() const {
    QMutexLocker lock(&m_qMutex);
    int iDuration = m_iDuration;
    lock.unlock();

    return Time::formatSeconds(iDuration, false);
}

QString TrackInfoObject::getLocation() const {
    // Copying QFileInfo is thread-safe due to "implicit sharing"
    // (copy-on write). But operating on a single instance of QFileInfo
    // might not be thread-safe due to internal caching!
    QMutexLocker lock(&m_qMutex);
    return m_fileInfo.absoluteFilePath();
}

QString TrackInfoObject::getCanonicalLocation() const {
    // Copying QFileInfo is thread-safe due to "implicit sharing"
    // (copy-on write). But operating on a single instance of QFileInfo
    // might not be thread-safe due to internal caching!
    QMutexLocker lock(&m_qMutex);
    return m_fileInfo.canonicalFilePath();
}

QString TrackInfoObject::getDirectory() const {
    // Copying QFileInfo is thread-safe due to "implicit sharing"
    // (copy-on write). But operating on a single instance of QFileInfo
    // might not be thread-safe due to internal caching!
    QMutexLocker lock(&m_qMutex);
    return m_fileInfo.absolutePath();
}

QString TrackInfoObject::getFileName() const {
    // Copying QFileInfo is thread-safe due to "implicit sharing"
    // (copy-on write). But operating on a single instance of QFileInfo
    // might not be thread-safe due to internal caching!
    QMutexLocker lock(&m_qMutex);
    return m_fileInfo.fileName();
}

int TrackInfoObject::getFileSize() const {
    // Copying QFileInfo is thread-safe due to "implicit sharing"
    // (copy-on write). But operating on a single instance of QFileInfo
    // might not be thread-safe due to internal caching!
    QMutexLocker lock(&m_qMutex);
    return m_fileInfo.size();
}

QDateTime TrackInfoObject::getFileModifiedTime() const {
    // Copying QFileInfo is thread-safe due to "implicit sharing"
    // (copy-on write). But operating on a single instance of QFileInfo
    // might not be thread-safe due to internal caching!
    QMutexLocker lock(&m_qMutex);
    return m_fileInfo.lastModified();
}

QDateTime TrackInfoObject::getFileCreationTime() const {
    // Copying QFileInfo is thread-safe due to "implicit sharing"
    // (copy-on write). But operating on a single instance of QFileInfo
    // might not be thread-safe due to internal caching!
    QMutexLocker lock(&m_qMutex);
    return m_fileInfo.created();
}

bool TrackInfoObject::exists() const {
    // Copying QFileInfo is thread-safe due to "implicit sharing"
    // (copy-on write). But operating on a single instance of QFileInfo
    // might not be thread-safe due to internal caching!
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
    if (m_pBeats) {
        // getBpm() returns -1 when invalid.
        double bpm = m_pBeats->getBpm();
        if (bpm >= 0.0) {
            return bpm;
        }
    }
    return 0.0;
}

double TrackInfoObject::setBpm(double bpm) {
    if (bpm < 0.0) {
        // Ignore invalid BPM values
        return Mixxx::TrackMetadata::kBpmUndefined;
    }

    if (bpm == 0.0) {
        // If the user sets the BPM to 0, we assume they want to clear the
        // beatgrid.
        setBeats(BeatsPointer());
        return bpm;
    }

    QMutexLocker lock(&m_qMutex);

    if (!m_pBeats) {
        // No beat grid available -> create and initialize
        BeatsPointer pBeats(BeatFactory::makeBeatGrid(this, bpm, 0));
        setBeatsAndUnlock(&lock, pBeats);
        return bpm;
    }

    // Continue with the regular case
    if (m_pBeats->getBpm() != bpm) {
        qDebug() << "Updating BPM:" << getLocation();
        m_pBeats->setBpm(bpm);
        setDirty(true);
        lock.unlock();
        // Tell the GUI to update the bpm label...
        //qDebug() << "TrackInfoObject signaling BPM update to" << f;
        emit(bpmUpdated(bpm));
    }

    return bpm;
}

QString TrackInfoObject::getBpmText() const {
    return QString("%1").arg(getBpm(), 3,'f',1);
}

void TrackInfoObject::setBeats(BeatsPointer pBeats) {
    QMutexLocker lock(&m_qMutex);
    setBeatsAndUnlock(&lock, pBeats);
}

void TrackInfoObject::setBeatsAndUnlock(QMutexLocker* pLock, BeatsPointer pBeats) {
    // This whole method is not so great. The fact that Beats is an ABC is
    // limiting with respect to QObject and signals/slots.

    if (m_pBeats == pBeats) {
        pLock->unlock();
        return;
    }

    QObject* pObject = nullptr;
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
    pLock->unlock();
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

bool TrackInfoObject::getHeaderParsed()  const {
    QMutexLocker lock(&m_qMutex);
    return m_bHeaderParsed;
}

void TrackInfoObject::setHeaderParsed(bool parsed) {
    QMutexLocker lock(&m_qMutex);
    if (m_bHeaderParsed != parsed) {
        m_bHeaderParsed = parsed;
        setDirty(true);
    }
}

QString TrackInfoObject::getInfo() const {
    QMutexLocker lock(&m_qMutex);
    if (m_sArtist.trimmed().isEmpty()) {
        return m_sTitle;
    } else {
        return m_sArtist + ", " + m_sTitle;
    }
}

QDateTime TrackInfoObject::getDateAdded() const {
    QMutexLocker lock(&m_qMutex);
    return m_dateAdded;
}

void TrackInfoObject::setDateAdded(const QDateTime& dateAdded) {
    QMutexLocker lock(&m_qMutex);
    m_dateAdded = dateAdded;
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

void TrackInfoObject::setPlayed(bool played) {
    QMutexLocker lock(&m_qMutex);
    PlayCounter playCounter(m_playCounter);
    playCounter.setPlayed(played);
    if (m_playCounter != playCounter) {
        m_playCounter = playCounter;
        setDirty(true);
    }
}

bool TrackInfoObject::isPlayed() const {
    QMutexLocker lock(&m_qMutex);
    return m_playCounter.isPlayed();
}

void TrackInfoObject::setTimesPlayed(int timesPlayed) {
    QMutexLocker lock(&m_qMutex);
    PlayCounter playCounter(m_playCounter);
    playCounter.setTimesPlayed(timesPlayed);
    if (m_playCounter != playCounter) {
        m_playCounter = playCounter;
        setDirty(true);
    }
}

void TrackInfoObject::resetTimesPlayed() {
    QMutexLocker lock(&m_qMutex);
    PlayCounter playCounter(m_playCounter);
    playCounter.resetTimesPlayed();
    if (m_playCounter != playCounter) {
        m_playCounter = playCounter;
        setDirty(true);
    }
}

int TrackInfoObject::getTimesPlayed() const {
    QMutexLocker lock(&m_qMutex);
    return m_playCounter.getTimesPlayed();
}

void TrackInfoObject::setPlayedAndUpdatePlayCount(bool bPlayed) {
    QMutexLocker lock(&m_qMutex);
    PlayCounter playCounter(m_playCounter);
    playCounter.updatePlayed(bPlayed);
    if (m_playCounter != playCounter) {
        m_playCounter = playCounter;
        setDirty(true);
    }
}

PlayCounter TrackInfoObject::getPlayCounter() const {
    QMutexLocker lock(&m_qMutex);
    return m_playCounter;
}

void TrackInfoObject::setPlayCounter(const PlayCounter& playCounter) {
    QMutexLocker lock(&m_qMutex);
    if (m_playCounter != playCounter) {
        m_playCounter = playCounter;
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

TrackId TrackInfoObject::getId() const {
    QMutexLocker lock(&m_qMutex);
    return m_id;
}

void TrackInfoObject::setId(TrackId trackId) {
    QMutexLocker lock(&m_qMutex);
    // The track's id must be set only once and immediately after
    // the object has been created.
    DEBUG_ASSERT(trackId.isValid());
    DEBUG_ASSERT(!m_id.isValid());
    m_id = std::move(trackId);
    // Changing the Id does not make the track dirty because the Id is always
    // generated by the Database itself.
}


//TODO (vrince) remove clean-up when new summary is ready
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
	int oldProgress = m_analyserProgress.fetchAndStoreAcquire(progress);
    if (progress != oldProgress) {
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

CuePointer TrackInfoObject::addCue() {
    //qDebug() << "TrackInfoObject::addCue()";
    QMutexLocker lock(&m_qMutex);
    CuePointer pCue(new Cue(m_trackRef.getId()));
    connect(pCue.data(), SIGNAL(updated()),
            this, SLOT(slotCueUpdated()));
    m_cuePoints.push_back(pCue);
    setDirty(true);
    lock.unlock();
    emit(cuesUpdated());
    return pCue;
}

void TrackInfoObject::removeCue(const CuePointer& pCue) {
    QMutexLocker lock(&m_qMutex);
    disconnect(pCue.data(), 0, this, 0);
    m_cuePoints.removeOne(pCue);
    setDirty(true);
    lock.unlock();
    emit(cuesUpdated());
}

QList<CuePointer> TrackInfoObject::getCuePoints() const {
    QMutexLocker lock(&m_qMutex);
    return m_cuePoints;
}

void TrackInfoObject::setCuePoints(const QList<CuePointer>& cuePoints) {
    //qDebug() << "setCuePoints" << cuePoints.length();
    QMutexLocker lock(&m_qMutex);
    // disconnect existing cue points
    for (const auto& pCue: m_cuePoints) {
        disconnect(pCue.data(), 0, this, 0);
    }
    m_cuePoints = cuePoints;
    // connect new cue points
    for (const auto& pCue: m_cuePoints) {
        connect(pCue.data(), SIGNAL(updated()),
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
    // qDebug() << "Track" << m_id << getInfo() << (change? "changed" : "unchanged")
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

    //qDebug() << QString("TrackInfoObject %1 %2 set to %3").arg(QString::number(m_id), m_fileInfo.absoluteFilePath(), m_bDirty ? "dirty" : "clean");
}

bool TrackInfoObject::isDirty() {
    QMutexLocker lock(&m_qMutex);
    return m_bDirty;
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
    return KeyUtils::getGlobalKeyText(m_keys);
}

void TrackInfoObject::setBpmLocked(bool bpmLocked) {
    QMutexLocker lock(&m_qMutex);
    if (bpmLocked != m_bBpmLocked) {
        m_bBpmLocked = bpmLocked;
        setDirty(true);
    }
}

bool TrackInfoObject::isBpmLocked() const {
    QMutexLocker lock(&m_qMutex);
    return m_bBpmLocked;
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
