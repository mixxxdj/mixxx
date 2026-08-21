#include "broadcast/mpris/mprisplayer.h"

#include <qfileinfo.h>

#include <QCryptographicHash>
#include <QStringRef>
#include <QThread>
#include <QtConcurrentRun>
#include <QtGlobal>
#include <memory>
#include <optional>

#include "library/autodj/autodjprocessor.h"
#include "library/coverartcache.h"
#include "mixer/deck.h"
#include "mixer/playerinfo.h"
#include "mixer/playermanager.h"
#include "moc_mprisplayer.cpp"
#include "util/assert.h"

namespace {

// Programmatic status flags (don't translate)
const QString kPlaybackStatusPlaying = QStringLiteral("Playing");
const QString kPlaybackStatusPaused = QStringLiteral("Paused");
const QString kPlaybackStatusStopped = QStringLiteral("Stopped");
const QString kTemporaryCoverArtNameTemplate = QStringLiteral("mixxx-dbus-mpris-XXXXXXXX.png");

// the playback will stop when there are no more tracks to play
const QString kLoopStatusNone = QStringLiteral("None");
// The current track will start again from the beginning once it has finished playing
const QString kLoopStatusTrack = QStringLiteral("Track");
// The playback loops through a list of tracks
const QString kLoopStatusPlaylist = QStringLiteral("Playlist");

const QString kPlayerInterfaceName = QStringLiteral("org.mpris.MediaPlayer2.Player");
const ConfigKey kAutoDJRequeueKey = ConfigKey("[Auto DJ]", "Requeue");

std::optional<std::size_t> getCurrentPlayingDeck() {
    auto deck = PlayerInfo::instance().getCurrentPlayingDeck();
    if (deck == -1) {
        return std::nullopt;
    }
    return static_cast<std::size_t>(deck);
}

} // namespace

MprisPlayer::MprisPlayer(PlayerManagerInterface* pPlayerManager,
        Mpris* pMpris,
        UserSettingsPointer pSettings)
        : m_pCPAutoDjEnabled(ConfigKey("[AutoDJ]", "enabled"), this),
          m_pCPFadeNow(ConfigKey("[AutoDJ]", "fade_now"), this),
          m_pCPMasterGain(ConfigKey("[Master]", "gain"), this),
          m_pPlayerManager(pPlayerManager),
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

    for (int i = 0; i < m_pPlayerManager->numberOfDecks(); ++i) {
        auto attributes = std::make_unique<DeckAttributes>(i,
                m_pPlayerManager->getDeckBase(i));
        connect(attributes.get(),
                &DeckAttributes::playChanged,
                this,
                &MprisPlayer::slotPlayChanged);
        connect(attributes.get(),
                &DeckAttributes::playPositionChanged,
                this,
                &MprisPlayer::slotPlayPositionChanged);
        m_deckAttributes.emplace_back(std::move(attributes));
    }

    m_pCPAutoDjEnabled.connectValueChanged(this, &MprisPlayer::slotChangeProperties);
    m_pCPMasterGain.connectValueChanged(this, &MprisPlayer::slotMasterGainChanged);

    requestMetadataFromTrack(PlayerInfo::instance().getCurrentPlayingTrack(), true).then([this]() {
        broadcastCurrentMetadata();
    });
}

QString MprisPlayer::playbackStatus() const {
    auto playingDeckIndex = getCurrentPlayingDeck();
    if (playingDeckIndex.has_value() && playingDeckIndex < m_deckAttributes.size()) {
        return kPlaybackStatusPlaying;
    }
    if (autoDjEnabled()) {
        return kPlaybackStatusPaused;
    }
    return kPlaybackStatusStopped;
}

QString MprisPlayer::loopStatus() const {
    if (!autoDjEnabled()) {
        return kLoopStatusNone;
    }
    for (const auto& pAttrib : std::as_const(m_deckAttributes)) {
        if (pAttrib->isRepeat() && pAttrib->isPlaying()) {
            return kLoopStatusTrack;
        }
    }
    return m_pSettings->getValue(kAutoDJRequeueKey, false)
            ? kLoopStatusPlaylist
            : kLoopStatusNone;
}

