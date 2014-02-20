#include "effects/effectslot.h"

// The maximum number of effect parameters we're going to support.
const unsigned int kDefaultMaxParameters = 32;

EffectSlot::EffectSlot(QObject* pParent, const unsigned int iRackNumber,
                       const unsigned int iChainNumber,
                       const unsigned int iEffectnumber)
        : QObject(),
          m_iRackNumber(iRackNumber),
          m_iChainNumber(iChainNumber),
          m_iEffectNumber(iEffectnumber),
          // The control group names are 1-indexed while internally everything
          // is 0-indexed.
          m_group(formatGroupString(m_iRackNumber, m_iChainNumber,
                                    m_iEffectNumber)) {
    m_pControlEnabled = new ControlObject(ConfigKey(m_group, "enabled"));
    m_pControlNumParameters = new ControlObject(ConfigKey(m_group, "num_parameters"));

    for (unsigned int i = 0; i < kDefaultMaxParameters; ++i) {
        addEffectParameterSlot();
    }

    clear();
}

EffectSlot::~EffectSlot() {
    qDebug() << debugString() << "destroyed";
    clear();

    delete m_pControlEnabled;
    delete m_pControlNumParameters;
}

void EffectSlot::addEffectParameterSlot() {
    EffectParameterSlot* pParameter = new EffectParameterSlot(
        this, m_iRackNumber, m_iChainNumber, m_iEffectNumber,
        m_parameters.size());
    m_parameters.append(EffectParameterSlotPointer(pParameter));
}

EffectPointer EffectSlot::getEffect() const {
    return m_pEffect;
}

unsigned int EffectSlot::numParameterSlots() const {
    return m_parameters.size();
}

EffectParameterSlotPointer EffectSlot::getEffectParameterSlot(unsigned int slotNumber) {
    qDebug() << debugString() << "getEffectParameterSlot" << slotNumber;
    if (slotNumber >= m_parameters.size()) {
        qDebug() << "WARNING: slotNumber out of range";
        return EffectParameterSlotPointer();
    }
    return m_parameters[slotNumber];
}

void EffectSlot::loadEffect(EffectPointer pEffect) {
    qDebug() << debugString() << "loadEffect"
             << (pEffect ? pEffect->getManifest().name() : "(null)");
    if (pEffect) {
        m_pEffect = pEffect;
        m_pControlEnabled->set(1.0f);
        m_pControlNumParameters->set(m_pEffect->numParameters());

        while (m_parameters.size() < m_pEffect->numParameters()) {
            addEffectParameterSlot();
        }

        foreach (EffectParameterSlotPointer pParameter, m_parameters) {
            pParameter->loadEffect(m_pEffect);
        }

        emit(effectLoaded(m_pEffect, m_iEffectNumber));
    } else {
        clear();
        // Broadcasts a null effect pointer
        emit(effectLoaded(m_pEffect, m_iEffectNumber));
    }
    emit(updated());
}

void EffectSlot::clear() {
    m_pEffect.clear();
    m_pControlEnabled->set(0.0f);
    m_pControlNumParameters->set(0.0f);
    foreach (EffectParameterSlotPointer pParameter, m_parameters) {
        pParameter->loadEffect(EffectPointer());
    }
    emit(updated());
}
