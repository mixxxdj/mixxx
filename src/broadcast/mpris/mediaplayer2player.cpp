#include "mediaplayer2player.h"

#include <QApplication>

#include "mixer/playerinfo.h"
#include "mixer/playermanager.h"
#include "control/controlobject.h"

#include "moc_mediaplayer2player.cpp"


static const char* kPlaybackStatusPlaying = "Playing";
//static const char* kPlaybackStatusPaused = "Paused";
static const char* kPlaybackStatusStopped = "Stopped";

// the playback will stop when there are no more tracks to play
static const char* kLoopStatusNone = "None";
// The current track will start again from the begining once it has finished playing
static const char* kLoopStatusTrack = "Track";
// The playback loops through a list of tracks
//static const char* kLoopStatusPlaylist = "Playlist";


MediaPlayer2Player::MediaPlayer2Player(QObject *parent)
    : QDBusAbstractAdaptor(parent) {
}

MediaPlayer2Player::~MediaPlayer2Player() {
}

QString MediaPlayer2Player::playbackStatus() const {
    int currentPlayingDeck = PlayerInfo::instance().getCurrentPlayingDeck();
    if (currentPlayingDeck == -1)
        return kPlaybackStatusStopped;
    return kPlaybackStatusPlaying;
}

QString MediaPlayer2Player::loopStatus() const {
    TrackPointer pTrack;
    int deckIndex = PlayerInfo::instance().getCurrentPlayingDeck();
    if (deckIndex >= 0) {
        QString group = PlayerManager::groupForDeck(deckIndex);
        ConfigKey key(group, "repeat");
        if (ControlObject::get(key) > 0.0) {
            return QString(kLoopStatusTrack);
        }
        // TODO: Decide when how to Handle playlist repeat mode
    }
    return QString(kLoopStatusNone);
}

void MediaPlayer2Player::setLoopStatus(const QString& value) {
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

double MediaPlayer2Player::rate() const {
    double rate = 1.0;
    return rate;
}

void MediaPlayer2Player::setRate(double value) {
    Q_UNUSED(value);
}

bool MediaPlayer2Player::shuffle() const {
    bool shuffle = false;
    return shuffle;
}

void MediaPlayer2Player::setShuffle(bool value) {
    Q_UNUSED(value);
}

QVariantMap MediaPlayer2Player::metadata() const {
    QVariantMap metadata;
    return metadata;
}

double MediaPlayer2Player::volume() const {
    double volume = 0.0;
    return volume;
}

void MediaPlayer2Player::setVolume(double value) {
    Q_UNUSED(value);
}

qlonglong MediaPlayer2Player::position() const {
    qlonglong position = 0;
    return position;
}

double MediaPlayer2Player::minimumRate() const {
    return 1.0;
}

double MediaPlayer2Player::maximumRate() const {
    return 1.0;
}

bool MediaPlayer2Player::canGoNext() const {
    return false;
}

bool MediaPlayer2Player::canGoPrevious() const {
    return false;
}

bool MediaPlayer2Player::canPlay() const {
    return false;
}

bool MediaPlayer2Player::canPause() const {
    return false;
}

bool MediaPlayer2Player::canSeek() const {
    return false;
}

bool MediaPlayer2Player::canControl() const {
    return false;
}

void MediaPlayer2Player::Next() {
}

void MediaPlayer2Player::Previous() {
}

void MediaPlayer2Player::Pause() {
//    TrackPointer pTrack;
//    int deckIndex = PlayerInfo::instance().getCurrentPlayingDeck();
//    if (deckIndex >= 0) {
//        QString group = PlayerManager::groupForDeck(deckIndex);
//        ConfigKey key(group, "play");
//        ControlObject::set(key, 0.0);
//    }
}

void MediaPlayer2Player::PlayPause() {
}

void MediaPlayer2Player::Stop() {
//    TrackPointer pTrack;
//    int deckIndex = PlayerInfo::instance().getCurrentPlayingDeck();
//    if (deckIndex >= 0) {
//        QString group = PlayerManager::groupForDeck(deckIndex);
//        ConfigKey key(group, "play");
//        ControlObject::set(key, 0.0);
//    }
}

void MediaPlayer2Player::Play() {
}

void MediaPlayer2Player::Seek(qlonglong offset) {
    Q_UNUSED(offset);
}

void MediaPlayer2Player::SetPosition(const QDBusObjectPath& trackId,
                                     qlonglong position) {
    Q_UNUSED(trackId);
    Q_UNUSED(position);
}

void MediaPlayer2Player::OpenUri(const QString& uri) {
    Q_UNUSED(uri);
}
