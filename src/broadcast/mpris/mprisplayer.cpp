#include "broadcast/mpris/mprisplayer.h"

#include <QCryptographicHash>
#include <QStringRef>
#include <QtGlobal>

#include "library/autodj/autodjprocessor.h"
#include "library/coverartcache.h"
#include "mixer/deck.h"
#include "mixer/playerinfo.h"
#include "mixer/playermanager.h"
#include "moc_mprisplayer.cpp"

namespace {

// Programmatic status flags (don't translate)
const QString kPlaybackStatusPlaying = QStringLiteral("Playing");
const QString kPlaybackStatusPaused = QStringLiteral("Paused");
const QString kPlaybackStatusStopped = QStringLiteral("Stopped");

// the playback will stop when there are no more tracks to play
const QString kLoopStatusNone = QStringLiteral("None");
// The current track will start again from the beginning once it has finished playing
const QString kLoopStatusTrack = QStringLiteral("Track");
// The playback loops through a list of tracks
const QString kLoopStatusPlaylist = QStringLiteral("Playlist");

const QString kPlayerInterfaceName = QStringLiteral("org.mpris.MediaPlayer2.Player");

} // namespace

MprisPlayer::MprisPlayer(PlayerManagerInterface* pPlayerManager,
        Mpris* pMpris,
        UserSettingsPointer pSettings)
        : m_pPlayerManager(pPlayerManager),
          m_bPropertiesEnabled(false),
          m_pMpris(pMpris),
          m_pPlayableDeck(nullptr),
          m_pSettings(pSettings) {
    CoverArtCache* pCache = CoverArtCache::instance();
    if (pCache) {
        connect(pCache,
                &CoverArtCache::coverFound,
                this,
                &MprisPlayer::slotCoverArtFound);
    }

    connect(&PlayerInfo::instance(),
            &PlayerInfo::currentPlayingTrackChanged,
            this,
            &MprisPlayer::slotPlayingTrackChanged);

    for (unsigned int i = 1; i <= m_pPlayerManager->numberOfDecks(); ++i) {
        DeckAttributes* attributes = new DeckAttributes(i,
                m_pPlayerManager->getDeck(i));
        m_deckAttributes.append(attributes);
        connect(attributes, &DeckAttributes::playChanged, this, &MprisPlayer::slotPlayChanged);
        connect(attributes,
                &DeckAttributes::playPositionChanged,
                this,
                &MprisPlayer::slotPlayPositionChanged);
    }

    m_pCPAutoDjEnabled = new ControlProxy(ConfigKey("[AutoDJ]", "enabled"), this);
    m_pCPAutoDjEnabled->connectValueChanged(this, &MprisPlayer::slotChangeProperties);

    m_pCPFadeNow = new ControlProxy(ConfigKey("[AutoDJ]", "fade_now"), this);

    m_pCPMasterGain = new ControlProxy(ConfigKey("[Master]", "gain"), this);
    m_pCPMasterGain->connectValueChanged(this, &MprisPlayer::slotMasterGainChanged);
}

MprisPlayer::~MprisPlayer() {
    for (DeckAttributes* attrib : qAsConst(m_deckAttributes)) {
        delete attrib;
    }
}

QString MprisPlayer::playbackStatus() const {
    int playingDeckIndex = PlayerInfo::instance().getCurrentPlayingDeck();
    if (playingDeckIndex >= 0 && playingDeckIndex < m_deckAttributes.size()) {
        // We have a playing deck
        qDebug() << kPlaybackStatusPlaying;
        return kPlaybackStatusPlaying;
    }
    if (autoDjEnabled()) {
        return kPlaybackStatusPaused;
    }
    return kPlaybackStatusStopped;
}

QString MprisPlayer::loopStatus() const {
    if (!autoDjEnabled())
        return kLoopStatusNone;
    for (DeckAttributes* attrib : qAsConst(m_deckAttributes)) {
        if (attrib->isRepeat() && attrib->isPlaying())
            return kLoopStatusTrack;
    }
    return m_pSettings->getValue(ConfigKey("[Auto DJ]", "Requeue"), false)
            ? kLoopStatusPlaylist
            : kLoopStatusNone;
}

