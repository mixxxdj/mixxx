
#include "broadcast/mpris/mprisplayer.h"

#include <QCryptographicHash>
#include <QtGlobal>

#include "library/coverartcache.h"
#include "mixer/deck.h"
#include "mixer/playerinfo.h"
#include "mixer/playermanager.h"
#include "mixxxmainwindow.h"
#include "moc_mprisplayer.cpp"
#include "mprisplayer.h"

namespace {

const QString kPlaybackStatusPlaying = "Playing";
const QString kPlaybackStatusPaused = "Paused";
const QString kPlaybackStatusStopped = "Stopped";

// the playback will stop when there are no more tracks to play
const QString kLoopStatusNone = "None";
// The current track will start again from the beginning once it has finished playing
const QString kLoopStatusTrack = "Track";
// The playback loops through a list of tracks
const QString kLoopStatusPlaylist = "Playlist";
const QString playerInterfaceName = "org.mpris.MediaPlayer2.Player";
} // namespace

#define AUTODJENABLED m_bComponentsInitialized && m_pCPAutoDjEnabled->toBool()
#define AUTODJIDLE AUTODJENABLED && m_pCPAutoDJIdle->toBool()

MprisPlayer::MprisPlayer(PlayerManager* pPlayerManager,
        MixxxMainWindow* pWindow,
        Mpris* pMpris,
        UserSettingsPointer pSettings)
        : m_pCPAutoDjEnabled(nullptr),
          m_pCPFadeNow(nullptr),
          m_pCPAutoDJIdle(nullptr),
          m_pPlayerManager(pPlayerManager),
          m_pWindow(pWindow),
          m_bComponentsInitialized(false),
          m_bPropertiesEnabled(false),
          m_pMpris(pMpris),
          m_pSettings(pSettings) {
    connect(m_pWindow, &MixxxMainWindow::componentsInitialized, this, &MprisPlayer::mixxxComponentsInitialized);
    CoverArtCache* pCache = CoverArtCache::instance();
    if (pCache) {
        connect(pCache,
                &CoverArtCache::coverFound,
                this,
                &MprisPlayer::slotCoverArtFound);
    }
}

QString MprisPlayer::playbackStatus() const {
    if (!AUTODJENABLED)
        return kPlaybackStatusStopped;
    for (DeckAttributes* attrib : m_deckAttributes) {
        if (attrib->isPlaying())
            return kPlaybackStatusPlaying;
    }
    return kPlaybackStatusPaused;
}

QString MprisPlayer::loopStatus() const {
    if (!AUTODJENABLED)
        return kLoopStatusNone;
    for (DeckAttributes* attrib : m_deckAttributes) {
        if (attrib->isRepeat() && attrib->isPlaying())
            return kLoopStatusTrack;
    }
    return m_pSettings->getValue(ConfigKey("[Auto DJ]", "Requeue"), false) ? kLoopStatusPlaylist : kLoopStatusNone;
}

