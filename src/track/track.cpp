#include "track/track.h"

#include <QDirIterator>
#include <atomic>

#include "engine/engine.h"
#include "moc_track.cpp"
#include "track/beatfactory.h"
#include "track/beatmap.h"
#include "track/trackref.h"
#include "util/assert.h"
#include "util/color/color.h"
#include "util/logger.h"

namespace {

const mixxx::Logger kLogger("Track");

constexpr bool kLogStats = false;
const ConfigKey kConfigKeySeratoMetadataExport("[Library]", "SeratoMetadataExport");

// Count the number of currently existing instances for detecting
// memory leaks.
std::atomic<int> s_numberOfInstances;

SecurityTokenPointer openSecurityToken(
        const TrackFile& trackFile,
        SecurityTokenPointer pSecurityToken = SecurityTokenPointer()) {
    if (pSecurityToken.isNull()) {
        return Sandbox::openSecurityToken(trackFile.asFileInfo(), true);
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

inline mixxx::Bpm getBeatsPointerBpm(
        const mixxx::BeatsPointer& pBeats) {
    return pBeats ? mixxx::Bpm{pBeats->getBpm()} : mixxx::Bpm{};
}

} // anonymous namespace

// Don't change this string without an entry in the CHANGELOG!
// Otherwise 3rd party software that picks up the currently
// playing track from the main window and relies on this
// formatting would stop working.
//static
const QString Track::kArtistTitleSeparator = QStringLiteral(" - ");

Track::Track(
        TrackFile fileInfo,
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
    if (m_pBeatsImporterPending && !m_pBeatsImporterPending->isEmpty()) {
        kLogger.warning()
                << "Import of beats is still pending and discarded";
    }
    if (m_pCueInfoImporterPending && !m_pCueInfoImporterPending->isEmpty()) {
        kLogger.warning()
                << "Import of"
                << m_pCueInfoImporterPending->size()
                << "cue(s) is still pending and discarded";
    }
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
        TrackFile fileInfo,
        SecurityTokenPointer pSecurityToken) {
    return std::make_shared<Track>(
            std::move(fileInfo),
            std::move(pSecurityToken));
}

//static
TrackPointer Track::newDummy(
        TrackFile fileInfo,
        TrackId trackId) {
    return std::make_shared<Track>(
            std::move(fileInfo),
            SecurityTokenPointer(),
            trackId);
}

void Track::relocate(
        TrackFile fileInfo,
        SecurityTokenPointer pSecurityToken) {
    QMutexLocker lock(&m_qMutex);
    m_pSecurityToken = openSecurityToken(fileInfo, std::move(pSecurityToken));
    m_fileInfo = std::move(fileInfo);
    // The track does not need to be marked as dirty,
    // because this function will always be called with
    // the updated location from the database.
}

void Track::importMetadata(
        mixxx::TrackMetadata importedMetadata,
        const QDateTime& metadataSynchronized) {
    // Information stored in Serato tags is imported separately after
    // importing the metadata (see below). The Serato tags BLOB itself
    // is updated together with the metadata.
    auto pSeratoBeatsImporter = importedMetadata.getTrackInfo().getSeratoTags().importBeats();
    const bool seratoBpmLocked = importedMetadata.getTrackInfo().getSeratoTags().isBpmLocked();
    auto pSeratoCuesImporter = importedMetadata.getTrackInfo().getSeratoTags().importCueInfos();

    {
        // enter locking scope
        QMutexLocker lock(&m_qMutex);

        // Preserve the both current bpm and key temporarily to avoid
        // overwriting with an inconsistent value. The bpm must always be
        // set together with the beat grid and the key text must be parsed
        // and validated.
        const auto importedBpm = importedMetadata.getTrackInfo().getBpm();
        importedMetadata.refTrackInfo().setBpm(getBpmWhileLocked());
        const auto importedKeyText = importedMetadata.getTrackInfo().getKey();
        importedMetadata.refTrackInfo().setKey(m_record.getMetadata().getTrackInfo().getKey());

        bool modified = false;
        // Only set the metadata synchronized flag (column `header_parsed`
        // in the database) from false to true, but never reset it back to
        // false. Otherwise file tags would be re-imported and overwrite
        // the metadata stored in the database, e.g. after retrieving metadata
        // from MusicBrainz!
        // TODO: In the future this flag should become a time stamp
        // to detect updates of files and then decide based on time
        // stamps if file tags need to be re-imported.
        if (!metadataSynchronized.isNull()) {
            modified |= compareAndSet(
                    m_record.ptrMetadataSynchronized(),
                    true);
        }

        const auto oldReplayGain =
                m_record.getMetadata().getTrackInfo().getReplayGain();
        if (m_record.getMetadata() != importedMetadata) {
            m_record.setMetadata(std::move(importedMetadata));
            // Don't use importedMetadata after move assignment!!
            modified = true;
        }
        const auto newReplayGain =
                m_record.getMetadata().getTrackInfo().getReplayGain();

        // Need to set BPM after sample rate since beat grid creation depends on
        // knowing the sample rate. Bug #1020438.
        auto beatsAndBpmModified = false;
        if (!m_pBeats || !mixxx::Bpm::isValidValue(m_pBeats->getBpm())) {
            // Only use the imported BPM if the current beat grid is either
            // missing or not valid! The BPM value in the metadata might be
            // imprecise (normalized or rounded), e.g. ID3v2 only supports
            // integer values.
            beatsAndBpmModified = trySetBpmWhileLocked(importedBpm.getValue());
        }
        modified |= beatsAndBpmModified;
        const auto newBpm = getBpmWhileLocked();

        auto keysModified = false;
        if (KeyUtils::guessKeyFromText(importedKeyText) != mixxx::track::io::key::INVALID) {
            // Only update the current key with a valid value. Otherwise preserve
            // the existing value.
            keysModified = m_record.updateGlobalKeyText(
                    importedKeyText,
                    mixxx::track::io::key::FILE_METADATA);
        }
        modified |= keysModified;
        const auto newKey = m_record.getGlobalKey();

        // Import track color from Serato tags if available
        const auto newColor = m_record.getMetadata().getTrackInfo().getSeratoTags().getTrackColor();
        const bool colorModified = compareAndSet(m_record.ptrColor(), newColor);
        modified |= colorModified;
        DEBUG_ASSERT(!colorModified || m_record.getColor() == newColor);

        if (!modified) {
            // Unmodified, nothing todo
            return;
        }
        // Explicitly unlock before emitting signals
        markDirtyAndUnlock(&lock);

        if (beatsAndBpmModified) {
            emitBeatsAndBpmUpdated(newBpm);
        }
        if (keysModified) {
            emitKeysUpdated(newKey);
        }
        if (oldReplayGain != newReplayGain) {
            emit replayGainUpdated(newReplayGain);
        }
        if (colorModified) {
            emit colorUpdated(newColor);
        }
    }

    // TODO: Import Serato metadata within the locking scope and not
    // as a post-processing step.
    if (pSeratoBeatsImporter) {
        kLogger.debug() << "Importing Serato beats";
        tryImportBeats(std::move(pSeratoBeatsImporter), seratoBpmLocked);
    }
    if (pSeratoCuesImporter) {
        kLogger.debug() << "Importing Serato cues";
        importCueInfos(std::move(pSeratoCuesImporter));
    }
}

void Track::mergeImportedMetadata(
        const mixxx::TrackMetadata& importedMetadata) {
    QMutexLocker lock(&m_qMutex);
    if (m_record.mergeImportedMetadata(importedMetadata)) {
        markDirtyAndUnlock(&lock);
    }
}

void Track::readTrackMetadata(
        mixxx::TrackMetadata* pTrackMetadata,
        bool* pMetadataSynchronized) const {
    DEBUG_ASSERT(pTrackMetadata);
    QMutexLocker lock(&m_qMutex);
    *pTrackMetadata = m_record.getMetadata();
    if (pMetadataSynchronized) {
        *pMetadataSynchronized = m_record.getMetadataSynchronized();
    }
}

void Track::readTrackRecord(
        mixxx::TrackRecord* pTrackRecord,
        bool* pDirty) const {
    DEBUG_ASSERT(pTrackRecord);
    QMutexLocker lock(&m_qMutex);
    *pTrackRecord = m_record;
    if (pDirty) {
        *pDirty = m_bDirty;
    }
}

QString Track::getCanonicalLocation() const {
    QMutexLocker lock(&m_qMutex);
    return /*mutable*/ m_fileInfo.freshCanonicalLocation(); // non-const
}

mixxx::ReplayGain Track::getReplayGain() const {
    QMutexLocker lock(&m_qMutex);
    return m_record.getMetadata().getTrackInfo().getReplayGain();
}

void Track::setReplayGain(const mixxx::ReplayGain& replayGain) {
    QMutexLocker lock(&m_qMutex);
    if (compareAndSet(m_record.refMetadata().refTrackInfo().ptrReplayGain(), replayGain)) {
        markDirtyAndUnlock(&lock);
        emit replayGainUpdated(replayGain);
    }
}

mixxx::Bpm Track::getBpmWhileLocked() const {
    // BPM values must be synchronized at all times!
    DEBUG_ASSERT(m_record.getMetadata().getTrackInfo().getBpm() == getBeatsPointerBpm(m_pBeats));
    return m_record.getMetadata().getTrackInfo().getBpm();
}

bool Track::trySetBpmWhileLocked(double bpmValue) {
    if (!mixxx::Bpm::isValidValue(bpmValue)) {
        // If the user sets the BPM to an invalid value, we assume
        // they want to clear the beatgrid.
        return trySetBeatsWhileLocked(nullptr);
    } else if (!m_pBeats) {
        // No beat grid available -> create and initialize
        double cue = m_record.getCuePoint().getPosition();
        auto pBeats = BeatFactory::makeBeatGrid(getSampleRate(), bpmValue, cue);
        return trySetBeatsWhileLocked(std::move(pBeats));
    } else if ((m_pBeats->getCapabilities() & mixxx::Beats::BEATSCAP_SETBPM) &&
            m_pBeats->getBpm() != bpmValue) {
        // Continue with the regular cases
        if (kLogger.debugEnabled()) {
            kLogger.debug() << "Updating BPM:" << getLocation();
        }
        return trySetBeatsWhileLocked(m_pBeats->setBpm(bpmValue));
    }
    return false;
}

QString Track::getBpmText() const {
    return QString("%1").arg(getBpm(), 3,'f',1);
}

bool Track::trySetBpm(double bpmValue) {
    QMutexLocker lock(&m_qMutex);
    if (!trySetBpmWhileLocked(bpmValue)) {
        return false;
    }
    afterBeatsAndBpmUpdated(&lock);
    return true;
}

bool Track::trySetBeats(mixxx::BeatsPointer pBeats) {
    QMutexLocker lock(&m_qMutex);
    return trySetBeatsMarkDirtyAndUnlock(&lock, pBeats, false);
}

bool Track::trySetAndLockBeats(mixxx::BeatsPointer pBeats) {
    QMutexLocker lock(&m_qMutex);
    return trySetBeatsMarkDirtyAndUnlock(&lock, pBeats, true);
}

bool Track::setBeatsWhileLocked(mixxx::BeatsPointer pBeats) {
    if (m_pBeats == pBeats) {
        return false;
    }
    m_pBeats = std::move(pBeats);
    m_record.refMetadata().refTrackInfo().setBpm(getBeatsPointerBpm(m_pBeats));
    return true;
}

bool Track::trySetBeatsWhileLocked(
        mixxx::BeatsPointer pBeats,
        bool lockBpmAfterSet) {
    if (m_pBeats && m_record.getBpmLocked()) {
        // Track has already a valid and locked beats object, abbort.
        qDebug() << "Track beats is already set and BPM-locked. Discard the new beats";
        return false;
    }

    bool dirty = false;
    if (setBeatsWhileLocked(pBeats)) {
        dirty = true;
    }
    if (compareAndSet(m_record.ptrBpmLocked(), lockBpmAfterSet)) {
        dirty = true;
    }
    return dirty;
}

bool Track::trySetBeatsMarkDirtyAndUnlock(
        QMutexLocker* pLock,
        mixxx::BeatsPointer pBeats,
        bool lockBpmAfterSet) {
    DEBUG_ASSERT(pLock);

    if (!trySetBeatsWhileLocked(pBeats, lockBpmAfterSet)) {
        return false;
    }

    afterBeatsAndBpmUpdated(pLock);
    return true;
}

mixxx::BeatsPointer Track::getBeats() const {
    QMutexLocker lock(&m_qMutex);
    return m_pBeats;
}

void Track::afterBeatsAndBpmUpdated(
        QMutexLocker* pLock) {
    DEBUG_ASSERT(pLock);

    const auto bpm = getBpmWhileLocked();
    markDirtyAndUnlock(pLock);
    emitBeatsAndBpmUpdated(bpm);
}

void Track::emitBeatsAndBpmUpdated(
        mixxx::Bpm newBpm) {
    emit bpmUpdated(newBpm.getValue());
    emit beatsUpdated();
}

void Track::setMetadataSynchronized(bool metadataSynchronized) {
    QMutexLocker lock(&m_qMutex);
    if (compareAndSet(m_record.ptrMetadataSynchronized(), metadataSynchronized)) {
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
        if (m_record.getMetadata().getTrackInfo().getTitle().trimmed().isEmpty()) {
            return m_fileInfo.fileName();
        } else {
            return m_record.getMetadata().getTrackInfo().getTitle();
        }
    } else {
        return m_record.getMetadata().getTrackInfo().getArtist() +
                kArtistTitleSeparator +
                m_record.getMetadata().getTrackInfo().getTitle();
    }
}

QString Track::getTitleInfo() const {
    QMutexLocker lock(&m_qMutex);
    if (m_record.getMetadata().getTrackInfo().getArtist().trimmed().isEmpty() &&
            m_record.getMetadata().getTrackInfo().getTitle().trimmed().isEmpty()) {
        return m_fileInfo.fileName();
    } else {
        return m_record.getMetadata().getTrackInfo().getTitle();
    }
}

QDateTime Track::getDateAdded() const {
    QMutexLocker lock(&m_qMutex);
    return m_record.getDateAdded();
}

void Track::setDateAdded(const QDateTime& dateAdded) {
    QMutexLocker lock(&m_qMutex);
    m_record.setDateAdded(dateAdded);
}

void Track::setDuration(mixxx::Duration duration) {
    QMutexLocker lock(&m_qMutex);
    // TODO: Move checks into TrackRecord
    VERIFY_OR_DEBUG_ASSERT(!m_record.getStreamInfoFromSource() ||
            m_record.getStreamInfoFromSource()->getDuration() <= mixxx::Duration::empty() ||
            m_record.getStreamInfoFromSource()->getDuration() == duration) {
        kLogger.warning()
                << "Cannot override stream duration:"
                << m_record.getStreamInfoFromSource()->getDuration()
                << "->"
                << duration;
        return;
    }
    if (compareAndSet(
                m_record.refMetadata().refStreamInfo().ptrDuration(),
                duration)) {
        markDirtyAndUnlock(&lock);
    }
}

void Track::setDuration(double duration) {
    setDuration(mixxx::Duration::fromSeconds(duration));
}

double Track::getDuration(DurationRounding rounding) const {
    QMutexLocker lock(&m_qMutex);
    const auto durationSeconds =
            m_record.getMetadata().getStreamInfo().getDuration().toDoubleSeconds();
    switch (rounding) {
    case DurationRounding::SECONDS:
        return std::round(durationSeconds);
    default:
        return durationSeconds;
    }
}

QString Track::getDurationText(mixxx::Duration::Precision precision) const {
    QMutexLocker lock(&m_qMutex);
    return m_record.getMetadata().getDurationText(precision);
}

QString Track::getTitle() const {
    QMutexLocker lock(&m_qMutex);
    return m_record.getMetadata().getTrackInfo().getTitle();
}

void Track::setTitle(const QString& s) {
    QMutexLocker lock(&m_qMutex);
    QString trimmed(s.trimmed());
    if (compareAndSet(m_record.refMetadata().refTrackInfo().ptrTitle(), trimmed)) {
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
    if (compareAndSet(m_record.refMetadata().refTrackInfo().ptrArtist(), trimmed)) {
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
    if (compareAndSet(m_record.refMetadata().refAlbumInfo().ptrTitle(), trimmed)) {
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
    if (compareAndSet(m_record.refMetadata().refAlbumInfo().ptrArtist(), trimmed)) {
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
    if (compareAndSet(m_record.refMetadata().refTrackInfo().ptrYear(), trimmed)) {
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
    if (compareAndSet(m_record.refMetadata().refTrackInfo().ptrGenre(), trimmed)) {
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
    if (compareAndSet(m_record.refMetadata().refTrackInfo().ptrComposer(), trimmed)) {
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
    if (compareAndSet(m_record.refMetadata().refTrackInfo().ptrGrouping(), trimmed)) {
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
    if (compareAndSet(m_record.refMetadata().refTrackInfo().ptrTrackNumber(), trimmed)) {
        markDirtyAndUnlock(&lock);
    }
}

void Track::setTrackTotal(const QString& s) {
    QMutexLocker lock(&m_qMutex);
    QString trimmed(s.trimmed());
    if (compareAndSet(m_record.refMetadata().refTrackInfo().ptrTrackTotal(), trimmed)) {
        markDirtyAndUnlock(&lock);
    }
}

PlayCounter Track::getPlayCounter() const {
    QMutexLocker lock(&m_qMutex);
    return m_record.getPlayCounter();
}

void Track::setPlayCounter(const PlayCounter& playCounter) {
    QMutexLocker lock(&m_qMutex);
    if (compareAndSet(m_record.ptrPlayCounter(), playCounter)) {
        markDirtyAndUnlock(&lock);
    }
}

void Track::updatePlayCounter(bool bPlayed) {
    QMutexLocker lock(&m_qMutex);
    PlayCounter playCounter(m_record.getPlayCounter());
    playCounter.updateLastPlayedNowAndTimesPlayed(bPlayed);
    if (compareAndSet(m_record.ptrPlayCounter(), playCounter)) {
        markDirtyAndUnlock(&lock);
    }
}

mixxx::RgbColor::optional_t Track::getColor() const {
    QMutexLocker lock(&m_qMutex);
    return m_record.getColor();
}

void Track::setColor(const mixxx::RgbColor::optional_t& color) {
    QMutexLocker lock(&m_qMutex);
    if (compareAndSet(m_record.ptrColor(), color)) {
        markDirtyAndUnlock(&lock);
        emit colorUpdated(color);
    }
}

QString Track::getComment() const {
    QMutexLocker lock(&m_qMutex);
    return m_record.getMetadata().getTrackInfo().getComment();
}

void Track::setComment(const QString& s) {
    QMutexLocker lock(&m_qMutex);
    if (compareAndSet(m_record.refMetadata().refTrackInfo().ptrComment(), s)) {
        markDirtyAndUnlock(&lock);
    }
}

QString Track::getType() const {
    QMutexLocker lock(&m_qMutex);
    return m_record.getFileType();
}

void Track::setType(const QString& sType) {
    QMutexLocker lock(&m_qMutex);
    if (compareAndSet(m_record.ptrFileType(), sType)) {
        markDirtyAndUnlock(&lock);
    }
}

mixxx::audio::SampleRate Track::getSampleRate() const {
    QMutexLocker lock(&m_qMutex);
    return m_record.getMetadata().getStreamInfo().getSignalInfo().getSampleRate();
}

int Track::getChannels() const {
    QMutexLocker lock(&m_qMutex);
    return m_record.getMetadata().getStreamInfo().getSignalInfo().getChannelCount();
}

int Track::getBitrate() const {
    QMutexLocker lock(&m_qMutex);
    return m_record.getMetadata().getStreamInfo().getBitrate();
}

QString Track::getBitrateText() const {
    QMutexLocker lock(&m_qMutex);
    return m_record.getMetadata().getBitrateText();
}

void Track::setBitrate(int iBitrate) {
    QMutexLocker lock(&m_qMutex);
    const mixxx::audio::Bitrate bitrate(iBitrate);
    // TODO: Move checks into TrackRecord
    VERIFY_OR_DEBUG_ASSERT(!m_record.getStreamInfoFromSource() ||
            !m_record.getStreamInfoFromSource()->getBitrate().isValid() ||
            m_record.getStreamInfoFromSource()->getBitrate() == bitrate) {
        kLogger.warning()
                << "Cannot override stream bitrate:"
                << m_record.getStreamInfoFromSource()->getBitrate()
                << "->"
                << bitrate;
        return;
    }
    if (compareAndSet(
                m_record.refMetadata().refStreamInfo().ptrBitrate(),
                bitrate)) {
        markDirtyAndUnlock(&lock);
    }
}

TrackId Track::getId() const {
    QMutexLocker lock(&m_qMutex);
    return m_record.getId();
}

void Track::initId(TrackId id) {
    QMutexLocker lock(&m_qMutex);
    DEBUG_ASSERT(id.isValid());
    if (m_record.getId() == id) {
        return;
    }
    // The track's id must be set only once and immediately after
    // the object has been created.
    VERIFY_OR_DEBUG_ASSERT(!m_record.getId().isValid()) {
        kLogger.warning() << "Cannot change id from"
                << m_record.getId() << "to" << id;
        return; // abort
    }
    m_record.setId(id);
    // Changing the Id does not make the track dirty because the Id is always
    // generated by the database itself.
}

void Track::resetId() {
    QMutexLocker lock(&m_qMutex);
    m_record.setId(TrackId());
}

void Track::setURL(const QString& url) {
    QMutexLocker lock(&m_qMutex);
    if (compareAndSet(m_record.ptrUrl(), url)) {
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
    emit waveformUpdated();
}

ConstWaveformPointer Track::getWaveformSummary() const {
    return m_waveformSummary;
}

void Track::setWaveformSummary(ConstWaveformPointer pWaveform) {
    m_waveformSummary = pWaveform;
    emit waveformSummaryUpdated();
}

void Track::setCuePoint(CuePosition cue) {
    QMutexLocker lock(&m_qMutex);

    if (!compareAndSet(m_record.ptrCuePoint(), cue)) {
        // Nothing changed.
        return;
    }

    // Store the cue point as main cue
    CuePointer pLoadCue = findCueByType(mixxx::CueType::MainCue);
    double position = cue.getPosition();
    if (position != -1.0) {
        if (pLoadCue) {
            pLoadCue->setStartPosition(position);
        } else {
            pLoadCue = CuePointer(new Cue(
                    mixxx::CueType::MainCue,
                    Cue::kNoHotCue,
                    position,
                    Cue::kNoPosition));
            // While this method could be called from any thread,
            // associated Cue objects should always live on the
            // same thread as their host, namely this->thread().
            pLoadCue->moveToThread(thread());
            connect(pLoadCue.get(),
                    &Cue::updated,
                    this,
                    &Track::slotCueUpdated);
            m_cuePoints.push_back(pLoadCue);
        }
    } else if (pLoadCue) {
        disconnect(pLoadCue.get(), nullptr, this, nullptr);
        m_cuePoints.removeOne(pLoadCue);
    }

    markDirtyAndUnlock(&lock);
    emit cuesUpdated();
}

void Track::shiftCuePositionsMillis(double milliseconds) {
    QMutexLocker lock(&m_qMutex);

    VERIFY_OR_DEBUG_ASSERT(m_record.getStreamInfoFromSource()) {
        return;
    }
    double frames = m_record.getStreamInfoFromSource()->getSignalInfo().millis2frames(milliseconds);
    for (const CuePointer& pCue : qAsConst(m_cuePoints)) {
        pCue->shiftPositionFrames(frames);
    }

    markDirtyAndUnlock(&lock);
}

void Track::analysisFinished() {
    emit analyzed();
}

CuePosition Track::getCuePoint() const {
    QMutexLocker lock(&m_qMutex);
    return m_record.getCuePoint();
}

void Track::slotCueUpdated() {
    markDirty();
    emit cuesUpdated();
}

CuePointer Track::createAndAddCue(
        mixxx::CueType type,
        int hotCueIndex,
        double sampleStartPosition,
        double sampleEndPosition) {
    CuePointer pCue(new Cue(
            type,
            hotCueIndex,
            sampleStartPosition,
            sampleEndPosition));
    // While this method could be called from any thread,
    // associated Cue objects should always live on the
    // same thread as their host, namely this->thread().
    pCue->moveToThread(thread());
    connect(pCue.get(),
            &Cue::updated,
            this,
            &Track::slotCueUpdated);
    QMutexLocker lock(&m_qMutex);
    m_cuePoints.push_back(pCue);
    markDirtyAndUnlock(&lock);
    emit cuesUpdated();
    return pCue;
}

CuePointer Track::findCueByType(mixxx::CueType type) const {
    // This method cannot be used for hotcues because there can be
    // multiple hotcues and this function returns only a single CuePointer.
    VERIFY_OR_DEBUG_ASSERT(type != mixxx::CueType::HotCue) {
        return CuePointer();
    }
    QMutexLocker lock(&m_qMutex);
    for (const CuePointer& pCue: m_cuePoints) {
        if (pCue->getType() == type) {
            return pCue;
        }
    }
    return CuePointer();
}

CuePointer Track::findCueById(DbId id) const {
    QMutexLocker lock(&m_qMutex);
    for (const CuePointer& pCue : m_cuePoints) {
        if (pCue->getId() == id) {
            return pCue;
        }
    }
    return CuePointer();
}

void Track::removeCue(const CuePointer& pCue) {
    if (!pCue) {
        return;
    }

    QMutexLocker lock(&m_qMutex);
    disconnect(pCue.get(), nullptr, this, nullptr);
    m_cuePoints.removeOne(pCue);
    if (pCue->getType() == mixxx::CueType::MainCue) {
        m_record.setCuePoint(CuePosition());
    }
    markDirtyAndUnlock(&lock);
    emit cuesUpdated();
}

void Track::removeCuesOfType(mixxx::CueType type) {
    QMutexLocker lock(&m_qMutex);
    bool dirty = false;
    QMutableListIterator<CuePointer> it(m_cuePoints);
    while (it.hasNext()) {
        CuePointer pCue = it.next();
        // FIXME: Why does this only work for the Hotcue Type?
        if (pCue->getType() == type) {
            disconnect(pCue.get(), nullptr, this, nullptr);
            it.remove();
            dirty = true;
        }
    }
    if (compareAndSet(m_record.ptrCuePoint(), CuePosition())) {
        dirty = true;
    }
    if (dirty) {
        markDirtyAndUnlock(&lock);
        emit cuesUpdated();
    }
}

void Track::setCuePoints(const QList<CuePointer>& cuePoints) {
    // While this method could be called from any thread,
    // associated Cue objects should always live on the
    // same thread as their host, namely this->thread().
    for (const auto& pCue : cuePoints) {
        pCue->moveToThread(thread());
    }
    QMutexLocker lock(&m_qMutex);
    setCuePointsMarkDirtyAndUnlock(
            &lock,
            cuePoints);
}

Track::ImportStatus Track::tryImportBeats(
        mixxx::BeatsImporterPointer pBeatsImporter,
        bool lockBpmAfterSet) {
    QMutexLocker lock(&m_qMutex);
    VERIFY_OR_DEBUG_ASSERT(pBeatsImporter) {
        return ImportStatus::Complete;
    }
    DEBUG_ASSERT(!m_pBeatsImporterPending);
    m_pBeatsImporterPending = pBeatsImporter;
    if (m_pBeatsImporterPending->isEmpty()) {
        m_pBeatsImporterPending.reset();
        return ImportStatus::Complete;
    } else if (m_record.hasStreamInfoFromSource()) {
        // Replace existing beats with imported beats immediately
        tryImportPendingBeatsMarkDirtyAndUnlock(&lock, lockBpmAfterSet);
        return ImportStatus::Complete;
    } else {
        kLogger.debug()
                << "Import of beats is pending until the actual sample rate becomes available";
        // Clear all existing beats, that are supposed
        // to be replaced with the imported beats soon.
        if (trySetBeatsMarkDirtyAndUnlock(&lock,
                    nullptr,
                    lockBpmAfterSet)) {
            return ImportStatus::Pending;
        } else {
            return ImportStatus::Complete;
        }
    }
}

Track::ImportStatus Track::getBeatsImportStatus() const {
    QMutexLocker lock(&m_qMutex);
    return (!m_pBeatsImporterPending || m_pBeatsImporterPending->isEmpty())
            ? ImportStatus::Complete
            : ImportStatus::Pending;
}

bool Track::importPendingBeatsWhileLocked() {
    if (!m_pBeatsImporterPending) {
        // Nothing to do here
        return false;
    }

    VERIFY_OR_DEBUG_ASSERT(!m_pBeatsImporterPending->isEmpty()) {
        m_pBeatsImporterPending.reset();
        return false;
    }
    // The sample rate can only be trusted after the audio
    // stream has been opened.
    DEBUG_ASSERT(m_record.getStreamInfoFromSource());
    // The sample rate is supposed to be consistent
    DEBUG_ASSERT(m_record.getStreamInfoFromSource()->getSignalInfo().getSampleRate() ==
            m_record.getMetadata().getStreamInfo().getSignalInfo().getSampleRate());
    const auto pBeats = mixxx::BeatMap::makeBeatMap(
            m_record.getStreamInfoFromSource()->getSignalInfo().getSampleRate(),
            QString(),
            m_pBeatsImporterPending->importBeatsAndApplyTimingOffset(
                    getLocation(), *m_record.getStreamInfoFromSource()));
    DEBUG_ASSERT(m_pBeatsImporterPending->isEmpty());
    m_pBeatsImporterPending.reset();
    return setBeatsWhileLocked(pBeats);
}

bool Track::tryImportPendingBeatsMarkDirtyAndUnlock(
        QMutexLocker* pLock,
        bool lockBpmAfterSet) {
    DEBUG_ASSERT(pLock);

    if (m_record.getBpmLocked()) {
        return false;
    }

    bool modified = false;
    // Both functions must be invoked even if one of them
    // returns false!
    if (importPendingBeatsWhileLocked()) {
        modified = true;
    }
    if (compareAndSet(m_record.ptrBpmLocked(), lockBpmAfterSet)) {
        modified = true;
    }
    if (!modified) {
        // Unmodified, nothing todo
        return true;
    }

    afterBeatsAndBpmUpdated(pLock);
    return true;
}

Track::ImportStatus Track::importCueInfos(
        mixxx::CueInfoImporterPointer pCueInfoImporter) {
    QMutexLocker lock(&m_qMutex);
    VERIFY_OR_DEBUG_ASSERT(pCueInfoImporter) {
        return ImportStatus::Complete;
    }
    DEBUG_ASSERT(!m_pCueInfoImporterPending);
    m_pCueInfoImporterPending = pCueInfoImporter;
    if (m_pCueInfoImporterPending->isEmpty()) {
        // Just return the current import status without clearing any
        // existing cue points.
        m_pCueInfoImporterPending.reset();
        return ImportStatus::Complete;
    } else if (m_record.hasStreamInfoFromSource()) {
        // Replace existing cue points with imported cue
        // points immediately
        importPendingCueInfosMarkDirtyAndUnlock(&lock);
        return ImportStatus::Complete;
    } else {
        kLogger.debug()
                << "Import of"
                << m_pCueInfoImporterPending->size()
                << "cue(s) is pending until the actual sample rate becomes available";
        // Clear all existing cue points, that are supposed
        // to be replaced with the imported cue points soon.
        setCuePointsMarkDirtyAndUnlock(
                &lock,
                QList<CuePointer>{});
        return ImportStatus::Pending;
    }
}

Track::ImportStatus Track::getCueImportStatus() const {
    QMutexLocker lock(&m_qMutex);
    return (!m_pCueInfoImporterPending || m_pCueInfoImporterPending->isEmpty())
            ? ImportStatus::Complete
            : ImportStatus::Pending;
}

bool Track::setCuePointsWhileLocked(const QList<CuePointer>& cuePoints) {
    if (m_cuePoints.isEmpty() && cuePoints.isEmpty()) {
        // Nothing to do
        return false;
    }
    // Prevent inconsistencies between cue infos that have been queued
    // and are waiting to be imported and new cue points. At least one
    // of these two collections must be empty.
    DEBUG_ASSERT(cuePoints.isEmpty() || !m_pCueInfoImporterPending ||
            m_pCueInfoImporterPending->isEmpty());
    // disconnect existing cue points
    for (const auto& pCue : qAsConst(m_cuePoints)) {
        disconnect(pCue.get(), nullptr, this, nullptr);
    }
    m_cuePoints = cuePoints;
    // connect new cue points
    for (const auto& pCue : qAsConst(m_cuePoints)) {
        DEBUG_ASSERT(pCue->thread() == thread());
        // Start listening to cue point updatess AFTER setting
        // the track id. Otherwise we would receive unwanted
        // signals about changed cue points that may cause all
        // sorts of issues, e.g. when adding new tracks during
        // the library scan!
        connect(pCue.get(),
                &Cue::updated,
                this,
                &Track::slotCueUpdated);
        if (pCue->getType() == mixxx::CueType::MainCue) {
            m_record.setCuePoint(CuePosition(pCue->getPosition()));
        }
    }
    return true;
}

void Track::setCuePointsMarkDirtyAndUnlock(
        QMutexLocker* pLock,
        const QList<CuePointer>& cuePoints) {
    DEBUG_ASSERT(pLock);

    if (!setCuePointsWhileLocked(cuePoints)) {
        pLock->unlock();
        return;
    }

    markDirtyAndUnlock(pLock);
    emit cuesUpdated();
}

bool Track::importPendingCueInfosWhileLocked() {
    if (!m_pCueInfoImporterPending) {
        // Nothing to do here
        return false;
    }

    VERIFY_OR_DEBUG_ASSERT(!m_pCueInfoImporterPending->isEmpty()) {
        m_pCueInfoImporterPending.reset();
        return false;
    }
    // The sample rate can only be trusted after the audio
    // stream has been opened.
    DEBUG_ASSERT(m_record.getStreamInfoFromSource());
    const auto sampleRate =
            m_record.getStreamInfoFromSource()->getSignalInfo().getSampleRate();
    // The sample rate is supposed to be consistent
    DEBUG_ASSERT(sampleRate ==
            m_record.getMetadata().getStreamInfo().getSignalInfo().getSampleRate());
    QList<CuePointer> cuePoints;
    cuePoints.reserve(m_pCueInfoImporterPending->size() + m_cuePoints.size());

    // Preserve all existing cues with types that are not available for
    // importing.
    for (const CuePointer& pCue : qAsConst(m_cuePoints)) {
        if (!m_pCueInfoImporterPending->hasCueOfType(pCue->getType())) {
            cuePoints.append(pCue);
        }
    }

    const auto cueInfos =
            m_pCueInfoImporterPending->importCueInfosAndApplyTimingOffset(
                    getLocation(), m_record.getStreamInfoFromSource()->getSignalInfo());
    for (const auto& cueInfo : cueInfos) {
        CuePointer pCue(new Cue(cueInfo, sampleRate, true));
        // While this method could be called from any thread,
        // associated Cue objects should always live on the
        // same thread as their host, namely this->thread().
        pCue->moveToThread(thread());
        cuePoints.append(pCue);
    }
    DEBUG_ASSERT(m_pCueInfoImporterPending->isEmpty());
    m_pCueInfoImporterPending.reset();
    return setCuePointsWhileLocked(cuePoints);
}

void Track::importPendingCueInfosMarkDirtyAndUnlock(
        QMutexLocker* pLock) {
    DEBUG_ASSERT(pLock);

    if (!importPendingCueInfosWhileLocked()) {
        pLock->unlock();
        return;
    }

    markDirtyAndUnlock(pLock);
    emit cuesUpdated();
}

void Track::markDirty() {
    QMutexLocker lock(&m_qMutex);
    setDirtyAndUnlock(&lock, true);
}

void Track::markClean() {
    QMutexLocker lock(&m_qMutex);
    setDirtyAndUnlock(&lock, false);
}

void Track::setDirtyAndUnlock(QMutexLocker* pLock, bool bDirty) {
    const bool dirtyChanged = m_bDirty != bDirty;
    m_bDirty = bDirty;

    const auto trackId = m_record.getId();

    // Unlock before emitting any signals!
    pLock->unlock();

    if (!trackId.isValid()) {
        return;
    }
    if (dirtyChanged) {
        if (bDirty) {
            emit dirty(trackId);
        } else {
            emit clean(trackId);
        }
    }
    if (bDirty) {
        // Emit a changed signal regardless if this attempted to set us dirty.
        emit changed(trackId);
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
    if (compareAndSet(m_record.ptrRating(), rating)) {
        markDirtyAndUnlock(&lock);
    }
}

void Track::afterKeysUpdated(QMutexLocker* pLock) {
    const auto newKey = m_record.getGlobalKey();
    markDirtyAndUnlock(pLock);
    emitKeysUpdated(newKey);
}

void Track::emitKeysUpdated(mixxx::track::io::key::ChromaticKey newKey) {
    // New key might be INVALID. We don't care.
    emit keyUpdated(KeyUtils::keyToNumericValue(newKey));
    emit keysUpdated();
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
    if (compareAndSet(m_record.ptrBpmLocked(), bpmLocked)) {
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
    if (compareAndSet(m_record.ptrCoverInfo(), coverInfo)) {
        markDirtyAndUnlock(&lock);
        emit coverArtUpdated();
    }
}

bool Track::refreshCoverImageDigest(
        const QImage& loadedImage) {
    QMutexLocker lock(&m_qMutex);
    auto coverInfo = CoverInfo(
            m_record.getCoverInfo(),
            m_fileInfo.location());
    if (!coverInfo.refreshImageDigest(
                loadedImage,
                m_pSecurityToken)) {
        return false;
    }
    if (!compareAndSet(
                m_record.ptrCoverInfo(),
                static_cast<const CoverInfoRelative&>(coverInfo))) {
        return false;
    }
    kLogger.info()
            << "Refreshed cover image digest"
            << m_fileInfo.location();
    markDirtyAndUnlock(&lock);
    emit coverArtUpdated();
    return true;
}

CoverInfoRelative Track::getCoverInfo() const {
    QMutexLocker lock(&m_qMutex);
    return m_record.getCoverInfo();
}

CoverInfo Track::getCoverInfoWithLocation() const {
    QMutexLocker lock(&m_qMutex);
    return CoverInfo(m_record.getCoverInfo(), m_fileInfo.location());
}

ExportTrackMetadataResult Track::exportMetadata(
        mixxx::MetadataSourcePointer pMetadataSource,
        UserSettingsPointer pConfig) {
    VERIFY_OR_DEBUG_ASSERT(pMetadataSource) {
        kLogger.warning()
                << "Cannot export track metadata:"
                << getLocation();
        return ExportTrackMetadataResult::Failed;
    }
    // Locking shouldn't be necessary here, because this function will
    // be called after all references to the object have been dropped.
    // But it doesn't hurt much, so let's play it safe ;)
    QMutexLocker lock(&m_qMutex);
    // TODO(XXX): m_record.getMetadataSynchronized() currently is a
    // boolean flag, but it should become a time stamp in the future.
    // We could take this time stamp and the file's last modification
    // time stamp into account and might decide to skip importing
    // the metadata again.
    if (!m_bMarkedForMetadataExport && !m_record.getMetadataSynchronized()) {
        // If the metadata has never been imported from file tags it
        // must be exported explicitly once. This ensures that we don't
        // overwrite existing file tags with completely different
        // information.
        kLogger.info()
                << "Skip exporting of unsynchronized track metadata:"
                << getLocation();
        // abort
        return ExportTrackMetadataResult::Skipped;
    }

    if (pConfig->getValue<bool>(kConfigKeySeratoMetadataExport)) {
        const auto streamInfo = m_record.getStreamInfoFromSource();
        VERIFY_OR_DEBUG_ASSERT(streamInfo &&
                streamInfo->getSignalInfo().isValid() &&
                streamInfo->getDuration() > mixxx::Duration::empty()) {
            kLogger.warning() << "Cannot write Serato metadata because signal "
                                 "info and/or duration is not available:"
                              << getLocation();
            return ExportTrackMetadataResult::Skipped;
        }

        const mixxx::audio::SampleRate sampleRate =
                streamInfo->getSignalInfo().getSampleRate();

        mixxx::SeratoTags* seratoTags = m_record.refMetadata().refTrackInfo().ptrSeratoTags();
        DEBUG_ASSERT(seratoTags);

        if (seratoTags->status() == mixxx::SeratoTags::ParserStatus::Failed) {
            kLogger.warning()
                    << "Refusing to overwrite Serato metadata that failed to parse:"
                    << getLocation();
        } else {
            seratoTags->setTrackColor(getColor());
            seratoTags->setBpmLocked(isBpmLocked());

            QList<mixxx::CueInfo> cueInfos;
            for (const CuePointer& pCue : qAsConst(m_cuePoints)) {
                cueInfos.append(pCue->getCueInfo(sampleRate));
            }

            const double timingOffset = mixxx::SeratoTags::guessTimingOffsetMillis(
                    getLocation(), streamInfo->getSignalInfo());
            seratoTags->setCueInfos(cueInfos, timingOffset);

            seratoTags->setBeats(m_pBeats,
                    streamInfo->getSignalInfo(),
                    streamInfo->getDuration(),
                    timingOffset);
        }
    }

    // Check if the metadata has actually been modified. Otherwise
    // we don't need to write it back. Exporting unmodified metadata
    // would needlessly update the file's time stamp and should be
    // avoided. Since we don't know in which state the file's metadata
    // is we import it again into a temporary variable.
    mixxx::TrackMetadata importedFromFile;
    // Normalize metadata before exporting to adjust the precision of
    // floating values, ... Otherwise the following comparisons may
    // repeatedly indicate that values have changed only due to
    // rounding errors.
    // The normalization has to be performed on a copy of the metadata.
    // Otherwise floating-point values like the bpm value might become
    // inconsistent with the actual value stored by the beat grid!
    mixxx::TrackMetadata normalizedFromRecord;
    if ((pMetadataSource->importTrackMetadataAndCoverImage(&importedFromFile, nullptr).first ==
            mixxx::MetadataSource::ImportResult::Succeeded)) {
        // Prevent overwriting any file tags that are not yet stored in the
        // library database! This will in turn update the current metadata
        // that is stored in the database. New columns that need to be populated
        // from file tags cannot be filled during a database migration.
        m_record.mergeImportedMetadata(importedFromFile);

        // Prepare export by cloning and normalizing the metadata
        normalizedFromRecord = m_record.getMetadata();
        normalizedFromRecord.normalizeBeforeExport();

        // Finally the track's current metadata and the imported/adjusted metadata
        // can be compared for differences to decide whether the tags in the file
        // would change if we perform the write operation. This function will also
        // copy all extra properties that are not (yet) stored in the library before
        // checking for differences! If an export has been requested explicitly then
        // we will continue even if no differences are detected.
        // NOTE(uklotzde, 2020-01-05): Detection of modified bpm values is restricted
        // to integer precision to avoid re-exporting of unmodified ID3 tags in case
        // of fractional bpm values. As a consequence small changes in bpm values
        // cannot be detected and file tags with fractional values might not be
        // updated as expected! In these edge cases users need to explicitly
        // trigger the re-export of file tags or they could modify other metadata
        // properties.
        if (!m_bMarkedForMetadataExport &&
                !normalizedFromRecord.anyFileTagsModified(
                        importedFromFile,
                        mixxx::Bpm::Comparison::Integer)) {
            // The file tags are in-sync with the track's metadata and don't need
            // to be updated.
            if (kLogger.debugEnabled()) {
                kLogger.debug()
                            << "Skip exporting of unmodified track metadata into file:"
                            << getLocation();
            }
            // abort
            return ExportTrackMetadataResult::Skipped;
        }
    } else {
        // The file doesn't contain any tags yet or it might be missing, unreadable,
        // or corrupt.
        if (m_bMarkedForMetadataExport) {
            kLogger.info()
                    << "Adding or overwriting tags after failure to import tags from file:"
                    << getLocation();
            // Prepare export by cloning and normalizing the metadata
            normalizedFromRecord = m_record.getMetadata();
            normalizedFromRecord.normalizeBeforeExport();
        } else {
            kLogger.warning()
                    << "Skip exporting of track metadata after failure to import tags from file:"
                    << getLocation();
            // abort
            return ExportTrackMetadataResult::Skipped;
        }
    }
    // The track's metadata will be exported instantly. The export should
    // only be tried once so we reset the marker flag.
    m_bMarkedForMetadataExport = false;
    kLogger.debug()
            << "Old metadata (imported)"
            << importedFromFile;
    kLogger.debug()
            << "New metadata (modified)"
            << normalizedFromRecord;
    const auto trackMetadataExported =
            pMetadataSource->exportTrackMetadata(normalizedFromRecord);
    switch (trackMetadataExported.first) {
    case mixxx::MetadataSource::ExportResult::Succeeded:
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
        return ExportTrackMetadataResult::Succeeded;
    case mixxx::MetadataSource::ExportResult::Unsupported:
        return ExportTrackMetadataResult::Skipped;
    case mixxx::MetadataSource::ExportResult::Failed:
        kLogger.warning()
                << "Failed to export track metadata:"
                << getLocation();
        return ExportTrackMetadataResult::Failed;
    }
    DEBUG_ASSERT(!"unhandled case in switch statement");
    return ExportTrackMetadataResult::Skipped;
}

void Track::setAudioProperties(
        mixxx::audio::ChannelCount channelCount,
        mixxx::audio::SampleRate sampleRate,
        mixxx::audio::Bitrate bitrate,
        mixxx::Duration duration) {
    setAudioProperties(mixxx::audio::StreamInfo{
            mixxx::audio::SignalInfo{
                    channelCount,
                    sampleRate,
            },
            bitrate,
            duration,
    });
}

void Track::setAudioProperties(
        const mixxx::audio::StreamInfo& streamInfo) {
    QMutexLocker lock(&m_qMutex);
    // These properties are stored separately in the database
    // and are also imported from file tags. They will be
    // overriden by the actual properties from the audio
    // source later.
    DEBUG_ASSERT(!m_record.hasStreamInfoFromSource());
    if (compareAndSet(
                m_record.refMetadata().ptrStreamInfo(),
                streamInfo)) {
        markDirtyAndUnlock(&lock);
    }
}

void Track::updateStreamInfoFromSource(
        mixxx::audio::StreamInfo&& streamInfo) {
    QMutexLocker lock(&m_qMutex);
    bool updated = m_record.updateStreamInfoFromSource(streamInfo);

    const bool importBeats = m_pBeatsImporterPending && !m_pBeatsImporterPending->isEmpty();
    const bool importCueInfos = m_pCueInfoImporterPending && !m_pCueInfoImporterPending->isEmpty();

    if (!importBeats && !importCueInfos) {
        // Nothing more to do
        if (updated) {
            markDirtyAndUnlock(&lock);
        }
        return;
    }

    auto beatsImported = false;
    if (importBeats) {
        kLogger.debug() << "Finishing deferred import of beats because stream "
                           "audio properties are available now";
        beatsImported = importPendingBeatsWhileLocked();
    }

    auto cuesImported = false;
    if (importCueInfos) {
        DEBUG_ASSERT(m_cuePoints.isEmpty());
        kLogger.debug()
                << "Finishing deferred import of"
                << m_pCueInfoImporterPending->size()
                << "cue(s) because stream audio properties are available now";
        cuesImported = importPendingCueInfosWhileLocked();
    }

    if (!beatsImported && !cuesImported) {
        return;
    }

    if (beatsImported) {
        afterBeatsAndBpmUpdated(&lock);
    } else {
        markDirtyAndUnlock(&lock);
    }
    if (cuesImported) {
        emit cuesUpdated();
    }
}
