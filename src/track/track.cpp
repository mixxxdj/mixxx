#include <QDirIterator>
#include <QFile>
#include <QMutexLocker>
#include <QtDebug>

#include "track/track.h"

#include "track/beatfactory.h"
#include "track/keyfactory.h"
#include "track/keyutils.h"
#include "track/trackmetadatataglib.h"
#include "util/assert.h"
#include "util/compatibility.h"


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

Track::Track(
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
TrackPointer Track::newTemporary(
        const QFileInfo& fileInfo,
        const SecurityTokenPointer& pSecurityToken) {
    return TrackPointer(
            new Track(
                    fileInfo,
                    pSecurityToken,
                    TrackId()),
            &QObject::deleteLater);
}

//static
TrackPointer Track::newDummy(
        const QFileInfo& fileInfo,
        TrackId trackId) {
    return TrackPointer(
            new Track(
                    fileInfo,
                    SecurityTokenPointer(),
                    trackId),
            &QObject::deleteLater);
}

// static
void Track::onTrackReferenceExpired(Track* pTrack) {
    DEBUG_ASSERT_AND_HANDLE(pTrack != nullptr) {
        return;
    }
    //qDebug() << "Track::onTrackReferenceExpired"
    //         << pTrack << pTrack->getId() << pTrack->getInfo();
    if (pTrack->m_bDeleteOnReferenceExpiration) {
        delete pTrack;
    } else {
        emit(pTrack->referenceExpired(pTrack));
    }
}

void Track::setDeleteOnReferenceExpiration(bool deleteOnReferenceExpiration) {
    m_bDeleteOnReferenceExpiration = deleteOnReferenceExpiration;
}

void Track::setTrackMetadata(
        const mixxx::TrackMetadata& trackMetadata,
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
            ((nullptr == m_pBeats) || !mixxx::Bpm::isValidValue(m_pBeats->getBpm()))) {
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

void Track::getTrackMetadata(
        mixxx::TrackMetadata* pTrackMetadata,
        bool* pHeaderParsed) const {
    QMutexLocker lock(&m_qMutex);
    *pTrackMetadata = m_metadata;
    *pHeaderParsed = m_bHeaderParsed;
}

QString Track::getLocation() const {
    // Copying QFileInfo is thread-safe due to "implicit sharing"
    // (copy-on write). But operating on a single instance of QFileInfo
    // might not be thread-safe due to internal caching!
    QMutexLocker lock(&m_qMutex);
    return m_fileInfo.absoluteFilePath();
}

QString Track::getCanonicalLocation() const {
    // Copying QFileInfo is thread-safe due to "implicit sharing"
    // (copy-on write). But operating on a single instance of QFileInfo
    // might not be thread-safe due to internal caching!
    QMutexLocker lock(&m_qMutex);
    return m_fileInfo.canonicalFilePath();
}

QString Track::getDirectory() const {
    // Copying QFileInfo is thread-safe due to "implicit sharing"
    // (copy-on write). But operating on a single instance of QFileInfo
    // might not be thread-safe due to internal caching!
    QMutexLocker lock(&m_qMutex);
    return m_fileInfo.absolutePath();
}

QString Track::getFileName() const {
    // Copying QFileInfo is thread-safe due to "implicit sharing"
    // (copy-on write). But operating on a single instance of QFileInfo
    // might not be thread-safe due to internal caching!
    QMutexLocker lock(&m_qMutex);
    return m_fileInfo.fileName();
}

int Track::getFileSize() const {
    // Copying QFileInfo is thread-safe due to "implicit sharing"
    // (copy-on write). But operating on a single instance of QFileInfo
    // might not be thread-safe due to internal caching!
    QMutexLocker lock(&m_qMutex);
    return m_fileInfo.size();
}

QDateTime Track::getFileModifiedTime() const {
    // Copying QFileInfo is thread-safe due to "implicit sharing"
    // (copy-on write). But operating on a single instance of QFileInfo
    // might not be thread-safe due to internal caching!
    QMutexLocker lock(&m_qMutex);
    return m_fileInfo.lastModified();
}

QDateTime Track::getFileCreationTime() const {
    // Copying QFileInfo is thread-safe due to "implicit sharing"
    // (copy-on write). But operating on a single instance of QFileInfo
    // might not be thread-safe due to internal caching!
    QMutexLocker lock(&m_qMutex);
    return m_fileInfo.created();
}

bool Track::exists() const {
    // Copying QFileInfo is thread-safe due to "implicit sharing"
    // (copy-on write). But operating on a single instance of QFileInfo
    // might not be thread-safe due to internal caching!
    QMutexLocker lock(&m_qMutex);
    // return here a fresh calculated value to be sure
    // the file is not deleted or gone with an USB-Stick
    // because it will probably stop the Auto-DJ
    return QFile::exists(m_fileInfo.absoluteFilePath());
}

mixxx::ReplayGain Track::getReplayGain() const {
    QMutexLocker lock(&m_qMutex);
    return m_metadata.getReplayGain();
}

void Track::setReplayGain(const mixxx::ReplayGain& replayGain) {
    QMutexLocker lock(&m_qMutex);
    if (m_metadata.getReplayGain() != replayGain) {
        m_metadata.setReplayGain(replayGain);
        markDirtyAndUnlock(&lock);
        emit(ReplayGainUpdated(replayGain));
    }
}

double Track::getBpm() const {
    double bpm = mixxx::Bpm::kValueUndefined;
    QMutexLocker lock(&m_qMutex);
    if (m_pBeats) {
        // BPM from beat grid overrides BPM from metadata
        // Reason: The BPM value in the metadata might be imprecise,
        // e.g. ID3v2 only supports integer values!
        double beatsBpm = m_pBeats->getBpm();
        if (mixxx::Bpm::isValidValue(beatsBpm)) {
            bpm = beatsBpm;
        }
    }
    return bpm;
}

double Track::setBpm(double bpmValue) {
    if (!mixxx::Bpm::isValidValue(bpmValue)) {
        // If the user sets the BPM to an invalid value, we assume
        // they want to clear the beatgrid.
        setBeats(BeatsPointer());
        return bpmValue;
    }

    mixxx::Bpm normalizedBpm(bpmValue);
    normalizedBpm.normalizeValue();

    QMutexLocker lock(&m_qMutex);

    if (!m_pBeats) {
        // No beat grid available -> create and initialize
        double cue = getCuePoint();
        BeatsPointer pBeats(BeatFactory::makeBeatGrid(this, bpmValue, cue));
        setBeatsAndUnlock(&lock, pBeats);
        return bpmValue;
    }

    // Continue with the regular case
    if (m_pBeats->getBpm() != bpmValue) {
        qDebug() << "Updating BPM:" << getLocation();
        m_pBeats->setBpm(bpmValue);
        markDirtyAndUnlock(&lock);
        // Tell the GUI to update the bpm label...
        //qDebug() << "Track signaling BPM update to" << f;
        emit(bpmUpdated(bpmValue));
    }

    return bpmValue;
}

QString Track::getBpmText() const {
    return QString("%1").arg(getBpm(), 3,'f',1);
}

void Track::setBeats(BeatsPointer pBeats) {
    QMutexLocker lock(&m_qMutex);
    setBeatsAndUnlock(&lock, pBeats);
}

void Track::setBeatsAndUnlock(QMutexLocker* pLock, BeatsPointer pBeats) {
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

    mixxx::Bpm bpm;
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

BeatsPointer Track::getBeats() const {
    QMutexLocker lock(&m_qMutex);
    return m_pBeats;
}

void Track::slotBeatsUpdated() {
    QMutexLocker lock(&m_qMutex);
    double bpmValue = m_pBeats->getBpm();
    mixxx::Bpm bpm(bpmValue);
    bpm.normalizeValue();
    m_metadata.setBpm(bpm);
    markDirtyAndUnlock(&lock);
    emit(bpmUpdated(bpmValue));
    emit(beatsUpdated());
}

void Track::setHeaderParsed(bool headerParsed) {
    QMutexLocker lock(&m_qMutex);
    if (compareAndSet(&m_bHeaderParsed, headerParsed)) {
        markDirtyAndUnlock(&lock);
    }
}

bool Track::isHeaderParsed() const {
    QMutexLocker lock(&m_qMutex);
    return m_bHeaderParsed;
}

QString Track::getInfo() const {
    QMutexLocker lock(&m_qMutex);
    if (m_metadata.getArtist().trimmed().isEmpty()) {
        return m_metadata.getTitle();
    } else {
        return m_metadata.getArtist() + ", " + m_metadata.getTitle();
    }
}

QDateTime Track::getDateAdded() const {
    QMutexLocker lock(&m_qMutex);
    return m_dateAdded;
}

void Track::setDateAdded(const QDateTime& dateAdded) {
    QMutexLocker lock(&m_qMutex);
    m_dateAdded = dateAdded;
}

void Track::setDuration(double duration) {
    QMutexLocker lock(&m_qMutex);
    if (m_metadata.getDuration() != duration) {
        m_metadata.setDuration(duration);
        markDirtyAndUnlock(&lock);
    }
}

double Track::getDuration(DurationRounding rounding) const {
    QMutexLocker lock(&m_qMutex);
    switch (rounding) {
    case DurationRounding::SECONDS:
        return std::round(m_metadata.getDuration());
    case DurationRounding::NONE:
        return m_metadata.getDuration();
    }
    // unreachable code / avoid compiler warnings
    DEBUG_ASSERT(!"unhandled enum value");
    return m_metadata.getDuration();
}

QString Track::getDurationText(mixxx::Duration::Precision precision) const {
    double duration;
    if (precision == mixxx::Duration::Precision::SECONDS) {
        // Round to full seconds before formatting for consistency:
        // getDurationText() should always display the same number
        // as getDuration(DurationRounding::SECONDS) = getDurationInt()
        duration = getDuration(DurationRounding::SECONDS);
    } else {
        duration = getDuration(DurationRounding::NONE);
    }
    return mixxx::Duration::formatSeconds(duration, precision);
}

QString Track::getTitle() const {
    QMutexLocker lock(&m_qMutex);
    return m_metadata.getTitle();
}

void Track::setTitle(const QString& s) {
    QMutexLocker lock(&m_qMutex);
    const QString trimmed(s.trimmed());
    if (m_metadata.getTitle() != trimmed) {
        m_metadata.setTitle(trimmed);
        markDirtyAndUnlock(&lock);
    }
}

QString Track::getArtist() const {
    QMutexLocker lock(&m_qMutex);
    return m_metadata.getArtist();
}

void Track::setArtist(const QString& s) {
    QMutexLocker lock(&m_qMutex);
    const QString trimmed(s.trimmed());
    if (m_metadata.getArtist() != trimmed) {
        m_metadata.setArtist(trimmed);
        markDirtyAndUnlock(&lock);
    }
}

QString Track::getAlbum() const {
    QMutexLocker lock(&m_qMutex);
    return m_metadata.getAlbum();
}

void Track::setAlbum(const QString& s) {
    QMutexLocker lock(&m_qMutex);
    const QString trimmed(s.trimmed());
    if (m_metadata.getAlbum() != trimmed) {
        m_metadata.setAlbum(trimmed);
        markDirtyAndUnlock(&lock);
    }
}

QString Track::getAlbumArtist()  const {
    QMutexLocker lock(&m_qMutex);
    return m_metadata.getAlbumArtist();
}

void Track::setAlbumArtist(const QString& s) {
    QMutexLocker lock(&m_qMutex);
    const QString trimmed(s.trimmed());
    if (m_metadata.getAlbumArtist() != trimmed) {
        m_metadata.setAlbumArtist(trimmed);
        markDirtyAndUnlock(&lock);
    }
}

QString Track::getYear()  const {
    QMutexLocker lock(&m_qMutex);
    return m_metadata.getYear();
}

void Track::setYear(const QString& s) {
    QMutexLocker lock(&m_qMutex);
    const QString trimmed(s.trimmed());
    if (m_metadata.getYear() != trimmed) {
        m_metadata.setYear(trimmed);
        markDirtyAndUnlock(&lock);
    }
}

QString Track::getGenre() const {
    QMutexLocker lock(&m_qMutex);
    return m_metadata.getGenre();
}

void Track::setGenre(const QString& s) {
    QMutexLocker lock(&m_qMutex);
    const QString trimmed(s.trimmed());
    if (m_metadata.getGenre() != trimmed) {
        m_metadata.setGenre(trimmed);
        markDirtyAndUnlock(&lock);
    }
}

QString Track::getComposer() const {
    QMutexLocker lock(&m_qMutex);
    return m_metadata.getComposer();
}

void Track::setComposer(const QString& s) {
    QMutexLocker lock(&m_qMutex);
    const QString trimmed(s.trimmed());
    if (m_metadata.getComposer() != trimmed) {
        m_metadata.setComposer(trimmed);
        markDirtyAndUnlock(&lock);
    }
}

QString Track::getGrouping()  const {
    QMutexLocker lock(&m_qMutex);
    return m_metadata.getGrouping();
}

void Track::setGrouping(const QString& s) {
    QMutexLocker lock(&m_qMutex);
    const QString trimmed(s.trimmed());
    if (m_metadata.getGrouping() != trimmed) {
        m_metadata.setGrouping(trimmed);
        markDirtyAndUnlock(&lock);
    }
}

QString Track::getTrackNumber()  const {
    QMutexLocker lock(&m_qMutex);
    return m_metadata.getTrackNumber();
}

QString Track::getTrackTotal()  const {
    QMutexLocker lock(&m_qMutex);
    return m_metadata.getTrackTotal();
}

void Track::setTrackNumber(const QString& s) {
    QMutexLocker lock(&m_qMutex);
    const QString trimmed(s.trimmed());
    if (m_metadata.getTrackNumber() != trimmed) {
        m_metadata.setTrackNumber(trimmed);
        markDirtyAndUnlock(&lock);
    }
}

void Track::setTrackTotal(const QString& s) {
    QMutexLocker lock(&m_qMutex);
    const QString trimmed(s.trimmed());
    if (m_metadata.getTrackTotal() != trimmed) {
        m_metadata.setTrackTotal(trimmed);
        markDirtyAndUnlock(&lock);
    }
}

PlayCounter Track::getPlayCounter() const {
    QMutexLocker lock(&m_qMutex);
    return m_playCounter;
}

void Track::setPlayCounter(const PlayCounter& playCounter) {
    QMutexLocker lock(&m_qMutex);
    if (compareAndSet(&m_playCounter, playCounter)) {
        markDirtyAndUnlock(&lock);
    }
}

void Track::updatePlayCounter(bool bPlayed) {
    QMutexLocker lock(&m_qMutex);
    PlayCounter playCounter(m_playCounter);
    playCounter.setPlayedAndUpdateTimesPlayed(bPlayed);
    if (compareAndSet(&m_playCounter, playCounter)) {
        markDirtyAndUnlock(&lock);
    }
}

QString Track::getComment() const {
    QMutexLocker lock(&m_qMutex);
    return m_metadata.getComment();
}

void Track::setComment(const QString& s) {
    QMutexLocker lock(&m_qMutex);
    if (m_metadata.getComment() != s) {
        m_metadata.setComment(s);
        markDirtyAndUnlock(&lock);
    }
}

QString Track::getType() const {
    QMutexLocker lock(&m_qMutex);
    return m_sType;
}

void Track::setType(const QString& s) {
    QMutexLocker lock(&m_qMutex);
    if (compareAndSet(&m_sType, s)) {
        markDirtyAndUnlock(&lock);
    }
}

void Track::setSampleRate(int iSampleRate) {
    QMutexLocker lock(&m_qMutex);
    if (m_metadata.getSampleRate() != iSampleRate) {
        m_metadata.setSampleRate(iSampleRate);
        markDirtyAndUnlock(&lock);
    }
}

int Track::getSampleRate() const {
    QMutexLocker lock(&m_qMutex);
    return m_metadata.getSampleRate();
}

void Track::setChannels(int iChannels) {
    QMutexLocker lock(&m_qMutex);
    if (m_metadata.getChannels() != iChannels) {
        m_metadata.setChannels(iChannels);
        markDirtyAndUnlock(&lock);
    }
}

int Track::getChannels() const {
    QMutexLocker lock(&m_qMutex);
    return m_metadata.getChannels();
}

int Track::getBitrate() const {
    QMutexLocker lock(&m_qMutex);
    return m_metadata.getBitrate();
}

QString Track::getBitrateText() const {
    return QString("%1").arg(getBitrate());
}

void Track::setBitrate(int iBitrate) {
    QMutexLocker lock(&m_qMutex);
    if (m_metadata.getBitrate() != iBitrate) {
        m_metadata.setBitrate(iBitrate);
        markDirtyAndUnlock(&lock);
    }
}

TrackId Track::getId() const {
    QMutexLocker lock(&m_qMutex);
    return m_id;
}

void Track::setId(TrackId id) {
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

void Track::setURL(const QString& url) {
    QMutexLocker lock(&m_qMutex);
    if (m_sURL != url) {
        m_sURL = url;
        markDirtyAndUnlock(&lock);
    }
}

QString Track::getURL() const {
    QMutexLocker lock(&m_qMutex);
    return m_sURL;
}

ConstWaveformPointer Track::getWaveform() {
    return m_waveform;
}

void Track::setWaveform(ConstWaveformPointer pWaveform) {
    m_waveform = pWaveform;
    emit(waveformUpdated());
}

ConstWaveformPointer Track::getWaveformSummary() const {
    return m_waveformSummary;
}

void Track::setWaveformSummary(ConstWaveformPointer pWaveform) {
    m_waveformSummary = pWaveform;
    emit(waveformSummaryUpdated());
}

void Track::setAnalyzerProgress(int progress) {
    // progress in 0 .. 1000. QAtomicInt so no need for lock.
    int oldProgress = m_analyzerProgress.fetchAndStoreAcquire(progress);
    if (progress != oldProgress) {
        emit(analyzerProgress(progress));
    }
}

int Track::getAnalyzerProgress() const {
    // QAtomicInt so no need for lock.
    return load_atomic(m_analyzerProgress);
}

void Track::setCuePoint(float cue) {
    QMutexLocker lock(&m_qMutex);
    if (compareAndSet(&m_fCuePoint, cue)) {
        markDirtyAndUnlock(&lock);
    }
}

float Track::getCuePoint() const {
    QMutexLocker lock(&m_qMutex);
    return m_fCuePoint;
}

void Track::slotCueUpdated() {
    markDirty();
    emit(cuesUpdated());
}

CuePointer Track::addCue() {
    QMutexLocker lock(&m_qMutex);
    CuePointer pCue(new Cue(m_id));
    connect(pCue.data(), SIGNAL(updated()),
            this, SLOT(slotCueUpdated()));
    m_cuePoints.push_back(pCue);
    markDirtyAndUnlock(&lock);
    emit(cuesUpdated());
    return pCue;
}

void Track::removeCue(const CuePointer& pCue) {
    QMutexLocker lock(&m_qMutex);
    disconnect(pCue.data(), 0, this, 0);
    m_cuePoints.removeOne(pCue);
    markDirtyAndUnlock(&lock);
    emit(cuesUpdated());
}

QList<CuePointer> Track::getCuePoints() const {
    QMutexLocker lock(&m_qMutex);
    return m_cuePoints;
}

void Track::setCuePoints(const QList<CuePointer>& cuePoints) {
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

void Track::markDirty() {
    QMutexLocker lock(&m_qMutex);
    setDirtyAndUnlock(&lock, true);
}

void Track::markClean() {
    QMutexLocker lock(&m_qMutex);
    setDirtyAndUnlock(&lock, false);
}

void Track::markDirtyAndUnlock(QMutexLocker* pLock, bool bDirty) {
    bool result = m_bDirty || bDirty;
    setDirtyAndUnlock(pLock, result);
}

void Track::setDirtyAndUnlock(QMutexLocker* pLock, bool bDirty) {
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

bool Track::isDirty() {
    QMutexLocker lock(&m_qMutex);
    return m_bDirty;
}

int Track::getRating() const {
    QMutexLocker lock(&m_qMutex);
    return m_iRating;
}

void Track::setRating (int rating) {
    QMutexLocker lock(&m_qMutex);
    if (compareAndSet(&m_iRating, rating)) {
        markDirtyAndUnlock(&lock);
    }
}

void Track::setKeys(const Keys& keys) {
    QMutexLocker lock(&m_qMutex);
    setKeysAndUnlock(&lock, keys);
}

void Track::setKeysAndUnlock(QMutexLocker* pLock, const Keys& keys) {
    m_keys = keys;
    m_metadata.setKey(KeyUtils::getGlobalKeyText(m_keys));
    // New key might be INVALID. We don't care.
    mixxx::track::io::key::ChromaticKey newKey = m_keys.getGlobalKey();
    markDirtyAndUnlock(pLock);
    emit(keyUpdated(KeyUtils::keyToNumericValue(newKey)));
    emit(keysUpdated());
}

Keys Track::getKeys() const {
    QMutexLocker lock(&m_qMutex);
    return m_keys;
}

void Track::setKey(mixxx::track::io::key::ChromaticKey key,
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

mixxx::track::io::key::ChromaticKey Track::getKey() const {
    QMutexLocker lock(&m_qMutex);
    if (m_keys.isValid()) {
        return m_keys.getGlobalKey();
    } else {
        return mixxx::track::io::key::INVALID;
    }
}

void Track::setKeyText(const QString& keyText,
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

QString Track::getKeyText() const {
    return KeyUtils::getGlobalKeyText(getKeys());
}

void Track::setBpmLocked(bool bpmLocked) {
    QMutexLocker lock(&m_qMutex);
    if (bpmLocked != m_bBpmLocked) {
        m_bBpmLocked = bpmLocked;
        markDirtyAndUnlock(&lock);
    }
}

bool Track::isBpmLocked() const {
    QMutexLocker lock(&m_qMutex);
    return m_bBpmLocked;
}

void Track::setCoverInfo(const CoverInfo& info) {
    QMutexLocker lock(&m_qMutex);
    if (info != m_coverInfo) {
        m_coverInfo = info;
        markDirtyAndUnlock(&lock);
        emit(coverArtUpdated());
    }
}

CoverInfo Track::getCoverInfo() const {
    QMutexLocker lock(&m_qMutex);
    return m_coverInfo;
}