void MprisPlayer::setLoopStatus(const QString& value) {
    if (value == kLoopStatusNone || value == kLoopStatusTrack) {
        for (DeckAttributes* attribute : qAsConst(m_deckAttributes)) {
            attribute->setRepeat(value == kLoopStatusTrack);
        }
        m_pSettings->setValue(ConfigKey("[Auto DJ]", "Requeue"), false);
    } else {
        for (DeckAttributes* attribute : qAsConst(m_deckAttributes)) {
            attribute->setRepeat(false);
        }
        m_pSettings->setValue(ConfigKey("[Auto DJ]", "Requeue"), true);
    }
}

QVariantMap MprisPlayer::metadata() {
    TrackPointer pTrack = PlayerInfo::instance().getCurrentPlayingTrack();
    requestMetadataFromTrack(pTrack, false);
    return getVariantMapMetadata();
}

double MprisPlayer::volume() const {
    return m_pCPMasterGain->get();
}

void MprisPlayer::setVolume(double value) {
    return m_pCPMasterGain->set(value);
}

qlonglong MprisPlayer::position() const {
    if (autoDjIdle()) {
        for (unsigned int i = 0; i < m_pPlayerManager->numberOfDecks(); ++i) {
            ControlProxy playing(ConfigKey(PlayerManager::groupForDeck(i), "play"));
            if (playing.toBool()) {
                DeckAttributes* pDeck = m_deckAttributes.at(i);
                qlonglong playPosition =
                        static_cast<qlonglong>(pDeck->playPosition() *   // Fraction of duration
                                pDeck->getLoadedTrack()->getDuration() * // Duration in seconds
                                1e6);
                return playPosition;
            }
        }
    }
    return 0;
}

bool MprisPlayer::canGoNext() const {
    return autoDjIdle();
}

bool MprisPlayer::canGoPrevious() const {
    return false;
}

bool MprisPlayer::canPlay() const {
    return true;
}

bool MprisPlayer::canPause() const {
    return true;
}

bool MprisPlayer::canSeek() const {
    return autoDjIdle();
}

void MprisPlayer::nextTrack() {
    if (autoDjIdle()) {
        m_pCPFadeNow->set(true);
    }
}

void MprisPlayer::pause() {
    int playingDeckIndex = PlayerInfo::instance().getCurrentPlayingDeck();
    for (DeckAttributes* attribute : qAsConst(m_deckAttributes)) {
        attribute->stop();
    }
    if (playingDeckIndex >= 0 && playingDeckIndex < m_deckAttributes.size()) {
        m_pMpris->notifyPropertyChanged(kPlayerInterfaceName, "Metadata", QVariantMap());
        m_pPlayableDeck = m_deckAttributes[playingDeckIndex];
    }
}

void MprisPlayer::stop() {
    pause();
    if (autoDjEnabled()) {
        // Also disable Auto-DJ.
        m_pCPAutoDjEnabled->set(false);
    }
}

void MprisPlayer::playPause() {
    int playingDeckIndex = PlayerInfo::instance().getCurrentPlayingDeck();
    if (playingDeckIndex >= 0 && playingDeckIndex < m_deckAttributes.size()) {
        // We have a playing deck
        pause();
    } else {
        play();
    }
}

void MprisPlayer::play() {
    int playingDeckIndex = PlayerInfo::instance().getCurrentPlayingDeck();
    if (playingDeckIndex >= 0 && playingDeckIndex < m_deckAttributes.size()) {
        // We have already a playing deck
        return;
    }
    if (!autoDjEnabled()) {
        // Let Auto DJ take care what to play.
        m_pCPAutoDjEnabled->set(true);
        return;
    }

    if (autoDjIdle()) {
        if (m_pPlayableDeck) {
            m_pPlayableDeck->play();
        }
    } else {
        VERIFY_OR_DEBUG_ASSERT(m_deckAttributes.size() >= 2) {
            return;
        }
        m_deckAttributes[0]->play();
        m_deckAttributes[1]->play();
    }
}

