#include <QDirIterator>
#include <QFile>
#include <QMutexLocker>
#include <QtDebug>
#include <QRegExp>

#include "trackinfoobject.h"

#include "controlobject.h"
#include "soundsourceproxy.h"
#include "library/coverartutils.h"
#include "track/trackmetadata.h"
#include "track/beatfactory.h"
#include "track/keyfactory.h"
#include "track/keyutils.h"
#include "util/compatibility.h"
#include "util/cmdlineargs.h"
#include "util/time.h"
#include "util/math.h"
#include "util/assert.h"
#include "util/xml.h"


namespace {

template<typename T>
inline bool compareAndSet(T* pField, const T& value) {
    if (*pField != value) {
        *pField = value;
        return true;
    } else {
        return false;
    }
}

} // anonymous namespace

TrackInfoObject::TrackInfoObject(const QFileInfo& fileInfo,
                                 SecurityTokenPointer pToken,
                                 bool parseHeader, bool parseCoverArt)
        : m_fileInfo(fileInfo),
          m_pSecurityToken(pToken.isNull() ? Sandbox::openSecurityToken(
                  m_fileInfo, true) : pToken),
          m_bDeleteOnReferenceExpiration(false),
          m_qMutex(QMutex::Recursive) {
    m_id = TrackId();
    m_analyzerProgress = -1;

    m_bDirty = false;
    m_bBpmLocked = false;
    m_bHeaderParsed = false;

    m_iDuration = 0;
    m_iBitrate = 0;
    m_iSampleRate = 0;
    m_iRating = 0;
    m_iChannels = 0;
    m_fCuePoint = 0.0f;
    m_dateAdded = QDateTime::currentDateTime();

    // Parse the metadata from file. This is not a quick operation!
    if (parseHeader) {
        parse(parseCoverArt);
    }
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
    setTrackTotal(trackMetadata.getTrackTotal());
    setChannels(trackMetadata.getChannels());
    setSampleRate(trackMetadata.getSampleRate());
    setDuration(trackMetadata.getDuration());
    setBitrate(trackMetadata.getBitrate());
    setReplayGain(trackMetadata.getReplayGain());

    // Need to set BPM after sample rate since beat grid creation depends on
    // knowing the sample rate. Bug #1020438.
    if (trackMetadata.getBpm().hasValue() &&
            ((nullptr == m_pBeats) || !Mixxx::Bpm::isValidValue(m_pBeats->getBpm()))) {
        // Only (re-)set the BPM if the beat grid is not valid.
        // Reason: The BPM value in the metadata might be normalized or rounded,
        // e.g. ID3v2 only supports integer values!
        setBpm(trackMetadata.getBpm().getValue());
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
    pTrackMetadata->setTrackTotal(getTrackTotal());
    pTrackMetadata->setChannels(getChannels());
    pTrackMetadata->setSampleRate(getSampleRate());
    pTrackMetadata->setDuration(getDuration());
    pTrackMetadata->setBitrate(getBitrate());
    pTrackMetadata->setReplayGain(getReplayGain());
    pTrackMetadata->setBpm(Mixxx::Bpm(getBpm()));
    pTrackMetadata->setKey(getKeyText());
}

void TrackInfoObject::parse(bool parseCoverArt) {
    // Log parsing of header information in developer mode. This is useful for
    // tracking down corrupt files.
    const QString canonicalLocation(getCanonicalLocation());
    if (CmdlineArgs::Instance().getDeveloper()) {
        qDebug() << "TrackInfoObject::parse()" << canonicalLocation;
    }

    // TODO(uklotzde): Move parsing of metadata to SoundSourceProxy
    SoundSourceProxy proxy(canonicalLocation, m_pSecurityToken);
    if (!proxy.getType().isEmpty()) {
        setType(proxy.getType());

        // Parse the information stored in the sound file.
        Mixxx::TrackMetadata trackMetadata;
        QImage coverArt;
        // If parsing of the cover art image should be omitted the
        // 2nd output parameter must be set to NULL.
        QImage* pCoverArt = parseCoverArt ? &coverArt : NULL;
        if (proxy.parseTrackMetadataAndCoverArt(&trackMetadata, pCoverArt) == OK) {
            // If Artist, Title and Type fields are not blank, modify them.
            // Otherwise, keep their current values.
            // TODO(rryan): Should we re-visit this decision?
            if (trackMetadata.getArtist().isEmpty() || trackMetadata.getTitle().isEmpty()) {
                Mixxx::TrackMetadata fileNameMetadata;
                parseMetadataFromFileName(fileNameMetadata, getFileName());
                if (trackMetadata.getArtist().isEmpty()) {
                    trackMetadata.setArtist(fileNameMetadata.getArtist());
                }
                if (trackMetadata.getTitle().isEmpty()) {
                    trackMetadata.setTitle(fileNameMetadata.getTitle());
                }
            }

            if (pCoverArt && !pCoverArt->isNull()) {
                QMutexLocker lock(&m_qMutex);
                m_coverArt.image = *pCoverArt;
                m_coverArt.info.hash = CoverArtUtils::calculateHash(
                    m_coverArt.image);
                m_coverArt.info.coverLocation = QString();
                m_coverArt.info.type = CoverInfo::METADATA;
                m_coverArt.info.source = CoverInfo::GUESSED;
            }

            setHeaderParsed(true);
        } else {
            qDebug() << "TrackInfoObject::parse() error at file"
                     << canonicalLocation;

            // Add basic information derived from the filename
            parseMetadataFromFileName(trackMetadata, getFileName());

            setHeaderParsed(false);
        }
        // Dump the metadata extracted from the file into the track.
        setMetadata(trackMetadata);
    } else {
        qDebug() << "TrackInfoObject::parse() error at file"
                 << canonicalLocation;
        setHeaderParsed(false);
    }
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

Mixxx::ReplayGain TrackInfoObject::getReplayGain() const {
    QMutexLocker lock(&m_qMutex);
    return m_replayGain;
}

void TrackInfoObject::setReplayGain(const Mixxx::ReplayGain& replayGain) {
    { // locked
        QMutexLocker lock(&m_qMutex);
        //qDebug() << "Reported ReplayGain value: " << m_fReplayGain;
        if (m_replayGain != replayGain) {
            m_replayGain = replayGain;
            markDirtyAndUnlock(&lock);
        }
    } // unlocked
    emit(ReplayGainUpdated(replayGain));
}

double TrackInfoObject::getBpm() const {
    double bpm = Mixxx::Bpm::kValueUndefined;
    QMutexLocker lock(&m_qMutex);
    if (m_pBeats) {
        // BPM from beat grid overrides BPM from metadata
        // Reason: The BPM value in the metadata might be imprecise,
        // e.g. ID3v2 only supports integer values!
        double beatsBpm = m_pBeats->getBpm();
        if (Mixxx::Bpm::isValidValue(beatsBpm)) {
            bpm = beatsBpm;
        }
    }
    return bpm;
}

double TrackInfoObject::setBpm(double bpmValue) {
    if (!Mixxx::Bpm::isValidValue(bpmValue)) {
        // If the user sets the BPM to an invalid value, we assume
        // they want to clear the beatgrid.
        setBeats(BeatsPointer());
        return bpmValue;
    }

    QMutexLocker lock(&m_qMutex);

    if (!m_pBeats) {
        // No beat grid available -> create and initialize
        BeatsPointer pBeats(BeatFactory::makeBeatGrid(this, bpmValue, 0));
        setBeatsAndUnlock(&lock, pBeats);
        return bpmValue;
    }

    // Continue with the regular case
    if (m_pBeats->getBpm() != bpmValue) {
        qDebug() << "Updating BPM:" << getLocation();
        m_pBeats->setBpm(bpmValue);
        markDirtyAndUnlock(&lock);
        // Tell the GUI to update the bpm label...
        //qDebug() << "TrackInfoObject signaling BPM update to" << f;
        emit(bpmUpdated(bpmValue));
    }

    return bpmValue;
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

    Mixxx::Bpm bpm;
    double bpmValue = bpm.getValue();

    m_pBeats = pBeats;
    if (m_pBeats) {
        bpmValue = m_pBeats->getBpm();
        bpm.setValue(bpmValue);
        pObject = dynamic_cast<QObject*>(m_pBeats.data());
        if (pObject) {
            connect(pObject, SIGNAL(updated()),
                    this, SLOT(slotBeatsUpdated()));
        }
    }

    markDirtyAndUnlock(pLock);
    emit(bpmUpdated(bpmValue));
    emit(beatsUpdated());
}

BeatsPointer TrackInfoObject::getBeats() const {
    QMutexLocker lock(&m_qMutex);
    return m_pBeats;
}

void TrackInfoObject::slotBeatsUpdated() {
    QMutexLocker lock(&m_qMutex);
    double bpm = m_pBeats->getBpm();
    markDirtyAndUnlock(&lock);
    emit(bpmUpdated(bpm));
    emit(beatsUpdated());
}

bool TrackInfoObject::getHeaderParsed()  const {
    QMutexLocker lock(&m_qMutex);
    return m_bHeaderParsed;
}

void TrackInfoObject::setHeaderParsed(bool parsed) {
    QMutexLocker lock(&m_qMutex);
    if (compareAndSet(&m_bHeaderParsed, parsed)) {
        markDirtyAndUnlock(&lock);
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

void TrackInfoObject::setDuration(int iDuration) {
    QMutexLocker lock(&m_qMutex);
    if (m_iDuration != iDuration) {
        m_iDuration = iDuration;
        markDirtyAndUnlock(&lock);
    }
}

int TrackInfoObject::getDuration() const {
    QMutexLocker lock(&m_qMutex);
    return m_iDuration;
}

QString TrackInfoObject::getDurationText() const {
    return Time::formatSeconds(getDuration());
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
        markDirtyAndUnlock(&lock);
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
        markDirtyAndUnlock(&lock);
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
        markDirtyAndUnlock(&lock);
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
        markDirtyAndUnlock(&lock);
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
        markDirtyAndUnlock(&lock);
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
        markDirtyAndUnlock(&lock);
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
        markDirtyAndUnlock(&lock);
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
        markDirtyAndUnlock(&lock);
    }
}

QString TrackInfoObject::getTrackNumber()  const {
    QMutexLocker lock(&m_qMutex);
    return m_sTrackNumber;
}

QString TrackInfoObject::getTrackTotal()  const {
    QMutexLocker lock(&m_qMutex);
    return m_sTrackTotal;
}

void TrackInfoObject::setTrackNumber(const QString& s) {
    QMutexLocker lock(&m_qMutex);
    QString tn = s.trimmed();
    if (m_sTrackNumber != tn) {
        m_sTrackNumber = tn;
        markDirtyAndUnlock(&lock);
    }
}

void TrackInfoObject::setTrackTotal(const QString& s) {
    QMutexLocker lock(&m_qMutex);
    QString tn = s.trimmed();
    if (m_sTrackTotal != tn) {
        m_sTrackTotal = tn;
        markDirtyAndUnlock(&lock);
    }
}

PlayCounter TrackInfoObject::getPlayCounter() const {
    QMutexLocker lock(&m_qMutex);
    return m_playCounter;
}

void TrackInfoObject::setPlayCounter(const PlayCounter& playCounter) {
    QMutexLocker lock(&m_qMutex);
    if (compareAndSet(&m_playCounter, playCounter)) {
        markDirtyAndUnlock(&lock);
    }
}

void TrackInfoObject::updatePlayCounter(bool bPlayed) {
    QMutexLocker lock(&m_qMutex);
    PlayCounter playCounter(m_playCounter);
    playCounter.setPlayedAndUpdateTimesPlayed(bPlayed);
    if (compareAndSet(&m_playCounter, playCounter)) {
        markDirtyAndUnlock(&lock);
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
        markDirtyAndUnlock(&lock);
    }
}

QString TrackInfoObject::getType() const {
    QMutexLocker lock(&m_qMutex);
    return m_sType;
}

void TrackInfoObject::setType(const QString& s) {
    QMutexLocker lock(&m_qMutex);
    if (compareAndSet(&m_sType, s)) {
        markDirtyAndUnlock(&lock);
    }
}

void TrackInfoObject::setSampleRate(int iSampleRate) {
    QMutexLocker lock(&m_qMutex);
    if (compareAndSet(&m_iSampleRate, iSampleRate)) {
        markDirtyAndUnlock(&lock);
    }
}

int TrackInfoObject::getSampleRate() const {
    QMutexLocker lock(&m_qMutex);
    return m_iSampleRate;
}

void TrackInfoObject::setChannels(int iChannels) {
    QMutexLocker lock(&m_qMutex);
    if (compareAndSet(&m_iChannels, iChannels)) {
        markDirtyAndUnlock(&lock);
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

QString TrackInfoObject::getBitrateText() const {
    return QString("%1").arg(getBitrate());
}

void TrackInfoObject::setBitrate(int iBitrate) {
    QMutexLocker lock(&m_qMutex);
    if (compareAndSet(&m_iBitrate, iBitrate)) {
        markDirtyAndUnlock(&lock);
    }
}

TrackId TrackInfoObject::getId() const {
    QMutexLocker lock(&m_qMutex);
    return m_id;
}

void TrackInfoObject::setId(TrackId id) {
    QMutexLocker lock(&m_qMutex);
    // The track's id must be set only once and immediately after
    // the object has been created.
    DEBUG_ASSERT(!m_id.isValid());
    // The Id must not be modified from a valid value to another valid value
    DEBUG_ASSERT(!id.isValid() || !m_id.isValid() || (id == m_id));
    m_id = std::move(id);
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
    markDirtyAndUnlock(&lock);
    emit(wavesummaryUpdated(this));
}*/

void TrackInfoObject::setURL(const QString& url) {
    QMutexLocker lock(&m_qMutex);
    if (m_sURL != url) {
        m_sURL = url;
        markDirtyAndUnlock(&lock);
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

void TrackInfoObject::setAnalyzerProgress(int progress) {
    // progress in 0 .. 1000. QAtomicInt so no need for lock.
    int oldProgress = m_analyzerProgress.fetchAndStoreAcquire(progress);
    if (progress != oldProgress) {
        emit(analyzerProgress(progress));
    }
}

int TrackInfoObject::getAnalyzerProgress() const {
    // QAtomicInt so no need for lock.
    return load_atomic(m_analyzerProgress);
}

void TrackInfoObject::setCuePoint(float cue) {
    QMutexLocker lock(&m_qMutex);
    if (compareAndSet(&m_fCuePoint, cue)) {
        markDirtyAndUnlock(&lock);
    }
}

float TrackInfoObject::getCuePoint() {
    QMutexLocker lock(&m_qMutex);
    return m_fCuePoint;
}

void TrackInfoObject::slotCueUpdated() {
    markDirty();
    emit(cuesUpdated());
}

CuePointer TrackInfoObject::addCue() {
    QMutexLocker lock(&m_qMutex);
    CuePointer pCue(new Cue(m_id));
    connect(pCue.data(), SIGNAL(updated()),
            this, SLOT(slotCueUpdated()));
    m_cuePoints.push_back(pCue);
    markDirtyAndUnlock(&lock);
    emit(cuesUpdated());
    return pCue;
}

void TrackInfoObject::removeCue(const CuePointer& pCue) {
    QMutexLocker lock(&m_qMutex);
    disconnect(pCue.data(), 0, this, 0);
    m_cuePoints.removeOne(pCue);
    markDirtyAndUnlock(&lock);
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
    markDirtyAndUnlock(&lock);
    emit(cuesUpdated());
}

void TrackInfoObject::resetDirty() {
    QMutexLocker lock(&m_qMutex);
    setDirtyAndUnlock(&lock, false);
}

bool TrackInfoObject::markDirty(bool bDirty) {
    QMutexLocker lock(&m_qMutex);
    return markDirtyAndUnlock(&lock, bDirty);
}

bool TrackInfoObject::markDirtyAndUnlock(QMutexLocker* pLock, bool bDirty) {
    bool result = m_bDirty || bDirty;
    setDirtyAndUnlock(pLock, result);
    return result;
}

void TrackInfoObject::setDirtyAndUnlock(QMutexLocker* pLock, bool bDirty) {
   const bool dirtyChanged = m_bDirty != bDirty;
    m_bDirty = bDirty;

    // Unlock before emitting any signals!
    pLock->unlock();

    if (dirtyChanged) {
        if (bDirty) {
            emit(dirty(this));
        } else {
            emit(clean(this));
        }
    }
    if (bDirty) {
        // Emit a changed signal regardless if this attempted to set us dirty.
        emit(changed(this));
    }
}

bool TrackInfoObject::isDirty() {
    QMutexLocker lock(&m_qMutex);
    return m_bDirty;
}

int TrackInfoObject::getRating() const {
    QMutexLocker lock(&m_qMutex);
    return m_iRating;
}

void TrackInfoObject::setRating (int rating) {
    QMutexLocker lock(&m_qMutex);
    if (compareAndSet(&m_iRating, rating)) {
        markDirtyAndUnlock(&lock);
    }
}

void TrackInfoObject::setKeys(const Keys& keys) {
    QMutexLocker lock(&m_qMutex);
    setKeysAndUnlock(&lock, keys);
}

void TrackInfoObject::setKeysAndUnlock(QMutexLocker* pLock, const Keys& keys) {
    m_keys = keys;
    // New key might be INVALID. We don't care.
    mixxx::track::io::key::ChromaticKey newKey = m_keys.getGlobalKey();
    markDirtyAndUnlock(pLock);
    emit(keyUpdated(KeyUtils::keyToNumericValue(newKey)));
    emit(keysUpdated());
}

Keys TrackInfoObject::getKeys() const {
    QMutexLocker lock(&m_qMutex);
    return m_keys;
}

void TrackInfoObject::setKey(mixxx::track::io::key::ChromaticKey key,
                             mixxx::track::io::key::Source keySource) {
    if (key == mixxx::track::io::key::INVALID) {
        resetKeys();
    } else {
        Keys keys(KeyFactory::makeBasicKeys(key, keySource));
        QMutexLocker lock(&m_qMutex);
        if (m_keys.getGlobalKey() != key) {
            setKeysAndUnlock(&lock, keys);
        }
    }
}

mixxx::track::io::key::ChromaticKey TrackInfoObject::getKey() const {
    QMutexLocker lock(&m_qMutex);
    if (m_keys.isValid()) {
        return m_keys.getGlobalKey();
    } else {
        return mixxx::track::io::key::INVALID;
    }
}

void TrackInfoObject::setKeyText(const QString& keyText,
                                 mixxx::track::io::key::Source keySource) {
    Keys keys(KeyFactory::makeBasicKeysFromText(keyText, keySource));
    const mixxx::track::io::key::ChromaticKey globalKey(keys.getGlobalKey());
    if (globalKey == mixxx::track::io::key::INVALID) {
        resetKeys();
    } else {
        QMutexLocker lock(&m_qMutex);
        if (m_keys.getGlobalKey() != globalKey) {
            setKeysAndUnlock(&lock, keys);
        }
    }
}

QString TrackInfoObject::getKeyText() const {
    return KeyUtils::getGlobalKeyText(getKeys());
}

void TrackInfoObject::setBpmLocked(bool bpmLocked) {
    QMutexLocker lock(&m_qMutex);
    if (bpmLocked != m_bBpmLocked) {
        m_bBpmLocked = bpmLocked;
        markDirtyAndUnlock(&lock);
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
        markDirtyAndUnlock(&lock);
        emit(coverArtUpdated());
    }
}

CoverInfo TrackInfoObject::getCoverInfo() const {
    QMutexLocker lock(&m_qMutex);
    return m_coverArt.info;
}

void TrackInfoObject::setCoverArt(const CoverArt& coverArt) {
    QMutexLocker lock(&m_qMutex);
    if (compareAndSet(&m_coverArt, coverArt)) {
        markDirtyAndUnlock(&lock);
        emit(coverArtUpdated());
    }
}

CoverArt TrackInfoObject::getCoverArt() const {
    QMutexLocker lock(&m_qMutex);
    return m_coverArt;
}
