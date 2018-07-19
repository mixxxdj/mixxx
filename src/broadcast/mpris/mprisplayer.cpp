
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
const QString playerInterfaceName = "org.mpris.MediaPlayer2.Player";
} // namespace

#define AUTODJENABLED m_bComponentsInitialized && m_pCPAutoDjEnabled->toBool()

MprisPlayer::MprisPlayer(PlayerManager* pPlayerManager,
        MixxxMainWindow* pWindow,
        Mpris* pMpris)
        : m_pCPAutoDjEnabled(nullptr),
          m_pPlayerManager(pPlayerManager),
          m_pWindow(pWindow),
          m_bComponentsInitialized(false),
          m_pMpris(pMpris) {
    connect(m_pWindow, &MixxxMainWindow::componentsInitialized, this, &MprisPlayer::mixxxComponentsInitialized);
}

QString MprisPlayer::playbackStatus() const {
    if (!m_bComponentsInitialized)
        return kPlaybackStatusStopped;
    for (unsigned int i = 1; i <= m_pPlayerManager->numberOfDecks(); ++i) {
        if (!m_pPlayerManager->getDeck(i)->isTrackPaused()) {
            return kPlaybackStatusPlaying;
        }
    }
    return kPlaybackStatusPaused;
}

QString MprisPlayer::loopStatus() const {
    if (!m_bComponentsInitialized)
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
    return AUTODJENABLED;
}

bool MprisPlayer::canGoPrevious() const {
    return false;
}

bool MprisPlayer::canPlay() const {
    return AUTODJENABLED;
}

bool MprisPlayer::canPause() const {
    return AUTODJENABLED;
}

bool MprisPlayer::canSeek() const {
    return AUTODJENABLED;
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

//Ugly hack because Control Proxy and Control Object are
void MprisPlayer::mixxxComponentsInitialized() {
    m_bComponentsInitialized = true;
    m_pCPAutoDjEnabled = new ControlProxy(ConfigKey("[AutoDJ]", "enabled"), this);
    m_pCPAutoDjEnabled->connectValueChanged(this, &MprisPlayer::autoDJStateChanged);
}

void MprisPlayer::autoDJStateChanged(double enabled) {
    ;
    broadcastPropertiesChange(enabled);
}

void MprisPlayer::broadcastPropertiesChange(bool enabled) {
    for (const QString& property : autoDJDependentProperties) {
        m_pMpris->notifyPropertyChanged(playerInterfaceName,
                property,
                enabled);
    }
}
