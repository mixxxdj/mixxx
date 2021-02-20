#include "broadcast/mpris/mediaplayer2.h"

#include <QApplication>
#include <QDebug>
#include <QString>
#include <QWidget>

#include "mixxx.h"
#include "moc_mediaplayer2.cpp"
#include "sources/soundsourceproxy.h"

MediaPlayer2::MediaPlayer2(MixxxMainWindow* pMixxx, QObject* parent)
        : QDBusAbstractAdaptor(parent),
          m_pMixxx(pMixxx) {
}

bool MediaPlayer2::canQuit() const {
    return true;
}

bool MediaPlayer2::fullscreen() const {
    return m_pMixxx->isFullScreen();
}

void MediaPlayer2::setFullscreen(bool fullscreen) {
    m_pMixxx->slotViewFullScreen(fullscreen);
}

bool MediaPlayer2::canSetFullscreen() const {
    return true;
}

bool MediaPlayer2::canRaise() const {
    return true;
}

bool MediaPlayer2::hasTrackList() const {
    return false;
}

QString MediaPlayer2::identity() const {
    return "Mixxx";
}

QString MediaPlayer2::desktopEntry() const {
    return "mixxx";
}

QStringList MediaPlayer2::supportedUriSchemes() const {
    QStringList protocols;
    protocols.append("file");
    return protocols;
}

QStringList MediaPlayer2::supportedMimeTypes() const {
    QStringList ret;
    ret << QStringLiteral("audio/mpeg") // for mp3
        << QStringLiteral("audio/ogg")  // for ogg and opus
        << QStringLiteral("audio/flac")
        << QStringLiteral("audio/x-flac")
        << QStringLiteral("audio/wav")
        << QStringLiteral("audio/x-wav");
    return ret;
}

void MediaPlayer2::Raise() {
    m_pMixxx->raise();
}

void MediaPlayer2::Quit() {
    QApplication::quit();
}
