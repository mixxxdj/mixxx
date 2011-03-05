#include <QMutexLocker>

#include "effects/effectslot.h"

// The maximum number of effect parameters we're going to support.
const unsigned int kMaxParameters = 20;

EffectSlot::EffectSlot(QObject* pParent, const unsigned int iChainNumber, const unsigned int iSlotNumber)
        : QObject(pParent),
          m_mutex(QMutex::Recursive),
          m_iChainNumber(iChainNumber),
          m_iSlotNumber(iSlotNumber),
          // The control group names are 1-indexed while internally everything is 0-indexed.
          m_group(QString("[EffectChain%1_Effect%2]").arg(m_iChainNumber+1).arg(m_iSlotNumber+1)) {
    m_pControlEnabled = new ControlObject(ConfigKey(m_group, "enabled"));
    m_pControlNumParameters = new ControlObject(ConfigKey(m_group, "num_parameters"));

    for (unsigned int i = 0; i < kMaxParameters; ++i) {
        EffectSlotParameter* pParameter = new EffectSlotParameter(this, m_group, m_parameters.size());
        m_parameters.append(pParameter);
    }

    clear();
}

EffectSlot::~EffectSlot() {
    qDebug() << debugString() << "destroyed";
    clear();

    delete m_pControlEnabled;
    delete m_pControlNumParameters;

    while (!m_parameters.isEmpty()) {
        EffectSlotParameter* pParameter = m_parameters.takeLast();
        delete pParameter;
    }
}

EffectPointer EffectSlot::getEffect() const {
    QMutexLocker locker(&m_mutex);
    return m_pEffect;
}

void EffectSlot::loadEffect(EffectPointer pEffect) {
    qDebug() << debugString() << "loadEffect" << (pEffect ? pEffect->getManifest()->name() : "(null)");
    QMutexLocker locker(&m_mutex);
    if (pEffect) {
        m_pEffect = pEffect;
        m_pControlEnabled->set(1.0f);
        m_pControlNumParameters->set(m_pEffect->getManifest()->parameters().size());

        foreach (EffectSlotParameter* pParameter, m_parameters) {
            pParameter->loadEffect(m_pEffect);
        }

        // Always unlock before signalling to prevent deadlock
        locker.unlock();
        emit(effectLoaded(m_pEffect, m_iSlotNumber));
    } else {
        clear();
        locker.unlock();
        // Broadcasts a null effect pointer
        emit(effectLoaded(m_pEffect, m_iSlotNumber));
    }
}

void EffectSlot::clear() {
    m_pEffect.clear();
    m_pControlEnabled->set(0.0f);
    m_pControlNumParameters->set(0.0f);
    foreach (EffectSlotParameter* pParameter, m_parameters) {
        pParameter->loadEffect(EffectPointer());
    }
}
