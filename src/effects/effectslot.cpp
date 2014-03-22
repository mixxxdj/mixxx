#include "effects/effectslot.h"

#include "controlpushbutton.h"

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
    m_pControlLoaded = new ControlObject(ConfigKey(m_group, "loaded"));
    m_pControlLoaded->connectValueChangeRequest(
        this, SLOT(slotLoaded(double)), Qt::AutoConnection);
    m_pControlNumParameters = new ControlObject(ConfigKey(m_group, "num_parameters"));
    m_pControlNumParameters->connectValueChangeRequest(
        this, SLOT(slotNumParameters(double)), Qt::AutoConnection);

    m_pControlNextEffect = new ControlPushButton(ConfigKey(m_group, "next_effect"));
    connect(m_pControlNextEffect, SIGNAL(valueChanged(double)),
            this, SLOT(slotNextEffect(double)));

    m_pControlPrevEffect = new ControlPushButton(ConfigKey(m_group, "prev_effect"));
    connect(m_pControlPrevEffect, SIGNAL(valueChanged(double)),
            this, SLOT(slotPrevEffect(double)));

    m_pControlEffectSelector = new ControlObject(ConfigKey(m_group, "effect_selector"));
    connect(m_pControlEffectSelector, SIGNAL(valueChanged(double)),
            this, SLOT(slotEffectSelector(double)));

    m_pControlClear = new ControlPushButton(ConfigKey(m_group, "clear"));
    connect(m_pControlClear, SIGNAL(valueChanged(double)),
            this, SLOT(slotClear(double)));

    for (unsigned int i = 0; i < kDefaultMaxParameters; ++i) {
        addEffectParameterSlot();
    }

    clear();
}

EffectSlot::~EffectSlot() {
    qDebug() << debugString() << "destroyed";
    clear();

    delete m_pControlLoaded;
    delete m_pControlNumParameters;
}

EffectParameterSlotPointer EffectSlot::addEffectParameterSlot() {
    EffectParameterSlotPointer pParameter = EffectParameterSlotPointer(
        new EffectParameterSlot(this, m_iRackNumber, m_iChainNumber, m_iEffectNumber,
                                m_parameters.size()));
    m_parameters.append(pParameter);
    return pParameter;
}

EffectPointer EffectSlot::getEffect() const {
    return m_pEffect;
}

unsigned int EffectSlot::numParameterSlots() const {
    return m_parameters.size();
}

void EffectSlot::slotLoaded(double v) {
    qDebug() << debugString() << "slotLoaded" << v;
    qDebug() << "WARNING: loaded is a read-only control.";
}

void EffectSlot::slotNumParameters(double v) {
    qDebug() << debugString() << "slotNumParameters" << v;
    qDebug() << "WARNING: num_parameters is a read-only control.";
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
        m_pControlLoaded->setAndConfirm(1.0);
        m_pControlNumParameters->setAndConfirm(m_pEffect->numParameters());

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
    m_pControlLoaded->setAndConfirm(0.0);
    m_pControlNumParameters->setAndConfirm(0.0);
    foreach (EffectParameterSlotPointer pParameter, m_parameters) {
        pParameter->loadEffect(EffectPointer());
    }
    emit(updated());
}

void EffectSlot::slotPrevEffect(double v) {
    if (v > 0) {
        slotEffectSelector(-1);
    }
}

void EffectSlot::slotNextEffect(double v) {
    if (v > 0) {
        slotEffectSelector(1);
    }
}

void EffectSlot::slotEffectSelector(double v) {
    if (v > 0) {
        emit(nextEffect(m_iChainNumber, m_iEffectNumber, m_pEffect));
    } else if (v < 0) {
        emit(prevEffect(m_iChainNumber, m_iEffectNumber, m_pEffect));
    }
}

void EffectSlot::slotClear(double v) {
    if (v > 0) {
        emit(clearEffect(m_iChainNumber, m_iEffectNumber, m_pEffect));
    }
}
