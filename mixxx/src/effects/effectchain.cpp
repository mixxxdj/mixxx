#include <QMutexLocker>

#include "effects/effectchain.h"

EffectChain::EffectChain(QObject* pParent, unsigned int iChainNumber)
        : QObject(pParent),
          m_mutex(QMutex::Recursive),
          m_iChainNumber(iChainNumber),
          // The control group names are 1-indexed while internally everything is 0-indexed.
          m_group(QString("[EffectChain%1]").arg(iChainNumber+1)) {
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
    // Rebroadcast effectLoaded signals
    connect(pEffectSlot, SIGNAL(effectLoaded(EffectPointer, unsigned int)),
            this, SLOT(slotEffectLoaded(EffectPointer, unsigned int)));

    m_slots.append(EffectSlotPointer(pEffectSlot, &QObject::deleteLater));
}

void EffectChain::slotEffectLoaded(EffectPointer pEffect, unsigned int slotNumber) {
    // const int is a safe read... don't bother locking
    emit(effectLoaded(pEffect, m_iChainNumber, slotNumber));
}

EffectSlotPointer EffectChain::getEffectSlot(unsigned int slotNumber) {
    qDebug() << debugString() << "getEffectSlot" << slotNumber;
    QMutexLocker locker(&m_mutex);
    if (slotNumber >= m_slots.size()) {
        qDebug() << "WARNING: slotNumber out of range";
        return EffectSlotPointer();
    }
    return m_slots[slotNumber];
}
