#include <QDirIterator>
#include <QFile>
#include <QMutexLocker>
#include <QtDebug>

#include "track/track.h"

#include "track/beatfactory.h"
#include "util/assert.h"
#include "util/logger.h"
#include "util/compatibility.h"


namespace {

mixxx::Logger kLogger("Track");

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
inline
bool compareAndSet(T* pField, const T& value) {
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
          m_record(trackId),
          m_bDirty(false),
          m_bExportMetadata(false),
          m_analyzerProgress(-1) {
}

//static
TrackPointer Track::newTemporary(
        const QFileInfo& fileInfo,
        const SecurityTokenPointer& pSecurityToken) {
    Track* pTrack =
            new Track(
                    fileInfo,
                    pSecurityToken,
                    TrackId());
    return TrackPointer(pTrack);
}

//static
TrackPointer Track::newDummy(
        const QFileInfo& fileInfo,
        TrackId trackId) {
    Track* pTrack =
            new Track(
                    fileInfo,
                    SecurityTokenPointer(),
                    trackId);
    return TrackPointer(pTrack);
}

// static
void Track::onTrackReferenceExpired(Track* pTrack) {
    VERIFY_OR_DEBUG_ASSERT(pTrack != nullptr) {
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
        mixxx::TrackMetadata trackMetadata,
        QDateTime metadataSynchronized) {
    // Safe some values that are needed after move assignment and unlocking(see below)
    const auto newBpm = trackMetadata.getTrackInfo().getBpm();
    const auto newKey = trackMetadata.getTrackInfo().getKey();
    const auto newReplayGain = trackMetadata.getTrackInfo().getReplayGain();

    {
        // enter locking scope
        QMutexLocker lock(&m_qMutex);

        bool modified = compareAndSet(
                &m_record.refMetadataSynchronized(),
                !metadataSynchronized.isNull());
        bool modifiedReplayGain = false;
        if (m_record.getMetadata() != trackMetadata) {
            modifiedReplayGain =
                    (m_record.getMetadata().getTrackInfo().getReplayGain() != newReplayGain);
            m_record.setMetadata(std::move(trackMetadata));
            // Don't use trackMetadata after move assignment!!
            modified = true;
        }
        if (modified) {
            // explicitly unlock before emitting signals
            markDirtyAndUnlock(&lock);
            if (modifiedReplayGain) {
                emit(ReplayGainUpdated(newReplayGain));
            }
        }
        // implicitly unlocked when leaving scope
    }

    // Need to set BPM after sample rate since beat grid creation depends on
    // knowing the sample rate. Bug #1020438.
    if (newBpm.hasValue() &&
            ((nullptr == m_pBeats) || !mixxx::Bpm::isValidValue(m_pBeats->getBpm()))) {
        // Only (re-)set the BPM to the new value if the beat grid is not valid.
        // Reason: The BPM value in the metadata might be normalized or rounded,
        // e.g. ID3v2 only supports integer values!
        setBpm(newBpm.getValue());
    }

    if (!newKey.isEmpty()) {
        setKeyText(newKey, mixxx::track::io::key::FILE_METADATA);
    }
}

void Track::getTrackMetadata(
        mixxx::TrackMetadata* pTrackMetadata,
        bool* pMetadataSynchronized,
        bool* pDirty) const {
    DEBUG_ASSERT(pTrackMetadata);
    QMutexLocker lock(&m_qMutex);
    *pTrackMetadata = m_record.getMetadata();
    if (pMetadataSynchronized != nullptr) {
        *pMetadataSynchronized = m_record.getMetadataSynchronized();
    }
    if (pDirty != nullptr) {
        *pDirty = m_bDirty;
    }
}

void Track::getTrackRecord(
        mixxx::TrackRecord* pTrackRecord,
        bool* pDirty) const {
    DEBUG_ASSERT(pTrackRecord);
    QMutexLocker lock(&m_qMutex);
    *pTrackRecord = m_record;
    if (pDirty != nullptr) {
        *pDirty = m_bDirty;
    }
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
    return m_record.getMetadata().getTrackInfo().getReplayGain();
}

void Track::setReplayGain(const mixxx::ReplayGain& replayGain) {
    QMutexLocker lock(&m_qMutex);
    if (compareAndSet(&m_record.refMetadata().refTrackInfo().refReplayGain(), replayGain)) {
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
        BeatsPointer pBeats(BeatFactory::makeBeatGrid(*this, bpmValue, cue));
        setBeatsAndUnlock(&lock, pBeats);
        return bpmValue;
    }

    // Continue with the regular case
    if (m_pBeats->getBpm() != bpmValue) {
        kLogger.debug() << "Updating BPM:" << getLocation();
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

    m_record.refMetadata().refTrackInfo().setBpm(bpm);

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
    m_record.refMetadata().refTrackInfo().setBpm(bpm);
    markDirtyAndUnlock(&lock);
    emit(bpmUpdated(bpmValue));
    emit(beatsUpdated());
}

void Track::setMetadataSynchronized(bool metadataSynchronized) {
    QMutexLocker lock(&m_qMutex);
    if (compareAndSet(&m_record.refMetadataSynchronized(), metadataSynchronized)) {
        markDirtyAndUnlock(&lock);
    }
}

bool Track::isMetadataSynchronized() const {
    QMutexLocker lock(&m_qMutex);
    return m_record.getMetadataSynchronized();
}

QString Track::getInfo() const {
    QMutexLocker lock(&m_qMutex);
    if (m_record.getMetadata().getTrackInfo().getArtist().trimmed().isEmpty()) {
        return m_record.getMetadata().getTrackInfo().getTitle();
    } else {
        return m_record.getMetadata().getTrackInfo().getArtist() + ", " + m_record.getMetadata().getTrackInfo().getTitle();
    }
}

QDateTime Track::getDateAdded() const {
    QMutexLocker lock(&m_qMutex);
    return m_record.getDateAdded();
}

void Track::setDateAdded(const QDateTime& dateAdded) {
    QMutexLocker lock(&m_qMutex);
    return m_record.setDateAdded(dateAdded);
}

void Track::setDuration(mixxx::Duration duration) {
    QMutexLocker lock(&m_qMutex);
    if (compareAndSet(&m_record.refMetadata().refTrackInfo().refDuration(), duration)) {
        markDirtyAndUnlock(&lock);
    }
}

void Track::setDuration(double duration) {
    setDuration(mixxx::Duration::fromNanos(duration * 1000000000L));
}

double Track::getDuration(DurationRounding rounding) const {
    QMutexLocker lock(&m_qMutex);
    switch (rounding) {
    case DurationRounding::SECONDS:
        return std::round(m_record.getMetadata().getTrackInfo().getDuration().toDoubleSeconds());
    default:
        return m_record.getMetadata().getTrackInfo().getDuration().toDoubleSeconds();
    }
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
    return m_record.getMetadata().getTrackInfo().getTitle();
}

void Track::setTitle(const QString& s) {
    QMutexLocker lock(&m_qMutex);
    QString trimmed(s.trimmed());
    if (compareAndSet(&m_record.refMetadata().refTrackInfo().refTitle(), trimmed)) {
        markDirtyAndUnlock(&lock);
    }
}

QString Track::getArtist() const {
    QMutexLocker lock(&m_qMutex);
    return m_record.getMetadata().getTrackInfo().getArtist();
}

void Track::setArtist(const QString& s) {
    QMutexLocker lock(&m_qMutex);
    QString trimmed(s.trimmed());
    if (compareAndSet(&m_record.refMetadata().refTrackInfo().refArtist(), trimmed)) {
        markDirtyAndUnlock(&lock);
    }
}

QString Track::getAlbum() const {
    QMutexLocker lock(&m_qMutex);
    return m_record.getMetadata().getAlbumInfo().getTitle();
}

void Track::setAlbum(const QString& s) {
    QMutexLocker lock(&m_qMutex);
    QString trimmed(s.trimmed());
    if (compareAndSet(&m_record.refMetadata().refAlbumInfo().refTitle(), trimmed)) {
        markDirtyAndUnlock(&lock);
    }
}

QString Track::getAlbumArtist()  const {
    QMutexLocker lock(&m_qMutex);
    return m_record.getMetadata().getAlbumInfo().getArtist();
}

void Track::setAlbumArtist(const QString& s) {
    QMutexLocker lock(&m_qMutex);
    QString trimmed(s.trimmed());
    if (compareAndSet(&m_record.refMetadata().refAlbumInfo().refArtist(), trimmed)) {
        markDirtyAndUnlock(&lock);
    }
}

QString Track::getYear()  const {
    QMutexLocker lock(&m_qMutex);
    return m_record.getMetadata().getTrackInfo().getYear();
}

void Track::setYear(const QString& s) {
    QMutexLocker lock(&m_qMutex);
    QString trimmed(s.trimmed());
    if (compareAndSet(&m_record.refMetadata().refTrackInfo().refYear(), trimmed)) {
        markDirtyAndUnlock(&lock);
    }
}

QString Track::getGenre() const {
    QMutexLocker lock(&m_qMutex);
    return m_record.getMetadata().getTrackInfo().getGenre();
}

void Track::setGenre(const QString& s) {
    QMutexLocker lock(&m_qMutex);
    QString trimmed(s.trimmed());
    if (compareAndSet(&m_record.refMetadata().refTrackInfo().refGenre(), trimmed)) {
        markDirtyAndUnlock(&lock);
    }
}

QString Track::getComposer() const {
    QMutexLocker lock(&m_qMutex);
    return m_record.getMetadata().getTrackInfo().getComposer();
}

void Track::setComposer(const QString& s) {
    QMutexLocker lock(&m_qMutex);
    QString trimmed(s.trimmed());
    if (compareAndSet(&m_record.refMetadata().refTrackInfo().refComposer(), trimmed)) {
        markDirtyAndUnlock(&lock);
    }
}

QString Track::getGrouping()  const {
    QMutexLocker lock(&m_qMutex);
    return m_record.getMetadata().getTrackInfo().getGrouping();
}

void Track::setGrouping(const QString& s) {
    QMutexLocker lock(&m_qMutex);
    QString trimmed(s.trimmed());
    if (compareAndSet(&m_record.refMetadata().refTrackInfo().refGrouping(), trimmed)) {
        markDirtyAndUnlock(&lock);
    }
}

QString Track::getTrackNumber()  const {
    QMutexLocker lock(&m_qMutex);
    return m_record.getMetadata().getTrackInfo().getTrackNumber();
}

QString Track::getTrackTotal()  const {
    QMutexLocker lock(&m_qMutex);
    return m_record.getMetadata().getTrackInfo().getTrackTotal();
}

void Track::setTrackNumber(const QString& s) {
    QMutexLocker lock(&m_qMutex);
    QString trimmed(s.trimmed());
    if (compareAndSet(&m_record.refMetadata().refTrackInfo().refTrackNumber(), trimmed)) {
        markDirtyAndUnlock(&lock);
    }
}

void Track::setTrackTotal(const QString& s) {
    QMutexLocker lock(&m_qMutex);
    QString trimmed(s.trimmed());
    if (compareAndSet(&m_record.refMetadata().refTrackInfo().refTrackTotal(), trimmed)) {
        markDirtyAndUnlock(&lock);
    }
}

PlayCounter Track::getPlayCounter() const {
    QMutexLocker lock(&m_qMutex);
    return m_record.getPlayCounter();
}

void Track::setPlayCounter(const PlayCounter& playCounter) {
    QMutexLocker lock(&m_qMutex);
    if (compareAndSet(&m_record.refPlayCounter(), playCounter)) {
        markDirtyAndUnlock(&lock);
    }
}

void Track::updatePlayCounter(bool bPlayed) {
    QMutexLocker lock(&m_qMutex);
    PlayCounter playCounter(m_record.getPlayCounter());
    playCounter.setPlayedAndUpdateTimesPlayed(bPlayed);
    if (compareAndSet(&m_record.refPlayCounter(), playCounter)) {
        markDirtyAndUnlock(&lock);
    }
}

QString Track::getComment() const {
    QMutexLocker lock(&m_qMutex);
    return m_record.getMetadata().getTrackInfo().getComment();
}

void Track::setComment(const QString& s) {
    QMutexLocker lock(&m_qMutex);
    if (compareAndSet(&m_record.refMetadata().refTrackInfo().refComment(), s)) {
        markDirtyAndUnlock(&lock);
    }
}

QString Track::getType() const {
    QMutexLocker lock(&m_qMutex);
    return m_record.getFileType();
}

void Track::setType(const QString& sType) {
    QMutexLocker lock(&m_qMutex);
    if (compareAndSet(&m_record.refFileType(), sType)) {
        markDirtyAndUnlock(&lock);
    }
}

void Track::setSampleRate(int iSampleRate) {
    QMutexLocker lock(&m_qMutex);
    if (compareAndSet(&m_record.refMetadata().refTrackInfo().refSampleRate(), mixxx::AudioSignal::SampleRate(iSampleRate))) {
        markDirtyAndUnlock(&lock);
    }
}

int Track::getSampleRate() const {
    QMutexLocker lock(&m_qMutex);
    return m_record.getMetadata().getTrackInfo().getSampleRate();
}

void Track::setChannels(int iChannels) {
    QMutexLocker lock(&m_qMutex);
    if (compareAndSet(&m_record.refMetadata().refTrackInfo().refChannels(), mixxx::AudioSignal::ChannelCount(iChannels))) {
        markDirtyAndUnlock(&lock);
    }
}

int Track::getChannels() const {
    QMutexLocker lock(&m_qMutex);
    return m_record.getMetadata().getTrackInfo().getChannels();
}

int Track::getBitrate() const {
    QMutexLocker lock(&m_qMutex);
    return m_record.getMetadata().getTrackInfo().getBitrate();
}

QString Track::getBitrateText() const {
    return QString("%1").arg(getBitrate());
}

void Track::setBitrate(int iBitrate) {
    QMutexLocker lock(&m_qMutex);
    if (compareAndSet(&m_record.refMetadata().refTrackInfo().refBitrate(), mixxx::AudioSource::Bitrate(iBitrate))) {
        markDirtyAndUnlock(&lock);
    }
}

TrackId Track::getId() const {
    QMutexLocker lock(&m_qMutex);
    return m_record.getId();
}

void Track::initId(TrackId id) {
    QMutexLocker lock(&m_qMutex);
    // The track's id must be set only once and immediately after
    // the object has been created.
    VERIFY_OR_DEBUG_ASSERT(!m_record.getId().isValid() || (m_record.getId() == id)) {
        kLogger.warning() << "Cannot change id from"
                << m_record.getId() << "to" << id;
        return; // abort
    }
    m_record.setId(std::move(id));
    // Changing the Id does not make the track dirty because the Id is always
    // generated by the Database itself.
}

void Track::setURL(const QString& url) {
    QMutexLocker lock(&m_qMutex);
    if (compareAndSet(&m_record.refUrl(), url)) {
        markDirtyAndUnlock(&lock);
    }
}

QString Track::getURL() const {
    QMutexLocker lock(&m_qMutex);
    return m_record.getUrl();
}

ConstWaveformPointer Track::getWaveform() const {
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

void Track::setCuePoint(double cue) {
    QMutexLocker lock(&m_qMutex);
    if (compareAndSet(&m_record.refCuePoint(), cue)) {
        // Store the cue point in a load cue
        CuePointer pLoadCue;
        for (const CuePointer& pCue: m_cuePoints) {
            if (pCue->getType() == Cue::LOAD) {
                pLoadCue = pCue;
                break;
            }
        }
        if (cue > 0) {
            if (!pLoadCue) {
                pLoadCue = CuePointer(new Cue(m_record.getId()));
                pLoadCue->setType(Cue::LOAD);
                connect(pLoadCue.get(), SIGNAL(updated()),
                        this, SLOT(slotCueUpdated()));
                m_cuePoints.push_back(pLoadCue);
            }
            pLoadCue->setPosition(cue);
        } else {
            disconnect(pLoadCue.get(), 0, this, 0);
            m_cuePoints.removeOne(pLoadCue);
        }
        markDirtyAndUnlock(&lock);
        emit(cuesUpdated());
    }
}

double Track::getCuePoint() const {
    QMutexLocker lock(&m_qMutex);
    return m_record.getCuePoint();
}

void Track::slotCueUpdated() {
    markDirty();
    emit(cuesUpdated());
}

CuePointer Track::createAndAddCue() {
    QMutexLocker lock(&m_qMutex);
    CuePointer pCue(new Cue(m_record.getId()));
    connect(pCue.get(), SIGNAL(updated()),
            this, SLOT(slotCueUpdated()));
    m_cuePoints.push_back(pCue);
    markDirtyAndUnlock(&lock);
    emit(cuesUpdated());
    return pCue;
}

void Track::removeCue(const CuePointer& pCue) {
    QMutexLocker lock(&m_qMutex);
    disconnect(pCue.get(), 0, this, 0);
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
        disconnect(pCue.get(), 0, this, 0);
    }
    m_cuePoints = cuePoints;
    // connect new cue points
    for (const auto& pCue: m_cuePoints) {
        connect(pCue.get(), SIGNAL(updated()),
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


void Track::markForMetadataExport() {
    QMutexLocker lock(&m_qMutex);
    if (compareAndSet(&m_bExportMetadata, true)) {
        markDirtyAndUnlock(&lock);
    }
}

int Track::getRating() const {
    QMutexLocker lock(&m_qMutex);
    return m_record.getRating();
}

void Track::setRating (int rating) {
    QMutexLocker lock(&m_qMutex);
    if (compareAndSet(&m_record.refRating(), rating)) {
        markDirtyAndUnlock(&lock);
    }
}

void Track::afterKeysUpdated(QMutexLocker* pLock) {
    // New key might be INVALID. We don't care.
    mixxx::track::io::key::ChromaticKey newKey = m_record.getGlobalKey();
    markDirtyAndUnlock(pLock);
    emit(keyUpdated(KeyUtils::keyToNumericValue(newKey)));
    emit(keysUpdated());
}

void Track::setKeys(const Keys& keys) {
    QMutexLocker lock(&m_qMutex);
    m_record.setKeys(keys);
    afterKeysUpdated(&lock);
}

void Track::resetKeys() {
    QMutexLocker lock(&m_qMutex);
    m_record.resetKeys();
    afterKeysUpdated(&lock);
}

Keys Track::getKeys() const {
    QMutexLocker lock(&m_qMutex);
    return m_record.getKeys();
}

void Track::setKey(mixxx::track::io::key::ChromaticKey key,
                   mixxx::track::io::key::Source keySource) {
    QMutexLocker lock(&m_qMutex);
    if (m_record.updateGlobalKey(key, keySource)) {
        afterKeysUpdated(&lock);
    }
}

mixxx::track::io::key::ChromaticKey Track::getKey() const {
    QMutexLocker lock(&m_qMutex);
    return m_record.getGlobalKey();
}

QString Track::getKeyText() const {
    QMutexLocker lock(&m_qMutex);
    return m_record.getGlobalKeyText();
}

void Track::setKeyText(const QString& keyText,
                       mixxx::track::io::key::Source keySource) {
    QMutexLocker lock(&m_qMutex);
    if (m_record.updateGlobalKeyText(keyText, keySource)) {
        afterKeysUpdated(&lock);
    }
}

void Track::setBpmLocked(bool bpmLocked) {
    QMutexLocker lock(&m_qMutex);
    if (compareAndSet(&m_record.refBpmLocked(), bpmLocked)) {
        markDirtyAndUnlock(&lock);
    }
}

bool Track::isBpmLocked() const {
    QMutexLocker lock(&m_qMutex);
    return m_record.getBpmLocked();
}

void Track::setCoverInfo(const CoverInfoRelative& coverInfoRelative) {
    QMutexLocker lock(&m_qMutex);
    if (compareAndSet(&m_record.refCoverInfo(), coverInfoRelative)) {
        markDirtyAndUnlock(&lock);
        emit(coverArtUpdated());
    }
}

void Track::setCoverInfo(const CoverInfo& coverInfo) {
    CoverInfoRelative coverInfoRelative(coverInfo);
    QMutexLocker lock(&m_qMutex);
    DEBUG_ASSERT(coverInfo.trackLocation == m_fileInfo.absoluteFilePath());
    if (compareAndSet(&m_record.refCoverInfo(), coverInfoRelative)) {
        markDirtyAndUnlock(&lock);
        emit(coverArtUpdated());
    }
}

void Track::setCoverInfo(const CoverArt& coverArt) {
    setCoverInfo(static_cast<const CoverInfo&>(coverArt));
}

CoverInfo Track::getCoverInfo() const {
    QMutexLocker lock(&m_qMutex);
    return CoverInfo(m_record.getCoverInfo(), m_fileInfo.absoluteFilePath());
}

quint16 Track::getCoverHash() const {
    QMutexLocker lock(&m_qMutex);
    return m_record.getCoverInfo().hash;
}

Track::ExportMetadataResult Track::exportMetadata(
        mixxx::MetadataSourcePointer pMetadataSource) {
    VERIFY_OR_DEBUG_ASSERT(pMetadataSource) {
        kLogger.warning()
                << "Cannot export track metadata:"
                << getLocation();
        return ExportMetadataResult::Failed;
    }
    // Locking shouldn't be necessary here, because this function will
    // be called after all references to the object have been dropped.
    // But it doesn't hurt much, so let's play it safe ;)
    QMutexLocker lock(&m_qMutex);
    if (!m_bExportMetadata) {
        // Perform some consistency checks if metadata is exported
        // implicitly after a track has been modified and has NOT
        // been explicitly requested by a user as indicated by this
        // flag.
        if (!m_record.getMetadataSynchronized()) {
            kLogger.debug()
                    << "Skip exporting of unsynchronized track metadata:"
                    << getLocation();
            return ExportMetadataResult::Skipped;
        }
        // Check if the metadata has actually been modified. Otherwise
        // we don't need to write it back. Exporting unmodified metadata
        // would needlessly update the file's time stamp and should be
        // avoided.
        // TODO(XXX): How to we handle the case that importTrackMetadataAndCoverImage()
        // returns a newer time stamp than m_record.getMetadataSynchronized(), i.e.
        // if the file has been modified by another program since we have imported
        // the metadata? But this is expected to happen if files have been copied
        // or after upgrading the column 'header_parsed' from bool to QDateTime.
        mixxx::TrackMetadata trackMetadata;
        if ((pMetadataSource->importTrackMetadataAndCoverImage(&trackMetadata, nullptr).first ==
                mixxx::MetadataSource::ImportResult::Succeeded) &&
                (m_record.getMetadata() == trackMetadata))  {
            kLogger.debug()
                    << "Skip exporting of unmodified track metadata:"
                    << getLocation();
            return ExportMetadataResult::Skipped;
        }
    }
    m_bExportMetadata = false; // reset flag
    const auto trackMetadataExported =
            pMetadataSource->exportTrackMetadata(m_record.getMetadata());
    if (trackMetadataExported.first == mixxx::MetadataSource::ExportResult::Succeeded) {
        // TODO(XXX): Replace bool with QDateTime
        DEBUG_ASSERT(!trackMetadataExported.second.isNull());
        //pTrack->setMetadataSynchronized(trackMetadataExported.second);
        m_record.setMetadataSynchronized(!trackMetadataExported.second.isNull());
        kLogger.debug()
                << "Exported track metadata:"
                << getLocation();
        return ExportMetadataResult::Succeeded;
    } else {
        kLogger.warning()
                << "Failed to export track metadata:"
                << getLocation();
        return ExportMetadataResult::Failed;
    }
}