qlonglong MprisPlayer::seek(qlonglong offset, bool& success) {
    if (canSeek()) {
        DeckAttributes* playingDeck = findPlayingDeck();
        VERIFY_OR_DEBUG_ASSERT(playingDeck) {
            success = false;
            return 0;
        }
        double durationSeconds = playingDeck->getLoadedTrack()->getDuration();
        double newPosition = playingDeck->playPosition() + offset / (durationSeconds * 1e6);
        if (newPosition < 0.0) {
            success = true;
            newPosition = 0.0;
            playingDeck->setPlayPosition(newPosition);
            return 0;
        }
        if (newPosition > 1.0) {
            success = true;
            nextTrack();
            return 0;
        }
        playingDeck->setPlayPosition(newPosition);
        success = true;
        return static_cast<qlonglong>(durationSeconds * 1e6) + offset;
    }
    success = false;
    return 0;
}

qlonglong MprisPlayer::setPosition(
        const QDBusObjectPath& trackId,
        qlonglong position,
        bool& success) {
    if (autoDjIdle()) {
        DeckAttributes* playingDeck = findPlayingDeck();
        VERIFY_OR_DEBUG_ASSERT(playingDeck) {
            success = false;
            return 0;
        }
        QString path = trackId.path();
        int lastSlashIndex = path.lastIndexOf('/');
        VERIFY_OR_DEBUG_ASSERT(lastSlashIndex != -1) {
            success = false;
            return 0;
        }
        QString id = path.right(path.size() - lastSlashIndex - 1);
        if (id != playingDeck->getLoadedTrack()->getId().toString()) {
            success = false;
            return 0;
        }
        double newPosition = position / (playingDeck->getLoadedTrack()->getDuration() * 1e6);
        if (newPosition < 0.0 || newPosition > 1.0) {
            success = false;
            return 0;
        }
        playingDeck->setPlayPosition(newPosition);
        success = true;
        return position;
    }
    success = false;
    return 0;
}

void MprisPlayer::openUri(const QString& uri) {
    qDebug() << "openUri" << uri << "not yet implemented";
}

void MprisPlayer::slotChangeProperties(double enabled) {
    if (enabled != m_bPropertiesEnabled) {
        broadcastPropertiesChange(enabled >= 0);
        m_bPropertiesEnabled = static_cast<bool>(enabled);
    }
}

void MprisPlayer::broadcastPropertiesChange(bool enabled) {
    for (const QString& property : autoDJDependentProperties) {
        m_pMpris->notifyPropertyChanged(kPlayerInterfaceName,
                property,
                enabled);
    }
}

void MprisPlayer::requestMetadataFromTrack(TrackPointer pTrack, bool requestCover) {
    if (!pTrack) {
        return;
    }
    m_currentMetadata.trackPath = QStringLiteral("/org/mixxx/") + pTrack->getId().toString();
    double trackDurationSeconds = pTrack->getDuration();
    trackDurationSeconds *= 1e6;
    m_currentMetadata.trackDuration =
            static_cast<long long int>(trackDurationSeconds);
    QStringList artists;
    artists << pTrack->getArtist();
    m_currentMetadata.artists = artists;
    m_currentMetadata.title = pTrack->getTitle();
    if (requestCover) {
        requestCoverartUrl(pTrack);
    }
    m_currentMetadata.album = pTrack->getAlbum();
    m_currentMetadata.userRating = pTrack->getRating();
    m_currentMetadata.useCount = pTrack->getTimesPlayed();
}

void MprisPlayer::requestCoverartUrl(TrackPointer pTrack) {
    CoverInfo coverInfo = pTrack->getCoverInfoWithLocation();
    if (coverInfo.type == CoverInfoRelative::FILE) {
        QFileInfo fileInfo;
        if (!coverInfo.trackLocation.isEmpty()) {
            fileInfo = QFileInfo(coverInfo.trackLocation);
        }
        QFileInfo coverFile(fileInfo.dir(), coverInfo.coverLocation);
        QString path = coverFile.absoluteFilePath();
        qDebug() << "Cover art path: " << path;
        QUrl fileUrl = QUrl::fromLocalFile(path);
        if (!fileUrl.isValid()) {
            qDebug() << "Invalid URL: " << fileUrl;
            return;
        }
        m_currentMetadata.coverartUrl = fileUrl.toString();
    } else if (coverInfo.type == CoverInfoRelative::METADATA) {
        CoverArtCache::requestCover(this, coverInfo);
    } else {
        m_currentMetadata.coverartUrl.clear();
    }
}

