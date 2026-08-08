#include "broadcast/mpris/mpris.h"

#include <QDBusMessage>

#include "broadcast/mpris/mediaplayer2.h"
#include "broadcast/mpris/mediaplayer2player.h"
#include "broadcast/mpris/mediaplayer2playlists.h"
#include "broadcast/mpris/mediaplayer2tracklist.h"
#include "moc_mpris.cpp"
#include "track/track.h"

namespace {

const QString kServiceName = QStringLiteral("org.mpris.MediaPlayer2.mixxx");
const QString KObjectPath = QStringLiteral("/org/mpris/MediaPlayer2");

} // namespace

Mpris::Mpris(PlayerManagerInterface* pPlayerManager,
        UserSettingsPointer pSettings)
        : m_busConnection(QDBusConnection::connectToBus(QDBusConnection::SessionBus, kServiceName)),
          m_pPlayer(new MediaPlayer2Player(pPlayerManager,
                  this,
                  pSettings)) {
    // Classes derived from QDBusAbstractAdaptor must be created
    // on the heap using the new operator and must not be deleted
    // by the user (they will be deleted automatically when the object
    // they are connected to is also deleted).
    // http://doc.qt.io/qt-5/qdbusabstractadaptor.html
    new MediaPlayer2(this);
    m_busConnection.registerObject(KObjectPath, this);
    m_busConnection.registerService(kServiceName);
}

Mpris::~Mpris() {
    m_busConnection.unregisterObject(KObjectPath);
    m_busConnection.unregisterService(kServiceName);
    QDBusConnection::disconnectFromBus(kServiceName);
}

void Mpris::notifyPropertyChanged(const QString& interface,
        const QString& propertyName,
        const QVariant& propertyValue) {
    QDBusMessage signal = QDBusMessage::createSignal(
            KObjectPath,
            QStringLiteral("org.freedesktop.DBus.Properties"),
            QStringLiteral("PropertiesChanged"));
    signal << interface;
    QVariantMap changedProps;
    changedProps.insert(propertyName, propertyValue);
    signal << changedProps;
    signal << QStringList();
    m_busConnection.send(signal);
}
