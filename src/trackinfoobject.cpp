#include <QDirIterator>
#include <QFile>
#include <QMutexLocker>
#include <QtDebug>

#include "trackinfoobject.h"

#include "library/coverartutils.h"
#include "metadata/trackmetadatataglib.h"
#include "soundsourceproxy.h"
#include "track/beatfactory.h"
#include "track/keyfactory.h"
#include "track/keyutils.h"
#include "util/assert.h"
#include "util/compatibility.h"
#include "waveform/waveform.h"


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
inline bool modifyFieldValue(T* pField, const T& value) {
    if (*pField != value) {
        *pField = value;
        return true;
    } else {
        return false;
    }
}

// Parses artist/title from the file name. Assumes that the file name is
// written like: "artist - title.xxx" or "artist_-_title.xxx". Artist
// and title fields are only overwritten if empty!
void parseTrackMetadataFromFileName(Mixxx::TrackMetadata* pTrackMetadata, QString fileName) {
    fileName.replace("_", " ");
    QString titleWithFileType;
    if (fileName.count('-') == 1) {
        if (pTrackMetadata->getArtist().isEmpty()) {
            const QString artist(fileName.section('-', 0, 0).trimmed());
            if (!artist.isEmpty()) {
                pTrackMetadata->setArtist(artist);
            }
        }
        titleWithFileType = fileName.section('-', 1, 1).trimmed();
    } else {
        titleWithFileType = fileName.trimmed();
    }
    if (pTrackMetadata->getTitle().isEmpty()) {
        const QString title(titleWithFileType.section('.', 0, -2).trimmed());
        if (!title.isEmpty()) {
            pTrackMetadata->setTitle(title);
        }
    }
}

} // anonymous namespace

