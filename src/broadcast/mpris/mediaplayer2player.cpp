#include "broadcast/mpris/mediaplayer2player.h"

#include <QApplication>
#include <QDBusConnection>
#include <QDBusMessage>

#include "moc_mediaplayer2player.cpp"

MediaPlayer2Player::MediaPlayer2Player(PlayerManagerInterface* playerManager,
        Mpris* pMpris,
        UserSettingsPointer pSettings)
        : QDBusAbstractAdaptor(pMpris),
          m_mprisPlayer(playerManager, pMpris, pSettings) {
}

QString MediaPlayer2Player::playbackStatus() const {
    return m_mprisPlayer.playbackStatus();
}

QString MediaPlayer2Player::loopStatus() const {
    return m_mprisPlayer.loopStatus();
}

void MediaPlayer2Player::setLoopStatus(const QString& value) {
    m_mprisPlayer.setLoopStatus(value);
}

double MediaPlayer2Player::rate() const {
    return m_mprisPlayer.rate();
}

void MediaPlayer2Player::setRate(double value) {
    m_mprisPlayer.setRate(value);
}

bool MediaPlayer2Player::shuffle() const {
    return false;
}

void MediaPlayer2Player::setShuffle(bool value) {
    Q_UNUSED(value);
}

QVariantMap MediaPlayer2Player::metadata() {
    return m_mprisPlayer.metadata();
}

double MediaPlayer2Player::volume() const {
    return m_mprisPlayer.volume();
}

void MediaPlayer2Player::setVolume(double value) {
    m_mprisPlayer.setVolume(value);
}

qlonglong MediaPlayer2Player::position() const {
    return m_mprisPlayer.position();
}

double MediaPlayer2Player::minimumRate() const {
    return -1.0;
}

double MediaPlayer2Player::maximumRate() const {
    return 1.0;
}

bool MediaPlayer2Player::canGoNext() const {
    return m_mprisPlayer.canGoNext();
}

bool MediaPlayer2Player::canGoPrevious() const {
    return m_mprisPlayer.canGoPrevious();
}

bool MediaPlayer2Player::canPlay() const {
    return m_mprisPlayer.canPlay();
}

bool MediaPlayer2Player::canPause() const {
    return m_mprisPlayer.canPause();
}

bool MediaPlayer2Player::canSeek() const {
    return m_mprisPlayer.canSeek();
}

bool MediaPlayer2Player::canControl() const {
    return true;
}

void MediaPlayer2Player::Next() {
    m_mprisPlayer.nextTrack();
}

void MediaPlayer2Player::Previous() {
}

void MediaPlayer2Player::Pause() {
    m_mprisPlayer.pause();
}

void MediaPlayer2Player::PlayPause() {
    m_mprisPlayer.playPause();
}

void MediaPlayer2Player::Stop() {
    m_mprisPlayer.stop();
}

void MediaPlayer2Player::Play() {
    m_mprisPlayer.play();
}

void MediaPlayer2Player::Seek(qlonglong offset) {
    bool success;
    m_mprisPlayer.seek(offset, success);
}

void MediaPlayer2Player::SetPosition(const QDBusObjectPath& trackId,
        qlonglong position) {
    bool success;
    m_mprisPlayer.setPosition(trackId, position, success);
}

void MediaPlayer2Player::OpenUri(const QString& uri) {
    m_mprisPlayer.openUri(uri);
}
