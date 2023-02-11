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

// static
QmlEffectsManagerProxy* QmlEffectsManagerProxy::create(
        QQmlEngine* pQmlEngine, QJSEngine* pJsEngine) {
    Q_UNUSED(pQmlEngine);

    // The implementation of this method is mostly taken from the code example
    // that shows the replacement for `qmlRegisterSingletonInstance()` when
    // using `QML_SINGLETON`.
    // https://doc.qt.io/qt-6/qqmlengine.html#QML_SINGLETON

    // The instance has to exist before it is used. We cannot replace it.
    DEBUG_ASSERT(s_pInstance);

    // The engine has to have the same thread affinity as the singleton.
    DEBUG_ASSERT(pJsEngine->thread() == s_pInstance->thread());

    // There can only be one engine accessing the singleton.
    if (s_pJsEngine) {
        DEBUG_ASSERT(pJsEngine == s_pJsEngine);
    } else {
        s_pJsEngine = pJsEngine;
    }

    // Explicitly specify C++ ownership so that the engine doesn't delete
    // the instance.
    QJSEngine::setObjectOwnership(s_pInstance, QJSEngine::CppOwnership);
    return s_pInstance;
}

} // namespace qml
} // namespace mixxx