void MprisPlayer::setLoopStatus(const QString& value) {
    if (value == kLoopStatusNone || value == kLoopStatusTrack) {
        for (DeckAttributes* attribute : m_deckAttributes) {
            attribute->setRepeat(value == kLoopStatusTrack);
        }
        m_pSettings->setValue(ConfigKey("[Auto DJ]", "Requeue"), false);
    } else {
        for (DeckAttributes* attribute : m_deckAttributes) {
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
    return getAverageVolume();
}

void MprisPlayer::setVolume(double value) {
    for (DeckAttributes* attrib : m_deckAttributes) {
        ControlProxy volume(ConfigKey(attrib->group, "volume"));
        volume.set(value);
    }
}

qlonglong MprisPlayer::position() const {
    if (AUTODJIDLE) {
        for (unsigned int i = 0; i < m_pPlayerManager->numberOfDecks(); ++i) {
            ControlProxy playing(ConfigKey(PlayerManager::groupForDeck(i), "play"));
            if (playing.toBool()) {
                DeckAttributes* pDeck = m_deckAttributes.at(i);
                qlonglong playPosition =
                        static_cast<qlonglong>(pDeck->playPosition() *   //Fraction of duration
                                pDeck->getLoadedTrack()->getDuration() * //Duration in seconds
                                1e6);
                return playPosition;
            }
        }
    }
    return 0;
}

bool MprisPlayer::canGoNext() const {
    return AUTODJIDLE;
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
    return AUTODJIDLE;
}

void MprisPlayer::nextTrack() {
    if (AUTODJIDLE) {
        m_pCPFadeNow->set(true);
    }
}

void MprisPlayer::pause() {
    if (AUTODJIDLE) {
        DeckAttributes* playingDeck = findPlayingDeck();
        if (playingDeck != nullptr) {
            playingDeck->stop();
            m_pMpris->notifyPropertyChanged(playerInterfaceName, "Metadata", QVariantMap());
            m_pausedDeck = playingDeck->group;
        }
    } else {
        for (DeckAttributes* attribute : m_deckAttributes) {
            attribute->stop();
        }
    }
}

void MprisPlayer::playPause() {
    if (AUTODJIDLE) {
        DeckAttributes* playingDeck = findPlayingDeck();
        if (playingDeck != nullptr) {
            playingDeck->stop();
            m_pMpris->notifyPropertyChanged(playerInterfaceName, "Metadata", QVariantMap());
            m_pausedDeck = playingDeck->group;
        } else {
            ControlProxy playing(ConfigKey(m_pausedDeck, "play"));
            BaseTrackPlayer* player = m_pPlayerManager->getPlayer(m_pausedDeck);
            DEBUG_ASSERT(player);
            TrackPointer pTrack = player->getLoadedTrack();
            playing.set(true);
        }
    }
}

void MprisPlayer::play() {
    if (!m_bComponentsInitialized) {
        return;
    }
    if (!m_pCPAutoDjEnabled->toBool()) {
        m_pCPAutoDjEnabled->set(true);
        return;
    }
    DeckAttributes* playingDeck = findPlayingDeck();
    if (playingDeck == nullptr) {
        ControlProxy playing(ConfigKey(m_pausedDeck, "play"));
        BaseTrackPlayer* player = m_pPlayerManager->getPlayer(m_pausedDeck);
        DEBUG_ASSERT(player);
        TrackPointer pTrack = player->getLoadedTrack();
        playing.set(true);
    }
}

qlonglong MprisPlayer::seek(qlonglong offset, bool& success) {
    if (AUTODJIDLE) {
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

qlonglong MprisPlayer::setPosition(const QDBusObjectPath& trackId, qlonglong position, bool& success) {
    if (AUTODJIDLE) {
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
    Q_UNUSED(uri);
}

void MprisPlayer::mixxxComponentsInitialized() {
    m_bComponentsInitialized = true;

    m_pCPAutoDjEnabled = new ControlProxy(ConfigKey("[AutoDJ]", "enabled"), this);
    m_pCPFadeNow = new ControlProxy(ConfigKey("[AutoDJ]", "fade_now"), this);
    m_pCPAutoDJIdle = new ControlProxy(ConfigKey("[AutoDJ]", "idle"), this);

    for (unsigned int i = 1; i <= m_pPlayerManager->numberOfDecks(); ++i) {
        DeckAttributes* attributes = new DeckAttributes(
                i,
                m_pPlayerManager->getDeck(i));
        m_deckAttributes.append(attributes);
        connect(attributes, &DeckAttributes::playChanged, this, &MprisPlayer::slotPlayChanged);
        connect(attributes, &DeckAttributes::playPositionChanged, this, &MprisPlayer::slotPlayPositionChanged);
        ControlProxy* volume = new ControlProxy(ConfigKey(attributes->group, "volume"));
        m_CPDeckVolumes.append(volume);
        volume->connectValueChanged(this, &MprisPlayer::slotVolumeChanged);
    }

    m_pCPAutoDjEnabled->connectValueChanged(this, &MprisPlayer::slotChangeProperties);
    m_pCPAutoDJIdle->connectValueChanged(this, &MprisPlayer::slotChangeProperties);
}

void MprisPlayer::slotChangeProperties(double enabled) {
    if (enabled != m_bPropertiesEnabled) {
        broadcastPropertiesChange(enabled);
        m_bPropertiesEnabled = static_cast<bool>(enabled);
    }
}

void MprisPlayer::broadcastPropertiesChange(bool enabled) {
    for (const QString& property : autoDJDependentProperties) {
        m_pMpris->notifyPropertyChanged(playerInterfaceName,
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
}

void MprisPlayer::requestCoverartUrl(TrackPointer pTrack) {
    CoverInfo coverInfo = pTrack->getCoverInfoWithLocation();
    if (coverInfo.type == CoverInfoRelative::FILE) {
        QString path = coverInfo.trackLocation + coverInfo.coverLocation;
        qDebug() << "Cover art path: " << path;
        QString urlString = "file://" + path;
        QUrl fileUrl(urlString, QUrl::StrictMode);
        if (!fileUrl.isValid()) {
            qDebug() << "Invalid URL: " << fileUrl;
            return;
        }
        m_currentMetadata.coverartUrl = urlString;
        broadcastCurrentMetadata();
    } else if (coverInfo.type == CoverInfoRelative::METADATA) {
        CoverArtCache::requestCover(this, coverInfo);
    }
}

void MprisPlayer::slotPlayChanged(DeckAttributes* pDeck, bool playing) {
    if (!AUTODJENABLED)
        return;
    bool otherDeckPlaying = false;
    DeckAttributes* playingDeck = playing ? pDeck : nullptr;
    for (int i = 0; i < m_deckAttributes.size(); ++i) {
        if (m_deckAttributes.at(i)->group != pDeck->group &&
                m_deckAttributes.at(i)->isPlaying()) {
            otherDeckPlaying = true;
            playingDeck = m_deckAttributes.at(i);
            break;
        }
    }
    m_pMpris->notifyPropertyChanged(playerInterfaceName, "PlaybackStatus", playing || otherDeckPlaying ? kPlaybackStatusPlaying : kPlaybackStatusPaused);
    if (!playing && !otherDeckPlaying) {
        m_pMpris->notifyPropertyChanged(playerInterfaceName, "Metadata", QVariantMap());
    } else if (!playing || !otherDeckPlaying) {
        requestMetadataFromTrack(playingDeck->getLoadedTrack(), true);
    }
}

MprisPlayer::~MprisPlayer() {
    for (DeckAttributes* attrib : m_deckAttributes) {
        delete attrib;
    }
}

void MprisPlayer::slotPlayPositionChanged(DeckAttributes* pDeck, double position) {
    if (AUTODJIDLE) {
        qlonglong playPosition = static_cast<qlonglong>(position * //Fraction of duration
                pDeck->getLoadedTrack()->getDuration() *           //Duration in seconds
                1e6);
        m_pMpris->notifyPropertyChanged(playerInterfaceName, "Position", playPosition);
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

void MprisPlayer::slotVolumeChanged(double volume) {
    Q_UNUSED(volume);
    double averageVolume = getAverageVolume();
    m_pMpris->notifyPropertyChanged(playerInterfaceName, "Volume", averageVolume);
}

double MprisPlayer::getAverageVolume() const {
    double averageVolume = 0.0;
    unsigned int numberOfPlayingDecks = 0;
    for (DeckAttributes* attrib : m_deckAttributes) {
        if (attrib->isPlaying()) {
            ControlProxy volume(ConfigKey(attrib->group, "volume"));
            averageVolume += volume.get();
            ++numberOfPlayingDecks;
        }
    }
    averageVolume /= numberOfPlayingDecks;
    return averageVolume;
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

void MprisPlayer::slotCoverArtFound(const QObject* requestor,
        const CoverInfo& info,
        const QPixmap& pixmap) {
    Q_UNUSED(requestor);
    Q_UNUSED(info);

    if (!pixmap.isNull() && requestor == this) {
        QImage coverImage = pixmap.toImage();
        QByteArray hash = QCryptographicHash::hash(
                m_currentMetadata.title.toUtf8() +
                        m_currentMetadata.artists.at(0).toUtf8(),
                QCryptographicHash::Sha1);
        QString imagePath = QDir::tempPath() + "/" + hash + ".jpg";
        bool success = coverImage.save(imagePath, "JPG");
        if (!success) {
            qDebug() << "Couldn't write metadata cover art";
            return;
        }
        m_currentMetadata.coverartUrl = imagePath;
        broadcastCurrentMetadata();
    }
}

void MprisPlayer::broadcastCurrentMetadata() {
    m_pMpris->notifyPropertyChanged(playerInterfaceName, "Metadata", getVariantMapMetadata());
}

QVariantMap MprisPlayer::getVariantMapMetadata() {
    QVariantMap metadata;
    metadata.insert("mpris:trackid", m_currentMetadata.trackPath);
    metadata.insert("mpris:length", m_currentMetadata.trackDuration);
    metadata.insert("xesam:artist", m_currentMetadata.artists);
    metadata.insert("xesam:title", m_currentMetadata.title);
    metadata.insert("mpris:artUrl", m_currentMetadata.coverartUrl);
    return metadata;
}
