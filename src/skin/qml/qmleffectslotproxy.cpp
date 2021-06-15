#include "skin/qml/qmleffectslotproxy.h"

#include <QObject>

#include "effects/effectrack.h"
#include "effects/effectslot.h"

namespace mixxx {
namespace skin {
namespace qml {

QmlEffectSlotProxy::QmlEffectSlotProxy(EffectRackPointer pRack,
        EffectChainSlotPointer pChainSlot,
        EffectSlotPointer pEffectSlot,
        QObject* parent)
        : QObject(parent),
          m_pRack(pRack),
          m_pChainSlot(pChainSlot),
          m_pEffectSlot(pEffectSlot) {
    DEBUG_ASSERT(m_pRack);
    DEBUG_ASSERT(m_pChainSlot);
    DEBUG_ASSERT(m_pEffectSlot);
    connect(m_pEffectSlot.get(),
            &EffectSlot::updated,
            this,
            &QmlEffectSlotProxy::effectIdChanged);
}

int QmlEffectSlotProxy::getRackNumber() const {
    return m_pRack->getRackNumber();
}

QString QmlEffectSlotProxy::getRackGroup() const {
    return m_pRack->getGroup();
}

int QmlEffectSlotProxy::getChainSlotNumber() const {
    return m_pChainSlot->getChainSlotNumber();
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
    const EffectPointer pEffect = m_pEffectSlot->getEffect();
    if (!pEffect) {
        return QString();
    }

    const EffectManifestPointer pManifest = pEffect->getManifest();
    return pManifest->id();
}

void QmlEffectSlotProxy::setEffectId(const QString& effectId) {
    m_pRack->maybeLoadEffect(
            m_pChainSlot->getChainSlotNumber(),
            m_pEffectSlot->getEffectSlotNumber(),
            effectId);
}

} // namespace qml
} // namespace skin
} // namespace mixxx
