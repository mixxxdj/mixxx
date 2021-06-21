#include "skin/qml/qmlplayermanagerproxy.h"

#include <QQmlEngine>

#include "mixer/playermanager.h"
#include "skin/qml/qmlplayerproxy.h"

namespace mixxx {
namespace skin {
namespace qml {

QmlPlayerManagerProxy::QmlPlayerManagerProxy(
        std::shared_ptr<PlayerManager> pPlayerManager, QObject* parent)
        : QObject(parent), m_pPlayerManager(pPlayerManager) {
    connect(this,
            &QmlPlayerManagerProxy::loadLocationToPlayer,
            m_pPlayerManager.get(),
            &PlayerManager::loadLocationToPlayer);
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
            [this, group](const QString& trackLocation) {
                emit loadLocationToPlayer(trackLocation, group);
            });
    connect(pPlayerProxy,
            &QmlPlayerProxy::cloneFromGroup,
            this,
            [this, group](const QString& sourceGroup) {
                m_pPlayerManager->slotCloneDeck(sourceGroup, group);
            });
    return pPlayerProxy;
}

} // namespace qml
} // namespace skin
} // namespace mixxx
