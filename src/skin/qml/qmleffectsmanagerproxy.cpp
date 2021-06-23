#include "skin/qml/qmleffectsmanagerproxy.h"

#include <QQmlEngine>
#include <memory>

#include "effects/effectchainslot.h"
#include "effects/effectrack.h"
#include "skin/qml/qmleffectslotproxy.h"
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

QmlEffectSlotProxy* QmlEffectsManagerProxy::getEffectSlot(
        int rackNumber, int unitNumber, int effectNumber) const {
    // Subtract 1 from all numbers, because internally our indices are
    // zero-based
    const int rackIndex = rackNumber - 1;
    const auto pRack = m_pEffectsManager->getStandardEffectRack(rackIndex);
    if (!pRack) {
        qWarning() << "QmlEffectsManagerProxy: Effect Rack" << rackNumber << "not found!";
        return nullptr;
    }

    const int unitIndex = unitNumber - 1;
    const auto pEffectUnit = pRack->getEffectChainSlot(unitIndex);
    if (!pEffectUnit) {
        qWarning() << "QmlEffectsManagerProxy: Effect Unit" << unitNumber
                   << "in Rack" << rackNumber << "not found!";
        return nullptr;
    }

    const int effectIndex = effectNumber - 1;
    const auto pEffectSlot = pEffectUnit->getEffectSlot(effectIndex);
    if (!pEffectSlot) {
        qWarning() << "QmlEffectsManagerProxy: Effect Slot" << effectNumber
                   << "in Unit" << unitNumber << "of Rack" << rackNumber
                   << "not found!";
        return nullptr;
    }

    // Don't set a parent here, so that the QML engine deletes the object when
    // the corresponding JS object is garbage collected.
    QmlEffectSlotProxy* pEffectSlotProxy = new QmlEffectSlotProxy(pRack, pEffectUnit, pEffectSlot);
    QQmlEngine::setObjectOwnership(pEffectSlotProxy, QQmlEngine::JavaScriptOwnership);
    return pEffectSlotProxy;
}

} // namespace qml
} // namespace skin
} // namespace mixxx
