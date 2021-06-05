#include "skin/qml/qmlplayermanagerproxy.h"

#include "mixer/playermanager.h"
#include "skin/qml/qmlplayerproxy.h"

namespace mixxx {
namespace skin {
namespace qml {

QmlPlayerManagerProxy::QmlPlayerManagerProxy(PlayerManager* pPlayerManager, QObject* parent)
        : QObject(parent), m_pPlayerManager(pPlayerManager) {
}

QObject* QmlPlayerManagerProxy::getPlayer(const QString& group) {
    BaseTrackPlayer* pPlayer = m_pPlayerManager->getPlayer(group);
    if (!pPlayer) {
        qWarning() << "PlayerManagerProxy failed to find player for group" << group;
        return nullptr;
    }
    return new QmlPlayerProxy(pPlayer, this);
}

} // namespace qml
} // namespace skin
} // namespace mixxx
