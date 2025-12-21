#include "qml/qmlcrateproxy.h"

#include <QBuffer>
#include <QQmlEngine>

#include "library/trackcollection.h"
#include "library/trackset/crate/crate.h"
#include "moc_qmlcrateproxy.cpp"
#include "qmltrackproxy.h"
#include "track/track.h"
#include "util/assert.h"

namespace mixxx {
namespace qml {

QmlCrateProxy::QmlCrateProxy(QObject* parent,
        TrackCollection* trackCollection,
        const CrateSummary& crate)
        : QObject(parent),
          m_trackCollection(trackCollection),
          m_internal(crate) {
}

Q_INVOKABLE void QmlCrateProxy::addTrack(QmlTrackProxy* track) {
    VERIFY_OR_DEBUG_ASSERT(track && track->internal()) {
        return;
    }
    m_trackCollection->addCrateTracks(m_internal.getId(), {track->internal()->getId()});
}
Q_INVOKABLE void QmlCrateProxy::removeTrack(QmlTrackProxy* track) {
    VERIFY_OR_DEBUG_ASSERT(track && track->internal()) {
        return;
    }
    m_trackCollection->removeCrateTracks(m_internal.getId(), {track->internal()->getId()});
}

QString QmlCrateProxy::name() const {
    return m_internal.getName();
}

void QmlCrateProxy::setName(const QString& value) {
    m_internal.setName(value);
}

bool QmlCrateProxy::isLocked() const {
    return m_internal.isLocked();
}

void QmlCrateProxy::setLocked(bool lock) {
    m_internal.setLocked(lock);
    m_trackCollection->updateCrate(m_internal);
}

uint QmlCrateProxy::trackCount() const {
    return m_internal.getTrackCount();
}

QmlLibraryTrackListModel* QmlCrateProxy::model() const {
    return nullptr;
}

} // namespace qml
} // namespace mixxx
