#include "broadcast/mpris/mediaplayer2.h"

#include <QApplication>
#include <QDebug>
#include <QString>
#include <QWidget>

#include "mixxxmainwindow.h"
#include "moc_mediaplayer2.cpp"
#include "sources/soundsourceproxy.h"

MediaPlayer2::MediaPlayer2(QObject* parent)
        : QDBusAbstractAdaptor(parent) {
}

bool MediaPlayer2::canQuit() const {
    return true;
}

bool MediaPlayer2::fullscreen() const {
    // TODO use CO
    return false;
}

void MediaPlayer2::setFullscreen(bool) {
    // TODO use CO
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
    // TODO use signal
}

void MediaPlayer2::Quit() {
    QApplication::quit();
}