void MprisPlayer::setLoopStatus(const QString& value) {
    if (value == kLoopStatusNone || value == kLoopStatusTrack) {
        for (const auto& pAttribute : std::as_const(m_deckAttributes)) {
            pAttribute->setRepeat(value == kLoopStatusTrack);
        }
        m_pSettings->setValue(kAutoDJRequeueKey, false);
    } else {
        for (const auto& pAttribute : std::as_const(m_deckAttributes)) {
            pAttribute->setRepeat(false);
        }
        m_pSettings->setValue(kAutoDJRequeueKey, true);
    }
}

QVariantMap MprisPlayer::metadata() {
    return getVariantMapMetadata();
}

double MprisPlayer::volume() const {
    return m_pCPMasterGain.get();
}

void MprisPlayer::setVolume(double value) {
    return m_pCPMasterGain.set(value);
}

qlonglong MprisPlayer::position() const {
    DeckAttributes* pDeck = findPlayingDeck();
    if (!pDeck) {
        return 0;
    }
    return static_cast<qlonglong>(pDeck->playPosition() * // Fraction of duration
            pDeck->getLoadedTrack()->getDuration() *      // Duration in seconds
            1e6);
}

bool MprisPlayer::canGoNext() const {
    return autoDjEnabled();
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
    return getCurrentPlayingDeck().has_value();
}

void MprisPlayer::nextTrack() {
    if (autoDjEnabled()) {
        m_pCPFadeNow.set(true);
    }
}

void MprisPlayer::pause() {
    auto playingDeckIndex = getCurrentPlayingDeck();
    for (const auto& pAttribute : std::as_const(m_deckAttributes)) {
        pAttribute->stop();
    }
    if (playingDeckIndex.has_value() && playingDeckIndex < m_deckAttributes.size()) {
        m_currentMetadata.idle(autoDjEnabled());
        broadcastCurrentMetadata();
        m_pPlayableDeck = m_deckAttributes[playingDeckIndex.value()].get();
    }
}

void MprisPlayer::stop() {
    pause();
    if (autoDjEnabled()) {
        // Also disable Auto-DJ.
        m_pCPAutoDjEnabled.set(false);
    }
}

void MprisPlayer::playPause() {
    auto playingDeckIndex = getCurrentPlayingDeck();
    if (playingDeckIndex.has_value() && playingDeckIndex < m_deckAttributes.size()) {
        // We have a playing deck
        pause();
    } else {
        play();
    }
}

void MprisPlayer::play() {
    auto playingDeckIndex = getCurrentPlayingDeck();
    if (playingDeckIndex.has_value() && playingDeckIndex < m_deckAttributes.size()) {
        // We have already a playing deck
        return;
    }
    if (m_pPlayableDeck) {
        m_pPlayableDeck->play();
        return;
    }
}