void MprisPlayer::slotPlayChanged(DeckAttributes* pDeck, bool playing) {
    Q_UNUSED(pDeck);
    Q_UNUSED(playing);
    m_pMpris->notifyPropertyChanged(kPlayerInterfaceName,
            "PlaybackStatus",
            playbackStatus());
}

void MprisPlayer::slotPlayPositionChanged(DeckAttributes* pDeck, double position) {
    if (autoDjIdle()) {
        qlonglong playPosition = static_cast<qlonglong>(position * // Fraction of duration
                pDeck->getLoadedTrack()->getDuration() *           // Duration in seconds
                1e6);
        m_pMpris->notifyPropertyChanged(kPlayerInterfaceName, "Position", playPosition);
    }
}

DeckAttributes* MprisPlayer::findPlayingDeck() const {
    for (int i = 0; i < m_deckAttributes.count(); ++i) {
        if (m_deckAttributes.at(i)->isPlaying()) {
            return m_deckAttributes.at(i);
        }
    }
    return nullptr;
}

void MprisPlayer::slotMasterGainChanged(double value) {
    m_pMpris->notifyPropertyChanged(kPlayerInterfaceName, "Volume", value);
}

double MprisPlayer::rate() const {
    DeckAttributes* playingDeck = findPlayingDeck();
    if (playingDeck != nullptr) {
        ControlProxy rate(ConfigKey(
                PlayerManager::groupForDeck(playingDeck->index - 1), "rate"));
        return rate.get();
    }
    return 0;
}

void MprisPlayer::setRate(double value) {
    DeckAttributes* playingDeck = findPlayingDeck();
    if (playingDeck != nullptr) {
        ControlProxy rate(ConfigKey(
                PlayerManager::groupForDeck(playingDeck->index - 1), "rate"));
        double clampedValue = qBound(-1.0, value, 1.0);
        rate.set(clampedValue);
    }
}

void MprisPlayer::slotCoverArtFound(const QObject* requester,
        const CoverInfoRelative& info,
        const QPixmap& pixmap) {
    Q_UNUSED(info);

    if (!pixmap.isNull() && requester == this) {
        QImage coverImage = pixmap.toImage();
        m_currentCoverArtFile.open();
        bool success = coverImage.save(&m_currentCoverArtFile, "JPG");
        if (!success) {
            qDebug() << "Couldn't write metadata cover art";
            return;
        }
        m_currentCoverArtFile.close();
        QUrl fileUrl = QUrl::fromLocalFile(m_currentCoverArtFile.fileName());
        m_currentMetadata.coverartUrl = fileUrl.toString();
        broadcastCurrentMetadata();
    }
}

void MprisPlayer::slotPlayingTrackChanged(TrackPointer pTrack) {
    requestMetadataFromTrack(pTrack, true);
    broadcastCurrentMetadata();
}

void MprisPlayer::broadcastCurrentMetadata() {
    m_pMpris->notifyPropertyChanged(kPlayerInterfaceName, "Metadata", getVariantMapMetadata());
}

QVariantMap MprisPlayer::getVariantMapMetadata() {
    QVariantMap metadata;
    metadata.insert("mpris:trackid", m_currentMetadata.trackPath);
    metadata.insert("mpris:length", m_currentMetadata.trackDuration);
    metadata.insert("xesam:artist", m_currentMetadata.artists);
    metadata.insert("xesam:title", m_currentMetadata.title);
    metadata.insert("mpris:artUrl", m_currentMetadata.coverartUrl);
    metadata.insert("xesam:album", m_currentMetadata.album);
    metadata.insert("xesam:userRating", m_currentMetadata.userRating);
    metadata.insert("xesam:useCount", m_currentMetadata.useCount);
    return metadata;
}

bool MprisPlayer::autoDjEnabled() const {
    return m_pCPAutoDjEnabled->toBool();
}

bool MprisPlayer::autoDjIdle() const {
    return autoDjEnabled();
}
