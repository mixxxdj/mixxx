
#include "broadcast/mpris/mprisplayer.h"

#include "mixer/deck.h"
#include "mixer/playerinfo.h"
#include "mixer/playermanager.h"
#include "mixxxmainwindow.h"
#include "moc_mprisplayer.cpp"

namespace {

const QString kPlaybackStatusPlaying = "Playing";
const QString kPlaybackStatusPaused = "Paused";
const QString kPlaybackStatusStopped = "Stopped";

// the playback will stop when there are no more tracks to play
const QString kLoopStatusNone = "None";
// The current track will start again from the begining once it has finished playing
const QString kLoopStatusTrack = "Track";
// The playback loops through a list of tracks
//const QString kLoopStatusPlaylist = "Playlist";
} // namespace

MprisPlayer::MprisPlayer(PlayerManager* pPlayerManager,
        MixxxMainWindow* pWindow)
        : m_CPAutoDjEnabled("[AutoDJ]", "enabled"),
          m_pPlayerManager(pPlayerManager),
          m_pWindow(pWindow),
          m_bAutoDJEnabled(false) {
    connect(m_pWindow, &MixxxMainWindow::componentsInitialized, this, &MprisPlayer::mixxxComponentsInitialized);
}

QString MprisPlayer::playbackStatus() const {
    if (!m_bAutoDJEnabled)
        return kPlaybackStatusStopped;
    for (unsigned int i = 1; i <= m_pPlayerManager->numberOfDecks(); ++i) {
        if (!m_pPlayerManager->getDeck(i)->isTrackPaused()) {
            return kPlaybackStatusPlaying;
        }
    }
    return kPlaybackStatusPaused;
}

QString MprisPlayer::loopStatus() const {
    if (!m_bAutoDJEnabled)
        return kLoopStatusNone;
    return QString();
}

void MprisPlayer::setLoopStatus(const QString& value) {
    double repeat = 0.0;
    if (value == kLoopStatusTrack) {
        repeat = 1.0;
    }

    TrackPointer pTrack;
    int deckIndex = PlayerInfo::instance().getCurrentPlayingDeck();
    if (deckIndex >= 0) {
        QString group = PlayerManager::groupForDeck(deckIndex);
        ConfigKey key(group, "repeat");
        ControlObject::set(key, repeat);
        // TODO: Decide when how to handle playlist repeat mode
    }
}

QVariantMap MprisPlayer::metadata() const {
    TrackPointer pTrack = PlayerInfo::instance().getCurrentPlayingTrack();
    QVariantMap metadata;
    if (!pTrack)
        return metadata;
    metadata.insert("mpris:trackid", QString(QStringLiteral("/org/mixxx/") + pTrack->getId().toString()));
    double trackDurationSeconds = pTrack->getDuration();
    trackDurationSeconds *= 1e6;
    metadata.insert("mpris:length", static_cast<long long int>(trackDurationSeconds));
    QStringList artists;
    artists << pTrack->getArtist();
    metadata.insert("xesam:artist", artists);
    metadata.insert("xesam:title", pTrack->getTitle());
    return metadata;
}

double MprisPlayer::volume() const {
    return 0;
}

void MprisPlayer::setVolume(double value) {
    Q_UNUSED(value);
}

qlonglong MprisPlayer::position() const {
    return 0;
}

bool MprisPlayer::canGoNext() const {
    return false;
}

bool MprisPlayer::canGoPrevious() const {
    return false;
}

bool MprisPlayer::canPlay() const {
    return false;
}

bool MprisPlayer::canPause() const {
    return false;
}

bool MprisPlayer::canSeek() const {
    return false;
}

void MprisPlayer::nextTrack() {
}

void MprisPlayer::pause() {
}

void MprisPlayer::playPause() {
}

void MprisPlayer::play() {
}

void MprisPlayer::stop() {
}

void MprisPlayer::seek(qlonglong offset) {
    Q_UNUSED(offset);
}

void MprisPlayer::setPosition(const QDBusObjectPath& trackId, qlonglong position) {
    Q_UNUSED(trackId);
    Q_UNUSED(position);
}

void MprisPlayer::openUri(const QString& uri) {
    Q_UNUSED(uri);
}

void MprisPlayer::mixxxComponentsInitialized() {
    m_bAutoDJEnabled = true;
}
