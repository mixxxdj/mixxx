#include "qml/qmlplayermanagerproxy.h"

#include <QQmlEngine>

#include "mixer/playermanager.h"
#include "qml/qmlplayerproxy.h"

namespace mixxx {
namespace qml {

QmlPlayerManagerProxy::QmlPlayerManagerProxy(
        std::shared_ptr<PlayerManager> pPlayerManager, QObject* parent)
        : QObject(parent), m_pPlayerManager(pPlayerManager) {
}

QObject* QmlPlayerManagerProxy::getPlayer(const QString& group) {
    BaseTrackPlayer* pPlayer = m_pPlayerManager->getPlayer(group);
    if (!pPlayer) {
        qWarning() << "PlayerManagerProxy failed to find player for group" << group;
        return nullptr;
    }

    // Don't set a parent here, so that the QML engine deletes the object when
    // the corresponding JS object is garbage collected.
    QmlPlayerProxy* pPlayerProxy = new QmlPlayerProxy(pPlayer);
    QQmlEngine::setObjectOwnership(pPlayerProxy, QQmlEngine::JavaScriptOwnership);
    connect(pPlayerProxy,
            &QmlPlayerProxy::loadTrackFromLocationRequested,
            this,
            [this, group](const QString& trackLocation, bool play) {
                loadLocationToPlayer(trackLocation, group, play);
            });
    connect(pPlayerProxy,
            &QmlPlayerProxy::cloneFromGroup,
            this,
            [this, group](const QString& sourceGroup) {
                m_pPlayerManager->slotCloneDeck(sourceGroup, group);
            });
    return pPlayerProxy;
}

void QmlPlayerManagerProxy::loadLocationIntoNextAvailableDeck(
        const QString& trackLocation, bool play) {
    m_pPlayerManager->slotLoadLocationIntoNextAvailableDeck(trackLocation, play);
}

void QmlPlayerManagerProxy::loadLocationUrlIntoNextAvailableDeck(
        const QUrl& trackLocationUrl, bool play) {
    if (trackLocationUrl.isLocalFile()) {
        loadLocationIntoNextAvailableDeck(trackLocationUrl.toLocalFile(), play);
    } else {
        qWarning() << "QmlPlayerManagerProxy: URL" << trackLocationUrl << "is not a local file!";
    }
}

void QmlPlayerManagerProxy::loadLocationToPlayer(
        const QString& location, const QString& group, bool play) {
    m_pPlayerManager->slotLoadLocationToPlayer(location, group, play);
}

} // namespace qml
} // namespace mixxx
