#include "qml/qmleffectsmanagerproxy.h"

#include <QQmlEngine>
#include <memory>

#include "effects/effectchain.h"
#include "moc_qmleffectsmanagerproxy.cpp"
#include "qml/qmlchainpresetmodel.h"

namespace mixxx {
namespace qml {

QmlEffectsManagerProxy::QmlEffectsManagerProxy(
        std::shared_ptr<EffectsManager> pEffectsManager, QObject* parent)
        : QObject(parent),
          m_pEffectsManager(std::move(pEffectsManager)),
          m_pVisibleEffectsModel(
                  new QmlVisibleEffectsModel(m_pEffectsManager, this)),
          m_pQuickChainPresetModel(
                  new QmlChainPresetModel(m_pEffectsManager,
                          QmlChainPresetModel::PresetType::Quick,
                          this)),
          m_pStandardChainPresetModel(
                  new QmlChainPresetModel(m_pEffectsManager,
                          QmlChainPresetModel::PresetType::Standard,
                          this)) {
    for (int unitIndex = 0; unitIndex < kNumStandardEffectUnits; ++unitIndex) {
        const auto pEffectUnit = m_pEffectsManager->getStandardEffectChain(unitIndex);
        if (pEffectUnit) {
            m_effectUnitProxies.append(new QmlEffectUnitProxy(
                    m_pEffectsManager, unitIndex + 1, pEffectUnit, this));
        }
    }
}

QmlEffectUnitProxy* QmlEffectsManagerProxy::getEffectUnit(int unitNumber) const {
    const int unitIndex = unitNumber - 1;
    if (unitIndex < 0 || unitIndex >= m_effectUnitProxies.size()) {
        qWarning() << "QmlEffectsManagerProxy: Effect Unit" << unitNumber
                   << "not found!";
        return nullptr;
    }
    return m_effectUnitProxies.at(unitIndex);
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

// static
QmlEffectsManagerProxy* QmlEffectsManagerProxy::create(
        QQmlEngine* pQmlEngine, QJSEngine* pJsEngine) {
    // The implementation of this method is mostly taken from the code example
    // that shows the replacement for `qmlRegisterSingletonInstance()` when
    // using `QML_SINGLETON`.
    // https://doc.qt.io/qt-6/qqmlengine.html#QML_SINGLETON

    // The instance has to exist before it is used. We cannot replace it.
    VERIFY_OR_DEBUG_ASSERT(s_pEffectManager) {
        qWarning() << "EffectManager hasn't been registered yet";
        return nullptr;
    }
    return new QmlEffectsManagerProxy(s_pEffectManager, pQmlEngine);
}

} // namespace qml
} // namespace mixxx