TrackInfoObject::TrackInfoObject(
        const QFileInfo& fileInfo,
        const SecurityTokenPointer& pSecurityToken,
        TrackId trackId)
        : m_fileInfo(fileInfo),
          m_pSecurityToken(openSecurityToken(m_fileInfo, pSecurityToken)),
          m_qMutex(QMutex::Recursive),
          m_id(std::move(trackId)),
          m_bDirty(false),
          m_fCuePoint(0.0f),
          m_dateAdded(QDateTime::currentDateTime()),
          m_iRating(0),
          m_metadataParsedFromFile(false),
          m_bBpmLocked(false),
          m_analyserProgress(-1) {
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

void TrackInfoObject::setTrackMetadata(
        const Mixxx::TrackMetadata& trackMetadata,
        QImage *pCoverArt,
        bool parsedFromFile) {
    {
        // enter locking scope
        QMutexLocker lock(&m_qMutex);

        bool modified = modifyFieldValue(&m_metadataParsedFromFile, parsedFromFile);
        bool modifiedReplayGain =
                (m_metadata.getReplayGain() != trackMetadata.getReplayGain());
        bool modifiedCoverArt = false;
        if (m_metadata != trackMetadata) {
            const double replayGainBackup = m_metadata.getReplayGain();
            m_metadata = trackMetadata;
            if (!trackMetadata.isReplayGainValid()) {
                // restore replay gain
                m_metadata.setReplayGain(replayGainBackup);
                modifiedReplayGain = false;
            }
            modified = true;
        }
        if (pCoverArt && !pCoverArt->isNull()) {
            m_coverArt.image = *pCoverArt;
            m_coverArt.info.hash = CoverArtUtils::calculateHash(
                m_coverArt.image);
            m_coverArt.info.coverLocation = QString();
            m_coverArt.info.type = CoverInfo::METADATA;
            m_coverArt.info.source = CoverInfo::GUESSED;
            modifiedCoverArt = true;
            modified = true;
        }
        if (modified) {
            markDirtyAndUnlock(&lock);
            if (modifiedReplayGain) {
                emit(ReplayGainUpdated(trackMetadata.getReplayGain()));
            }
            if (modifiedCoverArt) {
                emit(coverArtUpdated());
            }
        }
        // leave locking scope -> implicitly unlocked
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

void TrackInfoObject::getTrackMetadata(Mixxx::TrackMetadata* pTrackMetadata, bool* pParsedFromFile) const {
    QMutexLocker lock(&m_qMutex);
    *pTrackMetadata = m_metadata;
    *pParsedFromFile = m_metadataParsedFromFile;
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
        setTrackMetadataParsed(false);
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
        setTrackMetadataParsed(false);
        return;
    }

    if (proxy.getType().isEmpty()) {
        qWarning() << "Failed to parse track metadata from file"
                << getLocation()
                << ": Unsupported file type";
        setTrackMetadataParsed(false);
        return;
    }
    setType(proxy.getType());

    Mixxx::TrackMetadata trackMetadata;
    bool parsedFromFile = false;
    // Use the existing trackMetadata as default values. Otherwise
    // existing values in the library would be overwritten with
    // empty values if the corresponding file tags are missing.
    // Depending on the file type some kind of tags might even
    // not be supported at all and those would get lost!
    getTrackMetadata(&trackMetadata, &parsedFromFile);
    if (parsedFromFile && !reloadFromFile) {
        return; // do not reload from file
    }
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
        parseTrackMetadataFromFileName(&trackMetadata, m_fileInfo.fileName());
    }

    // Dump the trackMetadata extracted from the file back into the track.
    setTrackMetadata(trackMetadata, pCoverArt, parsedFromFile);
}

QString TrackInfoObject::getLocation() const {
    // Copying QFileInfo is thread-safe due to "implicit sharing"
    // (copy-on write). But operating on a single instance of QFileInfo
    // might not be thread-safe due to internal caching!
    QMutexLocker lock(&m_qMutex);
    return TrackRef::location(m_fileInfo);
}

QString TrackInfoObject::getCanonicalLocation() const {
    // Copying QFileInfo is thread-safe due to "implicit sharing"
    // (copy-on write). But operating on a single instance of QFileInfo
    // might not be thread-safe due to internal caching!
    QMutexLocker lock(&m_qMutex);
    return TrackRef::canonicalLocation(m_fileInfo);
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

TrackId TrackInfoObject::getId() const {
    QMutexLocker lock(&m_qMutex);
    return m_id;
}

void TrackInfoObject::setId(TrackId id) {
    QMutexLocker lock(&m_qMutex);
    // The Id must not be modified from a valid value to another valid value
    DEBUG_ASSERT(!id.isValid() || !m_id.isValid() || (id == m_id));
    m_id = std::move(id);
    // Changing the Id does not make the track dirty because the
    // Id is always generated by the Database itself.
}

TrackRef TrackInfoObject::createRef() const {
    QMutexLocker lock(&m_qMutex);
    return TrackRef(m_fileInfo, m_id);
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

    double normalizedBpm =
            Mixxx::TrackMetadata::normalizeBpm(bpm);
    DEBUG_ASSERT(normalizedBpm ==
            Mixxx::TrackMetadata::normalizeBpm(normalizedBpm));

    QMutexLocker lock(&m_qMutex);

    if (!m_pBeats) {
        // No beat grid available -> create and initialize
        BeatsPointer pBeats(BeatFactory::makeBeatGrid(this, normalizedBpm, 0));
        setBeatsAndUnlock(&lock, pBeats);
        return normalizedBpm;
    }

    // Continue with the regular case
    if (m_pBeats->getBpm() != normalizedBpm) {
        qDebug() << "Updating BPM:" << getLocation();
        m_pBeats->setBpm(normalizedBpm);
        m_metadata.setBpm(normalizedBpm);
        markDirtyAndUnlock(&lock);
        // Tell the GUI to update the bpm label...
        //qDebug() << "TrackInfoObject signaling BPM update to" << f;
        emit(bpmUpdated(normalizedBpm));
    }

    return normalizedBpm;
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
    m_metadata.setBpm(
            Mixxx::TrackMetadata::normalizeBpm(bpm));
    markDirtyAndUnlock(pLock);
    emit(bpmUpdated(bpm));
    emit(beatsUpdated());
}

BeatsPointer TrackInfoObject::getBeats() const {
    QMutexLocker lock(&m_qMutex);
    return m_pBeats;
}

void TrackInfoObject::slotBeatsUpdated() {
    QMutexLocker lock(&m_qMutex);
    double bpm = m_pBeats->getBpm();
    m_metadata.setBpm(bpm);
    markDirtyAndUnlock(&lock);
    emit(bpmUpdated(bpm));
    emit(beatsUpdated());
}

double TrackInfoObject::setReplayGain(double replayGain) {
    double normalizedReplayGain =
            Mixxx::TrackMetadata::normalizeReplayGain(replayGain);
    DEBUG_ASSERT(normalizedReplayGain ==
            Mixxx::TrackMetadata::normalizeReplayGain(normalizedReplayGain));
    QMutexLocker lock(&m_qMutex);
    if (m_metadata.getReplayGain() != normalizedReplayGain) {
        m_metadata.setReplayGain(normalizedReplayGain);
        markDirtyAndUnlock(&lock);
        emit(ReplayGainUpdated(replayGain));
    }
    return normalizedReplayGain;
}

double TrackInfoObject::getReplayGain() const {
    QMutexLocker lock(&m_qMutex);
    return m_metadata.getReplayGain();
}

void TrackInfoObject::setTrackMetadataParsed(bool parsedFromFile) {
    QMutexLocker lock(&m_qMutex);
    if (modifyFieldValue(&m_metadataParsedFromFile, parsedFromFile)) {
        markDirtyAndUnlock(&lock);
    }
}

bool TrackInfoObject::isTrackMetadataParsed()  const {
    QMutexLocker lock(&m_qMutex);
    return m_metadataParsedFromFile;
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
    return Mixxx::TrackMetadata::formatDuration(getDuration());
}

QString TrackInfoObject::getTitle() const {
    QMutexLocker lock(&m_qMutex);
    return m_metadata.getTitle();
}

void TrackInfoObject::setTitle(const QString& s) {
    QMutexLocker lock(&m_qMutex);
    if (m_metadata.getTitle() != s.trimmed()) {
        m_metadata.setTitle(s.trimmed());
        markDirtyAndUnlock(&lock);
    }
}

QString TrackInfoObject::getArtist() const {
    QMutexLocker lock(&m_qMutex);
    return m_metadata.getArtist();
}

void TrackInfoObject::setArtist(const QString& s) {
    QMutexLocker lock(&m_qMutex);
    if (m_metadata.getArtist() != s.trimmed()) {
        m_metadata.setArtist(s.trimmed());
        markDirtyAndUnlock(&lock);
    }
}

QString TrackInfoObject::getAlbum() const {
    QMutexLocker lock(&m_qMutex);
    return m_metadata.getAlbum();
}

void TrackInfoObject::setAlbum(const QString& s) {
    QMutexLocker lock(&m_qMutex);
    if (m_metadata.getAlbum() != s.trimmed()) {
        m_metadata.setAlbum(s.trimmed());
        markDirtyAndUnlock(&lock);
    }
}

QString TrackInfoObject::getAlbumArtist()  const {
    QMutexLocker lock(&m_qMutex);
    return m_metadata.getAlbumArtist();
}

void TrackInfoObject::setAlbumArtist(const QString& s) {
    QMutexLocker lock(&m_qMutex);
    if (m_metadata.getAlbumArtist() != s.trimmed()) {
        m_metadata.setAlbumArtist(s.trimmed());
        markDirtyAndUnlock(&lock);
    }
}

QString TrackInfoObject::getYear()  const {
    QMutexLocker lock(&m_qMutex);
    return m_metadata.getYear();
}

void TrackInfoObject::setYear(const QString& s) {
    QMutexLocker lock(&m_qMutex);
    if (m_metadata.getYear() != s.trimmed()) {
        m_metadata.setYear(s.trimmed());
        markDirtyAndUnlock(&lock);
    }
}

QString TrackInfoObject::getGenre() const {
    QMutexLocker lock(&m_qMutex);
    return m_metadata.getGenre();
}

void TrackInfoObject::setGenre(const QString& s) {
    QMutexLocker lock(&m_qMutex);
    if (m_metadata.getGenre() != s.trimmed()) {
        m_metadata.setGenre(s.trimmed());
        markDirtyAndUnlock(&lock);
    }
}

QString TrackInfoObject::getComposer() const {
    QMutexLocker lock(&m_qMutex);
    return m_metadata.getComposer();
}

void TrackInfoObject::setComposer(const QString& s) {
    QMutexLocker lock(&m_qMutex);
    if (m_metadata.getComposer() != s.trimmed()) {
        m_metadata.setComposer(s.trimmed());
        markDirtyAndUnlock(&lock);
    }
}

QString TrackInfoObject::getGrouping()  const {
    QMutexLocker lock(&m_qMutex);
    return m_metadata.getGrouping();
}

void TrackInfoObject::setGrouping(const QString& s) {
    QMutexLocker lock(&m_qMutex);
    if (m_metadata.getGrouping() != s.trimmed()) {
        m_metadata.setGrouping(s.trimmed());
        markDirtyAndUnlock(&lock);
    }
}

QString TrackInfoObject::getTrackNumber()  const {
    QMutexLocker lock(&m_qMutex);
    return m_metadata.getTrackNumber();
}

void TrackInfoObject::setTrackNumber(const QString& s) {
    QMutexLocker lock(&m_qMutex);
    if (m_metadata.getTrackNumber() != s.trimmed()) {
        m_metadata.setTrackNumber(s.trimmed());
        markDirtyAndUnlock(&lock);
    }
}

void TrackInfoObject::setPlayed(bool played) {
    QMutexLocker lock(&m_qMutex);
    PlayCounter playCounter(m_playCounter);
    playCounter.setPlayed(played);
    if (modifyFieldValue(&m_playCounter, playCounter)) {
        markDirtyAndUnlock(&lock);
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
    if (modifyFieldValue(&m_playCounter, playCounter)) {
        markDirtyAndUnlock(&lock);
    }
}

void TrackInfoObject::resetTimesPlayed() {
    QMutexLocker lock(&m_qMutex);
    PlayCounter playCounter(m_playCounter);
    playCounter.resetTimesPlayed();
    if (modifyFieldValue(&m_playCounter, playCounter)) {
        markDirtyAndUnlock(&lock);
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
    if (modifyFieldValue(&m_playCounter, playCounter)) {
        markDirtyAndUnlock(&lock);
    }
}

PlayCounter TrackInfoObject::getPlayCounter() const {
    QMutexLocker lock(&m_qMutex);
    return m_playCounter;
}

void TrackInfoObject::setPlayCounter(const PlayCounter& playCounter) {
    QMutexLocker lock(&m_qMutex);
    if (modifyFieldValue(&m_playCounter, playCounter)) {
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
    if (modifyFieldValue(&m_sType, s)) {
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

QString TrackInfoObject::getBitrateStr() const {
    return QString("%1").arg(getBitrate());
}

void TrackInfoObject::setBitrate(int iBitrate) {
    QMutexLocker lock(&m_qMutex);
    if (m_metadata.getBitrate() != iBitrate) {
        m_metadata.setBitrate(iBitrate);
        markDirtyAndUnlock(&lock);
    }
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
    if (m_metadata.getUrl() != url) {
        m_metadata.setUrl(url);
        markDirtyAndUnlock(&lock);
    }
}

QString TrackInfoObject::getURL() {
    QMutexLocker lock(&m_qMutex);
    return m_metadata.getUrl();
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
    if (modifyFieldValue(&m_fCuePoint, cue)) {
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
    //qDebug() << "TrackInfoObject::addCue()";
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

bool TrackInfoObject::markDirty(bool bDirty) {
    QMutexLocker lock(&m_qMutex);
    return markDirtyAndUnlock(&lock, bDirty);
}

void TrackInfoObject::resetDirty() {
    QMutexLocker lock(&m_qMutex);
    setDirtyAndUnlock(&lock, false);
}

bool TrackInfoObject::markDirtyAndUnlock(QMutexLocker* pLock, bool bDirty) {
    bool newValue = m_bDirty || bDirty;
    setDirtyAndUnlock(pLock, newValue);
    return newValue;
}

void TrackInfoObject::setDirtyAndUnlock(QMutexLocker* pLock, bool bDirty) {
    bool modified = modifyFieldValue(&m_bDirty, bDirty);
    pLock->unlock();

    // qDebug() << "Track" << m_id << getInfo() << (modified ? "changed" : "unchanged")
    //          << "set" << (bDirty ? "dirty" : "clean");
    if (modified) {
        if (bDirty) {
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
    return m_iRating;
}

void TrackInfoObject::setRating (int rating) {
    QMutexLocker lock(&m_qMutex);
    if (modifyFieldValue(&m_iRating, rating)) {
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

mixxx::track::io::key::ChromaticKey TrackInfoObject::getKey() const {
    QMutexLocker lock(&m_qMutex);
    if (m_keys.isValid()) {
        return m_keys.getGlobalKey();
    } else {
        return mixxx::track::io::key::INVALID;
    }
}

void TrackInfoObject::setKey(mixxx::track::io::key::ChromaticKey key,
                             mixxx::track::io::key::Source source) {
    if (key == mixxx::track::io::key::INVALID) {
        resetKeys();
        return;
    }

    Keys keys(KeyFactory::makeBasicKeys(key, source));
    QMutexLocker lock(&m_qMutex);
    if (m_keys.getGlobalKey() != key) {
        setKeysAndUnlock(&lock, keys);
    }
}

void TrackInfoObject::setKeyText(const QString& keyText,
                                 mixxx::track::io::key::Source source) {
    Keys keys(KeyFactory::makeBasicKeysFromText(keyText, source));
    const mixxx::track::io::key::ChromaticKey key(keys.getGlobalKey());
    if (key == mixxx::track::io::key::INVALID) {
        resetKeys();
        return;
    }

    QMutexLocker lock(&m_qMutex);
    if (m_keys.getGlobalKey() != key) {
        setKeysAndUnlock(&lock, keys);
    }
}

QString TrackInfoObject::getKeyText() const {
    QMutexLocker lock(&m_qMutex);
    return KeyUtils::getGlobalKeyText(m_keys);
}

void TrackInfoObject::setBpmLocked(bool bpmLocked) {
    QMutexLocker lock(&m_qMutex);
    if (modifyFieldValue(&m_bBpmLocked, bpmLocked)) {
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
    if (modifyFieldValue(&m_coverArt, coverArt)) {
        markDirtyAndUnlock(&lock);
        emit(coverArtUpdated());
    }
}

CoverArt TrackInfoObject::getCoverArt() const {
    QMutexLocker lock(&m_qMutex);
    return m_coverArt;
}
