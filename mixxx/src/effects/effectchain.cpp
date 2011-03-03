#include <QMutexLocker>

#include "effects/effectchain.h"

EffectChain::EffectChain(QObject* pParent, unsigned int iChainNumber)
        : QObject(pParent),
          m_mutex(QMutex::Recursive),
          m_iChainNumber(iChainNumber),
          m_group(QString("[EffectChain%1]").arg(iChainNumber)) {
}

EffectChain::~EffectChain() {
}

unsigned int EffectChain::numSlots() const {
    qDebug() << debugString() << "numSlots";
    QMutexLocker locker(&m_mutex);
    return m_slots.size();
}

void EffectChain::addEffectSlot() {
    qDebug() << debugString() << "addEffectSlot";
    QMutexLocker locker(&m_mutex);
    EffectSlot* pEffectSlot = new EffectSlot(this, m_iChainNumber, m_slots.size());
    m_slots.append(EffectSlotPointer(pEffectSlot, &QObject::deleteLater));
}

void EffectChain::loadEffectToSlot(EffectPointer pEffect, unsigned int slotNumber) {
    qDebug() << debugString() << "loadEffectToSlot" << (pEffect ? pEffect->getManifest().name() : "(null)")
             << slotNumber;
    QMutexLocker locker(&m_mutex);

    // TODO(rryan) implement
    qDebug() << "loadEffectToSlot not implemented";

    // Always unlock before signalling to prevent deadlock.
    locker.unlock();
    emit(effectLoaded(pEffect, slotNumber));
}
