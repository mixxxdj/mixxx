#include "mpris.h"

#include <QtDBus/QtDBus>

#include "broadcast/mpris/mediaplayer2.h"
#include "broadcast/mpris/mediaplayer2player.h"
#include "broadcast/mpris/mediaplayer2tracklist.h"
#include "broadcast/mpris/mediaplayer2playlists.h"
#include "moc_mpris.cpp"


Mpris::Mpris(MixxxMainWindow* pMixxx) {
    QDBusConnection connection = QDBusConnection::sessionBus();
    // Classes derived from QDBusAbstractAdaptor must be created
    // on the heap using the new operator and must not be deleted
    // by the user (they will be deleted automatically when the object
    // they are connected to is also deleted).
    // http://doc.qt.io/qt-5/qdbusabstractadaptor.html
    new MediaPlayer2(pMixxx, this);
    new MediaPlayer2Player(this);
    new MediaPlayer2TrackList(this);
    new MediaPlayer2Playlists(this);
    connection.registerObject("/org/mpris/MediaPlayer2", this);
    connection.registerService("org.mpris.MediaPlayer2.mixxx");
}

Mpris::~Mpris() {
    QDBusConnection::sessionBus().unregisterService("org.mpris.MediaPlayer2.mixxx");
}
