#include "qml/qmlplaylistproxy.h"

#include <QBuffer>
#include <QQmlEngine>

#include "moc_qmlplaylistproxy.cpp"
#include "qmltrackproxy.h"
#include "track/track.h"
#include "util/assert.h"

namespace mixxx {
namespace qml {

QmlPlaylistProxy::QmlPlaylistProxy(QObject* parent, PlaylistDAO& dao, int pid, const QString& name)
        : QObject(parent),
          m_id(pid),
          m_name(name),
          m_dao(dao) {
}

Q_INVOKABLE void QmlPlaylistProxy::addTrack(QmlTrackProxy* track) {
    VERIFY_OR_DEBUG_ASSERT(track && track->internal()) {
        return;
    }
    m_dao.appendTrackToPlaylist(track->internal()->getId(), m_id);
}

QmlLibraryTrackListModel* QmlPlaylistProxy::model() const {
    return nullptr;
}

} // namespace qml
} // namespace mixxx
