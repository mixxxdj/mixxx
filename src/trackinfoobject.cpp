#include <QDirIterator>
#include <QFile>
#include <QMutexLocker>
#include <QtDebug>

#include "trackinfoobject.h"

#include "library/coverartutils.h"
#include "soundsourceproxy.h"
#include "track/beatfactory.h"
#include "track/keyfactory.h"
#include "track/keyutils.h"
#include "track/trackmetadatataglib.h"
#include "util/assert.h"
#include "util/compatibility.h"
#include "util/time.h"
#include "util/xml.h"


namespace {

SecurityTokenPointer openSecurityToken(
        const QFileInfo& fileInfo,
        const SecurityTokenPointer& pSecurityToken = SecurityTokenPointer()) {
    if (pSecurityToken.isNull()) {
        return Sandbox::openSecurityToken(fileInfo, true);
    } else {
        return pSecurityToken;
    }
}

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

TrackInfoObject::TrackInfoObject(
        const QFileInfo& fileInfo,
        const SecurityTokenPointer& pSecurityToken,
        TrackId trackId)
        : m_fileInfo(fileInfo),
          m_pSecurityToken(openSecurityToken(m_fileInfo, pSecurityToken)),
          m_bDeleteOnReferenceExpiration(false),
          m_qMutex(QMutex::Recursive),
          m_id(trackId),
          m_bDirty(false),
          m_iRating(0),
          m_fCuePoint(0.0f),
          m_dateAdded(QDateTime::currentDateTime()),
          m_bHeaderParsed(false),
          m_bBpmLocked(false),
          m_analyzerProgress(-1) {
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

void TrackInfoObject::setTrackMetadata(
        const Mixxx::TrackMetadata& trackMetadata,
        bool parsedFromFile) {
    {
        // enter locking scope
        QMutexLocker lock(&m_qMutex);

        bool modified = compareAndSet(&m_bHeaderParsed, parsedFromFile);
        bool modifiedReplayGain = false;
        if (m_metadata != trackMetadata) {
            modifiedReplayGain =
                    (m_metadata.getReplayGain() != trackMetadata.getReplayGain());
            m_metadata = trackMetadata;
            modified = true;
        }
        if (modified) {
            // explicitly unlock before emitting signals
            markDirtyAndUnlock(&lock);
            if (modifiedReplayGain) {
                emit(ReplayGainUpdated(trackMetadata.getReplayGain()));
            }
        }
        // implicitly unlocked when leaving scope
    }

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

void TrackInfoObject::getTrackMetadata(
        Mixxx::TrackMetadata* pTrackMetadata,
        bool* pHeaderParsed) const {
    QMutexLocker lock(&m_qMutex);
    *pTrackMetadata = m_metadata;
    *pHeaderParsed = m_bHeaderParsed;
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
    return m_metadata.getReplayGain();
}

void TrackInfoObject::setReplayGain(const Mixxx::ReplayGain& replayGain) {
    QMutexLocker lock(&m_qMutex);
    if (m_metadata.getReplayGain() != replayGain) {
        m_metadata.setReplayGain(replayGain);
        markDirtyAndUnlock(&lock);
        emit(ReplayGainUpdated(replayGain));
    }
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

    Mixxx::Bpm normalizedBpm(bpmValue);
    normalizedBpm.normalizeValue();

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
        bpm.normalizeValue();
        pObject = dynamic_cast<QObject*>(m_pBeats.data());
        if (pObject) {
            connect(pObject, SIGNAL(updated()),
                    this, SLOT(slotBeatsUpdated()));
        }
    }

    m_metadata.setBpm(bpm);

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
    double bpmValue = m_pBeats->getBpm();
    Mixxx::Bpm bpm(bpmValue);
    bpm.normalizeValue();
    m_metadata.setBpm(bpm);
    markDirtyAndUnlock(&lock);
    emit(bpmUpdated(bpmValue));
    emit(beatsUpdated());
}

void TrackInfoObject::setHeaderParsed(bool headerParsed) {
    QMutexLocker lock(&m_qMutex);
    if (compareAndSet(&m_bHeaderParsed, headerParsed)) {
        markDirtyAndUnlock(&lock);
    }
}

bool TrackInfoObject::isHeaderParsed() const {
    QMutexLocker lock(&m_qMutex);
    return m_bHeaderParsed;
}

QString TrackInfoObject::getInfo() const {
    QMutexLocker lock(&m_qMutex);
    if (m_metadata.getArtist().trimmed().isEmpty()) {
        return m_metadata.getTitle();
    } else {
        return m_metadata.getArtist() + ", " + m_metadata.getTitle();
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
    if (m_metadata.getDuration() != iDuration) {
        m_metadata.setDuration(iDuration);
        markDirtyAndUnlock(&lock);
    }
}

int TrackInfoObject::getDuration() const {
    QMutexLocker lock(&m_qMutex);
    return m_metadata.getDuration();
}

QString TrackInfoObject::getDurationText() const {
    return Time::formatSeconds(getDuration());
}

QString TrackInfoObject::getTitle() const {
    QMutexLocker lock(&m_qMutex);
    return m_metadata.getTitle();
}

void TrackInfoObject::setTitle(const QString& s) {
    QMutexLocker lock(&m_qMutex);
    const QString trimmed(s.trimmed());
    if (m_metadata.getTitle() != trimmed) {
        m_metadata.setTitle(trimmed);
        markDirtyAndUnlock(&lock);
    }
}

QString TrackInfoObject::getArtist() const {
    QMutexLocker lock(&m_qMutex);
    return m_metadata.getArtist();
}

void TrackInfoObject::setArtist(const QString& s) {
    QMutexLocker lock(&m_qMutex);
    const QString trimmed(s.trimmed());
    if (m_metadata.getArtist() != trimmed) {
        m_metadata.setArtist(trimmed);
        markDirtyAndUnlock(&lock);
    }
}

QString TrackInfoObject::getAlbum() const {
    QMutexLocker lock(&m_qMutex);
    return m_metadata.getAlbum();
}

void TrackInfoObject::setAlbum(const QString& s) {
    QMutexLocker lock(&m_qMutex);
    const QString trimmed(s.trimmed());
    if (m_metadata.getAlbum() != trimmed) {
        m_metadata.setAlbum(trimmed);
        markDirtyAndUnlock(&lock);
    }
}

QString TrackInfoObject::getAlbumArtist()  const {
    QMutexLocker lock(&m_qMutex);
    return m_metadata.getAlbumArtist();
}

void TrackInfoObject::setAlbumArtist(const QString& s) {
    QMutexLocker lock(&m_qMutex);
    const QString trimmed(s.trimmed());
    if (m_metadata.getAlbumArtist() != trimmed) {
        m_metadata.setAlbumArtist(trimmed);
        markDirtyAndUnlock(&lock);
    }
}

QString TrackInfoObject::getYear()  const {
    QMutexLocker lock(&m_qMutex);
    return m_metadata.getYear();
}

void TrackInfoObject::setYear(const QString& s) {
    QMutexLocker lock(&m_qMutex);
    const QString trimmed(s.trimmed());
    if (m_metadata.getYear() != trimmed) {
        m_metadata.setYear(trimmed);
        markDirtyAndUnlock(&lock);
    }
}

QString TrackInfoObject::getGenre() const {
    QMutexLocker lock(&m_qMutex);
    return m_metadata.getGenre();
}

void TrackInfoObject::setGenre(const QString& s) {
    QMutexLocker lock(&m_qMutex);
    const QString trimmed(s.trimmed());
    if (m_metadata.getGenre() != trimmed) {
        m_metadata.setGenre(trimmed);
        markDirtyAndUnlock(&lock);
    }
}

QString TrackInfoObject::getComposer() const {
    QMutexLocker lock(&m_qMutex);
    return m_metadata.getComposer();
}

void TrackInfoObject::setComposer(const QString& s) {
    QMutexLocker lock(&m_qMutex);
    const QString trimmed(s.trimmed());
    if (m_metadata.getComposer() != trimmed) {
        m_metadata.setComposer(trimmed);
        markDirtyAndUnlock(&lock);
    }
}

QString TrackInfoObject::getGrouping()  const {
    QMutexLocker lock(&m_qMutex);
    return m_metadata.getGrouping();
}

void TrackInfoObject::setGrouping(const QString& s) {
    QMutexLocker lock(&m_qMutex);
    const QString trimmed(s.trimmed());
    if (m_metadata.getGrouping() != trimmed) {
        m_metadata.setGrouping(trimmed);
        markDirtyAndUnlock(&lock);
    }
}

QString TrackInfoObject::getTrackNumber()  const {
    QMutexLocker lock(&m_qMutex);
    return m_metadata.getTrackNumber();
}

QString TrackInfoObject::getTrackTotal()  const {
    QMutexLocker lock(&m_qMutex);
    return m_metadata.getTrackTotal();
}

void TrackInfoObject::setTrackNumber(const QString& s) {
    QMutexLocker lock(&m_qMutex);
    const QString trimmed(s.trimmed());
    if (m_metadata.getTrackNumber() != trimmed) {
        m_metadata.setTrackNumber(trimmed);
        markDirtyAndUnlock(&lock);
    }
}

void TrackInfoObject::setTrackTotal(const QString& s) {
    QMutexLocker lock(&m_qMutex);
    const QString trimmed(s.trimmed());
    if (m_metadata.getTrackTotal() != trimmed) {
        m_metadata.setTrackTotal(trimmed);
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
    return m_metadata.getComment();
}

void TrackInfoObject::setComment(const QString& s) {
    QMutexLocker lock(&m_qMutex);
    if (m_metadata.getComment() != s) {
        m_metadata.setComment(s);
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
    if (m_metadata.getSampleRate() != iSampleRate) {
        m_metadata.setSampleRate(iSampleRate);
        markDirtyAndUnlock(&lock);
    }
}

int TrackInfoObject::getSampleRate() const {
    QMutexLocker lock(&m_qMutex);
    return m_metadata.getSampleRate();
}

void TrackInfoObject::setChannels(int iChannels) {
    QMutexLocker lock(&m_qMutex);
    if (m_metadata.getChannels() != iChannels) {
        m_metadata.setChannels(iChannels);
        markDirtyAndUnlock(&lock);
    }
}

int TrackInfoObject::getChannels() const {
    QMutexLocker lock(&m_qMutex);
    return m_metadata.getChannels();
}

int TrackInfoObject::getBitrate() const {
    QMutexLocker lock(&m_qMutex);
    return m_metadata.getBitrate();
}

QString TrackInfoObject::getBitrateText() const {
    return QString("%1").arg(getBitrate());
}

void TrackInfoObject::setBitrate(int iBitrate) {
    QMutexLocker lock(&m_qMutex);
    if (m_metadata.getBitrate() != iBitrate) {
        m_metadata.setBitrate(iBitrate);
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
    m_id = id;
    // Changing the Id does not make the track dirty because the Id is always
    // generated by the Database itself.
}

void TrackInfoObject::setURL(const QString& url) {
    QMutexLocker lock(&m_qMutex);
    if (m_sURL != url) {
        m_sURL = url;
        markDirtyAndUnlock(&lock);
    }
}

QString TrackInfoObject::getURL() const {
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

float TrackInfoObject::getCuePoint() const {
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

void TrackInfoObject::markDirty() {
    QMutexLocker lock(&m_qMutex);
    setDirtyAndUnlock(&lock, true);
}

void TrackInfoObject::markClean() {
    QMutexLocker lock(&m_qMutex);
    setDirtyAndUnlock(&lock, false);
}

void TrackInfoObject::markDirtyAndUnlock(QMutexLocker* pLock, bool bDirty) {
    bool result = m_bDirty || bDirty;
    setDirtyAndUnlock(pLock, result);
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
    m_metadata.setKey(KeyUtils::getGlobalKeyText(m_keys));
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
