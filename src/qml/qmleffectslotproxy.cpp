#include "qml/qmleffectslotproxy.h"

#include <QObject>
#include <QQmlEngine>

#include "effects/effectchain.h"
#include "effects/effectslot.h"
#include "moc_qmleffectslotproxy.cpp"

namespace mixxx {
namespace qml {

QmlEffectSlotProxy::QmlEffectSlotProxy(
        std::shared_ptr<EffectsManager> pEffectsManager,
        int unitNumber,
        EffectChainPointer pChainSlot,
        EffectSlotPointer pEffectSlot,
        QObject* parent)
        : QObject(parent),
          m_pEffectsManager(pEffectsManager),
          m_unitNumber(unitNumber),
          m_pChainSlot(pChainSlot),
          m_pEffectSlot(pEffectSlot) {
    DEBUG_ASSERT(m_pChainSlot);
    DEBUG_ASSERT(m_pEffectSlot);
    connect(m_pEffectSlot.get(),
            &EffectSlot::effectChanged,
            this,
            &QmlEffectSlotProxy::effectIdChanged);
    connect(m_pEffectSlot.get(),
            &EffectSlot::effectChanged,
            this,
            &QmlEffectSlotProxy::parametersModelChanged);
    connect(m_pEffectSlot.get(),
            &EffectSlot::parametersChanged,
            this,
            &QmlEffectSlotProxy::parametersModelChanged);
}

int QmlEffectSlotProxy::getChainSlotNumber() const {
    return m_unitNumber;
}

QString QmlEffectSlotProxy::getChainSlotGroup() const {
    return m_pChainSlot->getGroup();
}

int QmlEffectSlotProxy::getNumber() const {
    return m_pEffectSlot->getEffectSlotNumber();
}

QString QmlEffectSlotProxy::getGroup() const {
    return m_pEffectSlot->getGroup();
}

QString QmlEffectSlotProxy::getEffectId() const {
    return m_pEffectSlot->id();
}

void QmlEffectSlotProxy::setEffectId(const QString& effectId) {
    const EffectManifestPointer pManifest =
            m_pEffectsManager->getBackendManager()->getManifestFromUniqueId(
                    effectId);
    m_pEffectSlot->loadEffectWithDefaults(pManifest);
}

QmlEffectManifestParametersModel* QmlEffectSlotProxy::getParametersModel() const {
    const EffectManifestPointer pManifest = m_pEffectSlot->getManifest();
    if (!pManifest) {
        return nullptr;
    }

    QmlEffectManifestParametersModel* pModel = new QmlEffectManifestParametersModel(pManifest);
    QQmlEngine::setObjectOwnership(pModel, QQmlEngine::JavaScriptOwnership);
    return pModel;
}

} // namespace qml
} // namespace mixxx
