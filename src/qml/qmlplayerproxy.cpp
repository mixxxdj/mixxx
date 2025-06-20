#include "qml/qmlplayerproxy.h"

#include <QBuffer>
#include <QQmlEngine>

#include "mixer/basetrackplayer.h"
#include "moc_qmlplayerproxy.cpp"
#include "qmltrackproxy.h"
#include "track/track.h"

namespace mixxx {
namespace qml {

QmlPlayerProxy::QmlPlayerProxy(BaseTrackPlayer* pTrackPlayer, QObject* parent)
        : QObject(parent),
          m_pTrackPlayer(pTrackPlayer) {
    connect(m_pTrackPlayer,
            &BaseTrackPlayer::loadingTrack,
            this,
            &QmlPlayerProxy::slotLoadingTrack);
    connect(m_pTrackPlayer,
            &BaseTrackPlayer::newTrackLoaded,
            this,
            &QmlPlayerProxy::slotTrackLoaded);
    connect(m_pTrackPlayer,
            &BaseTrackPlayer::trackUnloaded,
            this,
            &QmlPlayerProxy::slotTrackUnloaded);
    if (m_pTrackPlayer && m_pTrackPlayer->getLoadedTrack()) {
        slotTrackLoaded(pTrackPlayer->getLoadedTrack());
    }
}

void QmlPlayerProxy::loadTrack(QmlTrackProxy* track, bool play) {
    if (track == nullptr || track->internal() == nullptr) {
        return;
    }
    emit loadTrackRequested(track->internal(),
#ifdef __STEM__
            mixxx::StemChannel::All,
#endif
            play);
}

void QmlPlayerProxy::loadTrackFromLocation(const QString& trackLocation, bool play) {
    emit loadTrackFromLocationRequested(trackLocation, play);
}

void QmlPlayerProxy::loadTrackFromLocationUrl(const QUrl& trackLocationUrl, bool play) {
    if (trackLocationUrl.isLocalFile()) {
        loadTrackFromLocation(trackLocationUrl.toLocalFile(), play);
    } else {
        qWarning() << "QmlPlayerProxy: URL" << trackLocationUrl << "is not a local file!";
    }
}

void QmlPlayerProxy::slotTrackLoaded(TrackPointer pTrack) {
    m_pCurrentTrack = pTrack;
    emit trackChanged();
    emit trackLoaded();
}

void QmlPlayerProxy::slotTrackUnloaded(TrackPointer pOldTrack) {
    VERIFY_OR_DEBUG_ASSERT(pOldTrack == m_pCurrentTrack) {
        qWarning() << "QML Player proxy was expected to contain "
                   << pOldTrack.get() << "as active track but got"
                   << m_pCurrentTrack.get();
    }
    if (m_pCurrentTrack != nullptr) {
        disconnect(m_pCurrentTrack.get(), nullptr, this, nullptr);
    }
    m_pCurrentTrack.reset();
    emit trackChanged();
    emit trackUnloaded();
}

QmlTrackProxy* QmlPlayerProxy::currentTrack() {
    auto* pTrack = new QmlTrackProxy(m_pCurrentTrack, this);
    QQmlEngine::setObjectOwnership(pTrack, QQmlEngine::JavaScriptOwnership);
    return pTrack;
}

void QmlPlayerProxy::slotLoadingTrack(TrackPointer pNewTrack, TrackPointer pOldTrack) {
    if (pNewTrack.get() == m_pCurrentTrack.get()) {
        emit trackLoading();
        return;
    }

    if (m_pCurrentTrack != nullptr) {
        disconnect(m_pCurrentTrack.get(), nullptr, this, nullptr);
    }
    m_pCurrentTrack = pNewTrack;
    emit trackChanged();
    emit trackLoading();
}

bool QmlPlayerProxy::isLoaded() const {
    return m_pCurrentTrack != nullptr;
}

} // namespace qml
} // namespace mixxx
