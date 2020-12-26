#include "mpris.h"

#include <track/track.h>

#include <QtDBus/QtDBus>

#include "broadcast/mpris/mediaplayer2.h"
#include "broadcast/mpris/mediaplayer2player.h"
#include "broadcast/mpris/mediaplayer2playlists.h"
#include "broadcast/mpris/mediaplayer2tracklist.h"
#include "moc_mpris.cpp"

namespace {
const QString busName = "org.mpris.MediaPlayer2.mixxx";
}

Mpris::Mpris(MixxxMainWindow* pMixxx,
        PlayerManagerInterface* pPlayerManager,
        UserSettingsPointer pSettings)
        : m_busConnection(QDBusConnection::connectToBus(QDBusConnection::SessionBus, busName)),
          m_pPlayer(new MediaPlayer2Player(pPlayerManager,
                  this,
                  pMixxx,
                  this,
                  pSettings)) {
    // Classes derived from QDBusAbstractAdaptor must be created
    // on the heap using the new operator and must not be deleted
    // by the user (they will be deleted automatically when the object
    // they are connected to is also deleted).
    // http://doc.qt.io/qt-5/qdbusabstractadaptor.html
    new MediaPlayer2(pMixxx, this);
    m_busConnection.registerObject("/org/mpris/MediaPlayer2", this);
    m_busConnection.registerService("org.mpris.MediaPlayer2.mixxx");
}

Mpris::~Mpris() {
    m_busConnection.unregisterObject("/org/mpris/MediaPlayer2");
    m_busConnection.unregisterService("org.mpris.MediaPlayer2.mixxx");
    QDBusConnection::disconnectFromBus(busName);
}

void Mpris::broadcastCurrentTrack() {
    notifyPropertyChanged("org.mpris.MediaPlayer2.Player",
            "Metadata",
            m_pPlayer->metadata());
}

void Mpris::notifyPropertyChanged(const QString& interface,
        const QString& propertyName,
        const QVariant& propertyValue) {
    QDBusMessage signal = QDBusMessage::createSignal(
            "/org/mpris/MediaPlayer2",
            "org.freedesktop.DBus.Properties",
            "PropertiesChanged");
    signal << interface;
    QVariantMap changedProps;
    changedProps.insert(propertyName, propertyValue);
    signal << changedProps;
    signal << QStringList();
    m_busConnection.send(signal);
}
