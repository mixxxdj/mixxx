#include "broadcast/mpris/mediaplayer2.h"

#include <QApplication>
#include <QDebug>
#include <QString>
#include <QWidget>
#include <utility>

#include "mixxxmainwindow.h"
#include "moc_mediaplayer2.cpp"
#include "sources/soundsourceproxy.h"

MediaPlayer2::MediaPlayer2(QObject* parent)
        : QDBusAbstractAdaptor(parent),
          m_pMixxx(nullptr) {
    const auto widgetList = qApp->topLevelWidgets();
    for (auto* pWidget : std::as_const(widgetList)) {
        if ((m_pMixxx = qobject_cast<MixxxMainWindow*>(pWidget))) {
            break;
        }
    }
}

bool MediaPlayer2::canQuit() const {
    return true;
}

bool MediaPlayer2::fullscreen() const {
    return m_pMixxx && m_pMixxx->isFullScreen();
}

void MediaPlayer2::setFullscreen(bool value) {
    if (!m_pMixxx) {
        return;
    }
    m_pMixxx->slotViewFullScreen(value);
}

bool MediaPlayer2::canSetFullscreen() const {
    return m_pMixxx != nullptr;
}

bool MediaPlayer2::canRaise() const {
    return m_pMixxx != nullptr;
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
    return {"file"};
}

QStringList MediaPlayer2::supportedMimeTypes() const {
    return {
            "audio/flac",
            "audio/mpeg",
            "audio/ogg",
            "audio/wav",
            "audio/x-flac",
            "audio/x-wav",
    };
}

void MediaPlayer2::Raise() {
    if (!m_pMixxx) {
        return;
    }
    m_pMixxx->activateWindow();
    m_pMixxx->raise();
}

void MediaPlayer2::Quit() {
    QApplication::quit();
}
