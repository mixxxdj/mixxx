#include <QDirIterator>
#include <QFile>
#include <QMutexLocker>

#include <atomic>

#include "track/track.h"
#include "track/trackref.h"
#include "track/beatfactory.h"

#include "util/assert.h"
#include "util/logger.h"


namespace {

const mixxx::Logger kLogger("Track");

constexpr bool kLogStats = false;

// Count the number of currently existing instances for detecting
// memory leaks.
std::atomic<int> s_numberOfInstances;

SecurityTokenPointer openSecurityToken(
        const QFileInfo& fileInfo,
        SecurityTokenPointer pSecurityToken = SecurityTokenPointer()) {
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

inline
mixxx::Bpm getActualBpm(
        mixxx::Bpm bpm,
        BeatsPointer pBeats = BeatsPointer()) {
    // Only use the imported BPM if the beat grid is not valid!
    // Reason: The BPM value in the metadata might be normalized
    // or rounded, e.g. ID3v2 only supports integer values.
    if (pBeats) {
        return mixxx::Bpm(pBeats->getBpm());
    } else {
        return bpm;
    }
}

} // anonymous namespace

Track::Track(
        QFileInfo fileInfo,
        SecurityTokenPointer pSecurityToken,
        TrackId trackId)
        : m_qMutex(QMutex::Recursive),
          m_fileInfo(std::move(fileInfo)),
          m_pSecurityToken(openSecurityToken(m_fileInfo, std::move(pSecurityToken))),
          m_record(trackId),
          m_bDirty(false),
          m_bMarkedForMetadataExport(false) {
    if (kLogStats && kLogger.debugEnabled()) {
        long numberOfInstancesBefore = s_numberOfInstances.fetch_add(1);
        kLogger.debug()
                << "Creating instance:"
                << this
                << numberOfInstancesBefore
                << "->"
                << numberOfInstancesBefore + 1;
    }
}

Track::~Track() {
    if (kLogStats && kLogger.debugEnabled()) {
        long numberOfInstancesBefore = s_numberOfInstances.fetch_sub(1);
        kLogger.debug()
                << "Destroying instance:"
                << this
                << numberOfInstancesBefore
                << "->"
                << numberOfInstancesBefore - 1;
    }
}

//static
TrackPointer Track::newTemporary(
        QFileInfo fileInfo,
        SecurityTokenPointer pSecurityToken) {
    return std::make_shared<Track>(
            std::move(fileInfo),
            std::move(pSecurityToken));
}

//static
TrackPointer Track::newDummy(
        QFileInfo fileInfo,
        TrackId trackId) {
    return std::make_shared<Track>(
            std::move(fileInfo),
            SecurityTokenPointer(),
            trackId);
}

void Track::relocate(
        QFileInfo fileInfo,
        SecurityTokenPointer pSecurityToken) {
    QMutexLocker lock(&m_qMutex);
    m_fileInfo = fileInfo;
    m_pSecurityToken = pSecurityToken;
    // The track does not need to be marked as dirty,
    // because this function will always be called with
    // the updated location from the database.
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
    const auto actualBpm = getActualBpm(newBpm, m_pBeats);
    if (actualBpm.hasValue()) {
        setBpm(actualBpm.getValue());
    }

    if (!newKey.isEmpty()) {
        setKeyText(newKey, mixxx::track::io::key::FILE_METADATA);
    }
}

void Track::getTrackMetadata(
        mixxx::TrackMetadata* pTrackMetadata,
        bool* pMetadataSynchronized) const {
    DEBUG_ASSERT(pTrackMetadata);
    QMutexLocker lock(&m_qMutex);
    *pTrackMetadata = m_record.getMetadata();
    if (pMetadataSynchronized != nullptr) {
        *pMetadataSynchronized = m_record.getMetadataSynchronized();
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
    return TrackRef::location(m_fileInfo);
}

QString Track::getCanonicalLocation() const {
    // Copying QFileInfo is thread-safe due to "implicit sharing"
    // (copy-on write). But operating on a single instance of QFileInfo
    // might not be thread-safe due to internal caching!
    QMutexLocker lock(&m_qMutex);

    // Note: We return here the cached value, that was calculated just after 
    // init this Track object. This will avoid repeated use of the time 
    // consuming file IO.
    // We ignore the case when the user changes a symbolic link to 
    // point a file to an other location, since this is a user action.
    // We also don't care if a file disappears while Mixxx is running. Opening 
    // a non-existent file is already handled and doesn't cause any malfunction.
    QString loc = TrackRef::canonicalLocation(m_fileInfo);
    if (loc.isEmpty()) {
        // we see here an empty path because the file did not exist  
        // when creating the track object.
        // The user might have restored the track in the meanwhile.
        // So try again it again. 
        m_fileInfo.refresh();
        loc = TrackRef::canonicalLocation(m_fileInfo);
    }
    return loc;
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
        if (kLogger.debugEnabled()) {
            kLogger.debug() << "Updating BPM:" << getLocation();
        }
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

    if (m_pBeats) {
        auto pObject = dynamic_cast<QObject*>(m_pBeats.data());
        if (pObject) {
            disconnect(pObject, SIGNAL(updated()),
                       this, SLOT(slotBeatsUpdated()));
        }
    }

    m_pBeats = pBeats;

    auto bpmValue = mixxx::Bpm::kValueUndefined;
    if (m_pBeats) {
        bpmValue = m_pBeats->getBpm();
        auto pObject = dynamic_cast<QObject*>(m_pBeats.data());
        if (pObject) {
            connect(pObject, SIGNAL(updated()),
                    this, SLOT(slotBeatsUpdated()));
        }
    }
    m_record.refMetadata().refTrackInfo().setBpm(mixxx::Bpm(bpmValue));

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

    auto bpmValue = mixxx::Bpm::kValueUndefined;
    if (m_pBeats) {
        bpmValue = m_pBeats->getBpm();
    }
    m_record.refMetadata().refTrackInfo().setBpm(mixxx::Bpm(bpmValue));

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
    if (compareAndSet(&m_record.refMetadata().refDuration(), duration)) {
        markDirtyAndUnlock(&lock);
    }
}

void Track::setDuration(double duration) {
    setDuration(mixxx::Duration::fromSeconds(duration));
}

double Track::getDuration(DurationRounding rounding) const {
    QMutexLocker lock(&m_qMutex);
    switch (rounding) {
    case DurationRounding::SECONDS:
        return std::round(m_record.getMetadata().getDuration().toDoubleSeconds());
    default:
        return m_record.getMetadata().getDuration().toDoubleSeconds();
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
    return mixxx::Duration::formatTime(duration, precision);
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
    if (compareAndSet(&m_record.refMetadata().refSampleRate(), mixxx::AudioSignal::SampleRate(iSampleRate))) {
        markDirtyAndUnlock(&lock);
    }
}

int Track::getSampleRate() const {
    QMutexLocker lock(&m_qMutex);
    return m_record.getMetadata().getSampleRate();
}

void Track::setChannels(int iChannels) {
    QMutexLocker lock(&m_qMutex);
    if (compareAndSet(&m_record.refMetadata().refChannels(), mixxx::AudioSignal::ChannelCount(iChannels))) {
        markDirtyAndUnlock(&lock);
    }
}

int Track::getChannels() const {
    QMutexLocker lock(&m_qMutex);
    return m_record.getMetadata().getChannels();
}

int Track::getBitrate() const {
    QMutexLocker lock(&m_qMutex);
    return m_record.getMetadata().getBitrate();
}

QString Track::getBitrateText() const {
    return QString("%1").arg(getBitrate());
}

void Track::setBitrate(int iBitrate) {
    QMutexLocker lock(&m_qMutex);
    if (compareAndSet(&m_record.refMetadata().refBitrate(), mixxx::AudioSource::Bitrate(iBitrate))) {
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

void Track::removeCuesOfType(Cue::CueType type) {
    QMutexLocker lock(&m_qMutex);
    bool dirty = false;
    QMutableListIterator<CuePointer> it(m_cuePoints);
    while (it.hasNext()) {
        CuePointer pCue = it.next();
        // FIXME: Why does this only work for the CUE CueType?
        if (pCue->getType() == type) {
            disconnect(pCue.get(), 0, this, 0);
            it.remove();
            dirty = true;
        }
    }
    if (dirty) {
        markDirtyAndUnlock(&lock);
        emit(cuesUpdated());
    }
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
    m_bMarkedForMetadataExport = true;
    // No need to mark the track as dirty, because this flag
    // is transient and not stored in the database.
}

bool Track::isMarkedForMetadataExport() const {
    QMutexLocker lock(&m_qMutex);
    return m_bMarkedForMetadataExport;
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

void Track::setCoverInfo(const CoverInfoRelative& coverInfo) {
    DEBUG_ASSERT((coverInfo.type != CoverInfo::METADATA) || coverInfo.coverLocation.isEmpty());
    DEBUG_ASSERT((coverInfo.source != CoverInfo::UNKNOWN) || (coverInfo.type == CoverInfo::NONE));
    QMutexLocker lock(&m_qMutex);
    if (compareAndSet(&m_record.refCoverInfo(), coverInfo)) {
        markDirtyAndUnlock(&lock);
        emit(coverArtUpdated());
    }
}

CoverInfoRelative Track::getCoverInfo() const {
    QMutexLocker lock(&m_qMutex);
    return m_record.getCoverInfo();
}

CoverInfo Track::getCoverInfoWithLocation() const {
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
    // Discard the values of all currently unsupported fields that are
    // not stored in the library, yet. Those fields are already imported
    // from file tags, but the database schema needs to be extended for
    // storing them. Currently those fields will be empty/null when read
    // from the database and must be ignored until the schema has been
    // updated.
    m_record.refMetadata().resetUnsupportedValues();
    // Normalize metadata before exporting to adjust the precision of
    // floating values, ... Otherwise the following comparisons may
    // repeatedly indicate that values have changed only due to
    // rounding errors.
    m_record.refMetadata().normalizeBeforeExport();
    if (!m_bMarkedForMetadataExport) {
        // Perform some consistency checks if metadata is exported
        // implicitly after a track has been modified and NOT explicitly
        // requested by a user as indicated by this flag.
        if (!m_record.getMetadataSynchronized()) {
            // If the metadata has never been imported from file tags it
            // must be exported explicitly once. This ensures that we don't
            // overwrite existing file tags with completely different
            // information.
            kLogger.info()
                    << "Skip exporting of unsynchronized track metadata:"
                    << getLocation();
            return ExportMetadataResult::Skipped;
        }
        // Check if the metadata has actually been modified. Otherwise
        // we don't need to write it back. Exporting unmodified metadata
        // would needlessly update the file's time stamp and should be
        // avoided. Since we don't know in which state the file's metadata
        // is we import it again into a temporary variable.
        // TODO(XXX): m_record.getMetadataSynchronized() currently is a
        // boolean flag, but it should become a time stamp in the future.
        // We could take this time stamp and the file's last modification
        // time stamp into // account and might decide to skip importing
        // the metadata again.
        mixxx::TrackMetadata importedFromFile;
        if ((pMetadataSource->importTrackMetadataAndCoverImage(&importedFromFile, nullptr).first ==
                mixxx::MetadataSource::ImportResult::Succeeded)) {
            // Discard the values of all currently unsupported fields that are
            // not stored in the library, yet. We have done the same with the track's
            // current metadata to make the tags comparable (see above).
            importedFromFile.resetUnsupportedValues();
            // Before comparison: Adjust imported bpm values that might be imprecise,
            // e.g. integer values from ID3v2. The same strategy has is used when
            // importing the track's metadata in order to preserve the more accurate
            // bpm value stored by Mixxx. Again, this is necessary to make the tags
            // comparable.
            auto actualBpm =
                    getActualBpm(importedFromFile.getTrackInfo().getBpm(), m_pBeats);
            // All imported floating point values are already properly rounded, because
            // they have just been imported. But the imported bpm value might have been
            // replaced by a more accurate bpm value, that also needs to be rounded before
            // comparison.
            actualBpm.normalizeBeforeExport();
            // Replace the imported with the actual bpm value managed by Mixxx.
            importedFromFile.refTrackInfo().setBpm(actualBpm);
            // Finally the track's current metadata and the imported/adjusted metadata
            // can be compared for differences to decide whether the tags in the file
            // would change if we perform the write operation. We are using a special
            // comparison function that excludes all read-only audio properties which
            // are stored in file tags, but may not be accurate. They can't be written
            // anyway, so we must not take them into account here.
            if (!m_record.getMetadata().hasBeenModifiedAfterImport(importedFromFile))  {
                // The file tags are in-sync with the track's metadata and don't need
                // to be updated.
                if (kLogger.debugEnabled()) {
                    kLogger.debug()
                                << "Skip exporting of unmodified track metadata into file:"
                                << getLocation();
                }
                return ExportMetadataResult::Skipped;
            }
        } else {
            // Something must be wrong with the file or it doesn't
            // contain any file tags. We don't want to risk a failure
            // during export and abort the operation for safety here.
            // The user may decide to explicitly export the metadata.
            kLogger.warning()
                    << "Skip exporting of track metadata after import failed."
                    << "Export of metadata must be triggered explicitly for this file:"
                    << getLocation();
            return ExportMetadataResult::Skipped;
        }
        // ...by continuing the file tags will be updated
    }
    // The track's metadata will be exported instantly. The export should
    // only be tried once so we reset the marker flag.
    m_bMarkedForMetadataExport = false;
    const auto trackMetadataExported =
            pMetadataSource->exportTrackMetadata(m_record.getMetadata());
    if (trackMetadataExported.first == mixxx::MetadataSource::ExportResult::Succeeded) {
        // After successfully exporting the metadata we record the fact
        // that now the file tags and the track's metadata are in sync.
        // This information (flag or time stamp) is stored in the database.
        // The database update will follow immediately after returning from
        // this operation!
        // TODO(XXX): Replace bool with QDateTime
        DEBUG_ASSERT(!trackMetadataExported.second.isNull());
        //pTrack->setMetadataSynchronized(trackMetadataExported.second);
        m_record.setMetadataSynchronized(!trackMetadataExported.second.isNull());
        if (kLogger.debugEnabled()) {
            kLogger.debug()
                    << "Exported track metadata:"
                    << getLocation();
        }
        return ExportMetadataResult::Succeeded;
    } else {
        kLogger.warning()
                << "Failed to export track metadata:"
                << getLocation();
        return ExportMetadataResult::Failed;
    }
}
