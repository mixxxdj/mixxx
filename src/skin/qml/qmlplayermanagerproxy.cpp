#include "skin/qml/qmlplayermanagerproxy.h"

#include "mixer/playermanager.h"
#include "skin/qml/qmlplayerproxy.h"

namespace mixxx {
namespace skin {
namespace qml {

QmlPlayerManagerProxy::QmlPlayerManagerProxy(PlayerManager* pPlayerManager, QObject* parent)
        : QObject(parent), m_pPlayerManager(pPlayerManager) {
    connect(this,
            &QmlPlayerManagerProxy::loadLocationToPlayer,
            m_pPlayerManager,
            &PlayerManager::loadLocationToPlayer);
}

QObject* QmlPlayerManagerProxy::getPlayer(const QString& group) {
    BaseTrackPlayer* pPlayer = m_pPlayerManager->getPlayer(group);
    if (!pPlayer) {
        qWarning() << "PlayerManagerProxy failed to find player for group" << group;
        return nullptr;
    }
    QmlPlayerProxy* pPlayerProxy = new QmlPlayerProxy(pPlayer, this);
    connect(pPlayerProxy,
            &QmlPlayerProxy::loadTrackFromLocationRequested,
            this,
            [this, group](const QString& trackLocation) {
                emit loadLocationToPlayer(trackLocation, group);
            });
    return pPlayerProxy;
}

} // namespace qml
} // namespace skin
} // namespace mixxx