qlonglong MprisPlayer::seek(qlonglong offset, bool& success) {
    success = false;
    if (!canSeek()) {
        return 0;
    }
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

qlonglong MprisPlayer::setPosition(
        const QDBusObjectPath& trackId,
        qlonglong position,
        bool& success) {
    success = false;
    DeckAttributes* pDeck = findPlayingDeck();
    if (!pDeck) {
        return 0;
    }
    QString path = trackId.path();
    int lastSlashIndex = path.lastIndexOf('/');
    VERIFY_OR_DEBUG_ASSERT(lastSlashIndex != -1) {
        return 0;
    }
    QString id = path.right(path.size() - lastSlashIndex - 1);
    if (id != pDeck->getLoadedTrack()->getId().toString()) {
        return 0;
    }
    double newPosition = position / (pDeck->getLoadedTrack()->getDuration() * 1e6);
    if (newPosition < 0.0 || newPosition > 1.0) {
        return 0;
    }
    pDeck->setPlayPosition(newPosition);
    success = true;
    return position;
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

QFuture<void> MprisPlayer::requestMetadataFromTrack(TrackPointer pTrack, bool requestCover) {
    m_currentMetadata.idle(autoDjEnabled());
    if (!pTrack) {
        return {};
    }
    m_currentMetadata.album = pTrack->getAlbum();
    m_currentMetadata.artists = {pTrack->getArtist()};
    m_currentMetadata.title = pTrack->getTitle();
    m_currentMetadata.url = QUrl::fromLocalFile(pTrack->getLocation());
    m_currentMetadata.trackDuration = pTrack->getDuration() * 1e6;
    m_currentMetadata.trackPath = QStringLiteral("/org/mixxx/") + pTrack->getId().toString();
    m_currentMetadata.useCount = pTrack->getTimesPlayed();
    m_currentMetadata.userRating = pTrack->getRating();
    if (requestCover) {
        return requestCoverArtUrl(pTrack);
    }
    return {};
}

QFuture<void> MprisPlayer::requestCoverArtUrl(TrackPointer pTrack) {
    CoverInfo coverInfo = pTrack->getCoverInfoWithLocation();
    if (coverInfo.type == CoverInfoRelative::FILE) {
        return QtConcurrent::run(QThreadPool::globalInstance(), [pTrack, coverInfo, this]() {
            QFileInfo fileInfo;
            if (!coverInfo.trackLocation.isEmpty()) {
                fileInfo = QFileInfo(coverInfo.trackLocation);
            }
            QFileInfo coverFile(fileInfo.dir(), coverInfo.coverLocation);
            if (!coverFile.exists()) {
                return;
            }
            QImage cover(coverFile.absoluteFilePath());
            if (cover.isNull()) {
                return;
            }
            m_currentMetadata.newCoverArt();
            m_currentMetadata.pCoverArtFile->open();
            bool success = cover.save(m_currentMetadata.pCoverArtFile.get(), "PNG");
            if (!success) {
                qWarning() << "Couldn't write metadata cover art";
            }
            m_currentMetadata.pCoverArtFile->close();
        });
    } else if (coverInfo.type == CoverInfoRelative::METADATA) {
        CoverArtCache::requestCover(this, coverInfo);
    }
    return {};
}

void MprisPlayer::slotPlayChanged(DeckAttributes*, bool) {
    m_pMpris->notifyPropertyChanged(kPlayerInterfaceName,
            "PlaybackStatus",
            playbackStatus());
}

void MprisPlayer::slotPlayPositionChanged(DeckAttributes* pDeck, double position) {
    auto pTrack = pDeck->getLoadedTrack();
    if (!pTrack) {
        return;
    }
    auto playPosition = position *  // Fraction of duration
            pTrack->getDuration() * // Duration in seconds
            1e6;
    m_pMpris->notifyPropertyChanged(kPlayerInterfaceName,
            "Position",
            static_cast<qlonglong>(playPosition));
}

DeckAttributes* MprisPlayer::findPlayingDeck() const {
    auto playingDeckIndex = getCurrentPlayingDeck();
    if (!playingDeckIndex.has_value() || playingDeckIndex >= m_deckAttributes.size()) {
        return nullptr;
    }
    return m_deckAttributes.at(playingDeckIndex.value()).get();
}

void MprisPlayer::slotMasterGainChanged(double value) {
    m_pMpris->notifyPropertyChanged(kPlayerInterfaceName, "Volume", value);
}

double MprisPlayer::rate() const {
    DeckAttributes* playingDeck = findPlayingDeck();
    if (playingDeck == nullptr) {
        return 0;
    }
    ControlProxy rate(ConfigKey(
            PlayerManager::groupForDeck(playingDeck->index - 1), "rate"));
    return rate.get();
}

void MprisPlayer::setRate(double value) {
    DeckAttributes* playingDeck = findPlayingDeck();
    if (playingDeck == nullptr) {
        return;
    }
    ControlProxy rate(ConfigKey(
            PlayerManager::groupForDeck(playingDeck->index - 1), "rate"));
    rate.set(qBound(-1.0, value, 1.0));
}

void MprisPlayer::slotCoverArtFound(const QObject* requester,
        const CoverInfoRelative&,
        const QPixmap& pixmap) {
    if (pixmap.isNull() || requester != this) {
        return;
    }
    QImage coverImage = pixmap.toImage();
    m_currentMetadata.newCoverArt();
    m_currentMetadata.pCoverArtFile->open();
    bool success = coverImage.save(m_currentMetadata.pCoverArtFile.get(), "PNG");
    if (!success) {
        qWarning() << "Couldn't write metadata cover art";
    }
    m_currentMetadata.pCoverArtFile->close();
    broadcastCurrentMetadata();
}

void MprisPlayer::slotPlayingTrackChanged(TrackPointer pTrack) {
    requestMetadataFromTrack(pTrack, true).then([this]() {
        broadcastCurrentMetadata();
    });
}

void MprisPlayer::broadcastCurrentMetadata() {
    m_pMpris->notifyPropertyChanged(kPlayerInterfaceName, "Metadata", getVariantMapMetadata());
}

QVariantMap MprisPlayer::getVariantMapMetadata() {
    return {
            {"mpris:artUrl", m_currentMetadata.coverArtUrl()},
            {"mpris:length", static_cast<long long int>(m_currentMetadata.trackDuration)},
            {"mpris:trackid", m_currentMetadata.trackPath},
            {"xesam:album", m_currentMetadata.album},
            {"xesam:artist", m_currentMetadata.artists},
            {"xesam:title", m_currentMetadata.title},
            {"xesam:url", m_currentMetadata.url.toString()},
            {"xesam:useCount", m_currentMetadata.useCount},
            {"xesam:userRating", m_currentMetadata.userRating},
    };
}

bool MprisPlayer::autoDjEnabled() const {
    return m_pCPAutoDjEnabled.toBool();
}

MprisPlayer::CurrentMetadata::CurrentMetadata()
        : defaultCoverArtFile(QDir::temp().filePath(kTemporaryCoverArtNameTemplate)) {
    VERIFY_OR_DEBUG_ASSERT(defaultCoverArtFile.open()) {
        return;
    }
    QImage logo(":/images/icons/128x128/apps/mixxx.png");
    VERIFY_OR_DEBUG_ASSERT(!logo.isNull()) {
        qWarning() << "Couldn't load Mixxx logo";
        defaultCoverArtFile.close();
        return;
    }
    bool success = logo.save(&defaultCoverArtFile, "PNG");
    VERIFY_OR_DEBUG_ASSERT(success) {
        qWarning() << "Couldn't write Mixxx logo in metadata cover art";
    }
    defaultCoverArtFile.close();
}
void MprisPlayer::CurrentMetadata::newCoverArt() {
    pCoverArtFile = std::make_unique<QTemporaryFile>(
            QDir::temp().filePath(kTemporaryCoverArtNameTemplate));
}

QString MprisPlayer::CurrentMetadata::coverArtUrl() const {
    if (pCoverArtFile && QFileInfo::exists(pCoverArtFile->fileName()) &&
            !QImage(pCoverArtFile->fileName()).isNull()) {
        return QUrl::fromLocalFile(pCoverArtFile->fileName()).toString();
    }
    return QUrl::fromLocalFile(defaultCoverArtFile.fileName()).toString();
}

void MprisPlayer::CurrentMetadata::idle(bool autoDjEnabled) {
    pCoverArtFile.reset();
    trackPath.clear();
    trackDuration = 0;
    artists = {autoDjEnabled ? tr("AutoDJ is ready!") : tr("No track playing")};
    title = "Mixxx";
    album.clear();
    url.clear();
    userRating = 0;
    useCount = 0;
}
