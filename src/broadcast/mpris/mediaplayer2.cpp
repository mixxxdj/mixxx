#include "mediaplayer2.h"

#include <QApplication>
#include <QDebug>
#include <QWidget>

#include "mixxxmainwindow.h"
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
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return false;
#else
    return m_pMixxx && m_pMixxx->isFullScreen();
#endif
}

void MediaPlayer2::setFullscreen(bool fullscreen) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    if (m_pMixxx) {
        m_pMixxx->slotViewFullScreen(fullscreen);
    }
#endif
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
    ret << "audio/mpeg"
        << "audio/ogg";
    return ret;
}

void MediaPlayer2::Raise() {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    if (m_pMixxx) {
        m_pMixxx->raise();
    }
#endif
}

void MediaPlayer2::Quit() {
    QApplication::quit();
}
