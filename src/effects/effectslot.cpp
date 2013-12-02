#include "effects/effectslot.h"

// The maximum number of effect parameters we're going to support.
const unsigned int kDefaultMaxParameters = 32;

EffectSlot::EffectSlot(QObject* pParent, const unsigned int iRackNumber,
                       const unsigned int iChainNumber,
                       const unsigned int iSlotNumber)
        : QObject(),
          m_iRackNumber(iRackNumber),
          m_iChainNumber(iChainNumber),
          m_iSlotNumber(iSlotNumber),
          // The control group names are 1-indexed while internally everything
          // is 0-indexed.
          m_group(formatGroupString(m_iRackNumber, m_iChainNumber,
                                    m_iSlotNumber)) {
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

    while (!m_parameters.isEmpty()) {
        EffectParameterSlot* pParameter = m_parameters.takeLast();
        delete pParameter;
    }
}

void EffectSlot::addEffectParameterSlot() {
    EffectParameterSlot* pParameter = new EffectParameterSlot(
        this, m_iRackNumber, m_iChainNumber, m_iSlotNumber,
        m_parameters.size());
    m_parameters.append(pParameter);
}

EffectPointer EffectSlot::getEffect() const {
    return m_pEffect;
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

        foreach (EffectParameterSlot* pParameter, m_parameters) {
            pParameter->loadEffect(m_pEffect);
        }

        emit(effectLoaded(m_pEffect, m_iSlotNumber));
    } else {
        clear();
        // Broadcasts a null effect pointer
        emit(effectLoaded(m_pEffect, m_iSlotNumber));
    }
}

void EffectSlot::clear() {
    m_pEffect.clear();
    m_pControlEnabled->set(0.0f);
    m_pControlNumParameters->set(0.0f);
    foreach (EffectParameterSlot* pParameter, m_parameters) {
        pParameter->loadEffect(EffectPointer());
    }
}
