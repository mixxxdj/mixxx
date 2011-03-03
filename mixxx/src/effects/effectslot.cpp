#include <QMutexLocker>

#include "effects/effectslot.h"

EffectSlot::EffectSlot(QObject* pParent, const unsigned int iChainNumber, const unsigned int iSlotNumber)
        : QObject(pParent),
          m_mutex(QMutex::Recursive),
          m_iChainNumber(iChainNumber),
          m_iSlotNumber(iSlotNumber),
          m_group(QString("[EffectChain%1_EffectSlot%2]").arg(m_iChainNumber).arg(m_iSlotNumber)) {
    m_pControlEnabled = new ControlObject(ConfigKey(m_group, "enabled"));
    m_pControlNumParameters = new ControlObject(ConfigKey(m_group, "num_parameters"));

    clear();
}

EffectSlot::~EffectSlot() {

}

void EffectSlot::loadEffect(EffectPointer pEffect) {
    qDebug() << debugString() << "loadEffect" << (pEffect ? pEffect->getManifest().name() : "(null)");
    QMutexLocker locker(&m_mutex);
    if (pEffect) {
        m_pControlEnabled->set(1.0f);
        m_pControlNumParameters->set(0.0f); // TODO(rryan) implement parameters
        // Always unlock before signalling to prevent deadlock
        locker.unlock();
        emit(effectLoaded(m_pEffect));
    } else {
        clear();
    }
}

void EffectSlot::clear() {
    QMutexLocker locker(&m_mutex);
    EffectPointer pEffect = m_pEffect;
    m_pEffect.clear();
    m_pControlEnabled->set(0.0f);
    m_pControlNumParameters->set(0.0f);
    locker.unlock();
    emit(effectUnloaded(pEffect));
}
