#include "skin/qml/qmleffectsmanagerproxy.h"

#include <memory>

#include "effects/effectrack.h"
#include "skin/qml/qmlvisibleeffectsmodel.h"

namespace mixxx {
namespace skin {
namespace qml {

QmlEffectsManagerProxy::QmlEffectsManagerProxy(
        std::shared_ptr<EffectsManager> pEffectsManager, QObject* parent)
        : QObject(parent),
          m_pEffectsManager(pEffectsManager),
          m_pVisibleEffectsModel(
                  new QmlVisibleEffectsModel(pEffectsManager, this)) {
}
void QmlEffectsManagerProxy::loadEffect(int rackNumber,
        int unitNumber,
        int effectNumber,
        const QString& effectId) {
    // Subtract 1 from all numbers, because internally our indices are
    // zero-based
    const int rackIndex = rackNumber - 1;

    const auto pEffectRack = m_pEffectsManager->getStandardEffectRack(rackIndex);
    if (!pEffectRack) {
        qWarning() << "QmlEffectsManagerProxy: Effect Rack" << rackNumber << "not found!";
        return;
    }

    pEffectRack->maybeLoadEffect(unitNumber - 1, effectNumber - 1, effectId);
}

} // namespace qml
} // namespace skin
} // namespace mixxx
