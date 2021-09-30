#include "qml/qmleffectsmanagerproxy.h"

#include <QQmlEngine>
#include <memory>

#include "effects/effectchain.h"
#include "qml/qmleffectslotproxy.h"
#include "qml/qmlvisibleeffectsmodel.h"

namespace mixxx {
namespace qml {

QmlEffectsManagerProxy::QmlEffectsManagerProxy(
        std::shared_ptr<EffectsManager> pEffectsManager, QObject* parent)
        : QObject(parent),
          m_pEffectsManager(pEffectsManager),
          m_pVisibleEffectsModel(
                  new QmlVisibleEffectsModel(pEffectsManager, this)) {
}

QmlEffectSlotProxy* QmlEffectsManagerProxy::getEffectSlot(int unitNumber, int effectNumber) const {
    // Subtract 1 from all numbers, because internally our indices are
    // zero-based
    const int unitIndex = unitNumber - 1;
    const auto pEffectUnit = m_pEffectsManager->getStandardEffectChain(unitIndex);
    if (!pEffectUnit) {
        qWarning() << "QmlEffectsManagerProxy: Effect Unit" << unitNumber
                   << "not found!";
        return nullptr;
    }

    const int effectIndex = effectNumber - 1;
    const auto pEffectSlot = pEffectUnit->getEffectSlot(effectIndex);
    if (!pEffectSlot) {
        qWarning() << "QmlEffectsManagerProxy: Effect Slot" << effectNumber
                   << "in Unit" << unitNumber << "not found!";
        return nullptr;
    }

    // Don't set a parent here, so that the QML engine deletes the object when
    // the corresponding JS object is garbage collected.
    QmlEffectSlotProxy* pEffectSlotProxy = new QmlEffectSlotProxy(
            m_pEffectsManager, unitNumber, pEffectUnit, pEffectSlot);
    QQmlEngine::setObjectOwnership(pEffectSlotProxy, QQmlEngine::JavaScriptOwnership);
    return pEffectSlotProxy;
}

} // namespace qml
} // namespace mixxx
