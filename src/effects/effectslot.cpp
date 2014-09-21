#include "effects/effectslot.h"

#include "controlpushbutton.h"
#include "controlobjectslave.h"

// The maximum number of effect parameters we're going to support.
const unsigned int kDefaultMaxParameters = 8;

EffectSlot::EffectSlot(const unsigned int iRackNumber,
                       const unsigned int iChainNumber,
                       const unsigned int iEffectnumber)
        : m_iRackNumber(iRackNumber),
          m_iChainNumber(iChainNumber),
          m_iEffectNumber(iEffectnumber),
          // The control group names are 1-indexed while internally everything
          // is 0-indexed.
          m_group(formatGroupString(m_iRackNumber, m_iChainNumber,
                                    m_iEffectNumber)) {
    m_pControlLoaded = new ControlObject(ConfigKey(m_group, "loaded"));
    m_pControlLoaded->connectValueChangeRequest(
        this, SLOT(slotLoaded(double)));

    m_pControlNumParameters = new ControlObject(ConfigKey(m_group, "num_parameters"));
    m_pControlNumParameters->connectValueChangeRequest(
        this, SLOT(slotNumParameters(double)));

    m_pControlNumParameterSlots = new ControlObject(ConfigKey(m_group, "num_parameterslots"));
    m_pControlNumParameterSlots->connectValueChangeRequest(
        this, SLOT(slotNumParameterSlots(double)));

    m_pControlNumButtonParameters = new ControlObject(ConfigKey(m_group, "num_button_parameters"));
    m_pControlNumButtonParameters->connectValueChangeRequest(
        this, SLOT(slotNumParameters(double)));

    m_pControlNumButtonParameterSlots = new ControlObject(ConfigKey(m_group, "num_button_parameterslots"));
    m_pControlNumButtonParameterSlots->connectValueChangeRequest(
        this, SLOT(slotNumParameterSlots(double)));

    m_pControlEnabled = new ControlPushButton(ConfigKey(m_group, "enabled"));
    m_pControlEnabled->setButtonMode(ControlPushButton::POWERWINDOW);
    // Default to enabled. The skin might not show these buttons.
    m_pControlEnabled->setDefaultValue(true);
    m_pControlEnabled->set(true);
    connect(m_pControlEnabled, SIGNAL(valueChanged(double)),
            this, SLOT(slotEnabled(double)));

    m_pControlNextEffect = new ControlPushButton(ConfigKey(m_group, "next_effect"));
    connect(m_pControlNextEffect, SIGNAL(valueChanged(double)),
            this, SLOT(slotNextEffect(double)));

    m_pControlPrevEffect = new ControlPushButton(ConfigKey(m_group, "prev_effect"));
    connect(m_pControlPrevEffect, SIGNAL(valueChanged(double)),
            this, SLOT(slotPrevEffect(double)));

    // Ignoring no-ops is important since this is for +/- tickers.
    m_pControlEffectSelector = new ControlObject(ConfigKey(m_group, "effect_selector"), false);
    connect(m_pControlEffectSelector, SIGNAL(valueChanged(double)),
            this, SLOT(slotEffectSelector(double)));

    m_pControlClear = new ControlPushButton(ConfigKey(m_group, "clear"));
    connect(m_pControlClear, SIGNAL(valueChanged(double)),
            this, SLOT(slotClear(double)));

    for (unsigned int i = 0; i < kDefaultMaxParameters; ++i) {
        addEffectParameterSlot();
        addEffectButtonParameterSlot();
    }

    QString effectUnitGroup =  QString("[EffectRack%1_EffectUnit%2]").arg(
        QString::number(iRackNumber+1), QString::number(iChainNumber+1));

    m_pCoSuper = new ControlObjectSlave(ConfigKey(effectUnitGroup, "parameter"));

    clear();
}

EffectSlot::~EffectSlot() {
    //qDebug() << debugString() << "destroyed";
    clear();

    delete m_pControlLoaded;
    delete m_pControlNumParameters;
    delete m_pControlNumParameterSlots;
    delete m_pControlNumButtonParameters;
    delete m_pControlNumButtonParameterSlots;
    delete m_pControlNextEffect;
    delete m_pControlPrevEffect;
    delete m_pControlEffectSelector;
    delete m_pControlClear;
    delete m_pControlEnabled;
    delete m_pCoSuper;
}

EffectParameterSlotPointer EffectSlot::addEffectParameterSlot() {
    EffectParameterSlotPointer pParameter = EffectParameterSlotPointer(
        new EffectParameterSlot(m_iRackNumber, m_iChainNumber, m_iEffectNumber,
                                m_parameters.size()));
    m_parameters.append(pParameter);
    m_pControlNumParameterSlots->setAndConfirm(
            m_pControlNumParameterSlots->get() + 1);
    return pParameter;
}

EffectButtonParameterSlotPointer EffectSlot::addEffectButtonParameterSlot() {
    EffectButtonParameterSlotPointer pParameter = EffectButtonParameterSlotPointer(
        new EffectButtonParameterSlot(m_iRackNumber, m_iChainNumber, m_iEffectNumber,
                                m_buttonParameters.size()));
    m_buttonParameters.append(pParameter);
    m_pControlNumButtonParameterSlots->setAndConfirm(
            m_pControlNumButtonParameterSlots->get() + 1);
    return pParameter;
}

EffectPointer EffectSlot::getEffect() const {
    return m_pEffect;
}

unsigned int EffectSlot::numParameterSlots() const {
    return m_parameters.size();
}

unsigned int EffectSlot::numButtonParameterSlots() const {
    return m_buttonParameters.size();
}

void EffectSlot::slotLoaded(double v) {
    Q_UNUSED(v);
    //qDebug() << debugString() << "slotLoaded" << v;
    qWarning() << "WARNING: loaded is a read-only control.";
}

void EffectSlot::slotNumParameters(double v) {
    Q_UNUSED(v);
    //qDebug() << debugString() << "slotNumParameters" << v;
    qWarning() << "WARNING: num_parameters is a read-only control.";
}

void EffectSlot::slotNumParameterSlots(double v) {
    Q_UNUSED(v);
    //qDebug() << debugString() << "slotNumParameterSlots" << v;
    qWarning() << "WARNING: num_parameterslots is a read-only control.";
}

void EffectSlot::slotEnabled(double v) {
    //qDebug() << debugString() << "slotEnabled" << v;
    if (m_pEffect) {
        m_pEffect->setEnabled(v > 0);
    }
}

void EffectSlot::slotEffectEnabledChanged(bool enabled) {
    m_pControlEnabled->set(enabled);
}

EffectParameterSlotPointer EffectSlot::getEffectParameterSlot(unsigned int slotNumber) {
    //qDebug() << debugString() << "getEffectParameterSlot" << slotNumber;
    if (slotNumber >= static_cast<unsigned int>(m_parameters.size())) {
        qWarning() << "WARNING: slotNumber out of range";
        return EffectParameterSlotPointer();
    }
    return m_parameters[slotNumber];
}

EffectButtonParameterSlotPointer EffectSlot::getEffectButtonParameterSlot(unsigned int slotNumber) {
    //qDebug() << debugString() << "getEffectParameterSlot" << slotNumber;
    if (slotNumber >= static_cast<unsigned int>(m_buttonParameters.size())) {
        qWarning() << "WARNING: slotNumber out of range";
        return EffectButtonParameterSlotPointer();
    }
    return m_buttonParameters[slotNumber];
}

void EffectSlot::loadEffect(EffectPointer pEffect) {
    // qDebug() << debugString() << "loadEffect"
    //          << (pEffect ? pEffect->getManifest().name() : "(null)");
    if (pEffect) {
        m_pEffect = pEffect;
        m_pControlLoaded->setAndConfirm(1.0);
        m_pControlNumParameters->setAndConfirm(m_pEffect->numParameters());
        m_pControlNumButtonParameters->setAndConfirm(m_pEffect->numButtonParameters());

        // Enabled is a persistent property of the effect slot, not of the
        // effect. Propagate the current setting to the effect.
        m_pEffect->setEnabled(m_pControlEnabled->get() > 0.0);

        connect(m_pEffect.data(), SIGNAL(enabledChanged(bool)),
                this, SLOT(slotEffectEnabledChanged(bool)));

        while (static_cast<unsigned int>(m_parameters.size()) < m_pEffect->numParameters()) {
            addEffectParameterSlot();
        }

        while (static_cast<unsigned int>(m_buttonParameters.size()) < m_pEffect->numButtonParameters()) {
            addEffectButtonParameterSlot();
        }

        foreach (EffectParameterSlotPointer pParameter, m_parameters) {
            pParameter->loadEffect(m_pEffect);
        }

        foreach (EffectButtonParameterSlotPointer pParameter, m_buttonParameters) {
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
    if (m_pEffect) {
        m_pEffect->disconnect(this);
        m_pEffect.clear();
    }
    m_pControlLoaded->setAndConfirm(0.0);
    m_pControlNumParameters->setAndConfirm(0.0);
    m_pControlNumButtonParameters->setAndConfirm(0.0);
    foreach (EffectParameterSlotPointer pParameter, m_parameters) {
        pParameter->loadEffect(EffectPointer());
    }
    foreach (EffectButtonParameterSlotPointer pParameter, m_buttonParameters) {
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

void EffectSlot::onChainParameterChanged(double parameter) {
    for (int i = 0; i < m_parameters.size(); ++i) {
        m_parameters[i]->onChainParameterChanged(parameter);
    }
}

void EffectSlot::syncSofttakeover() {
    for (int i = 0; i < m_parameters.size(); ++i) {
        m_parameters[i]->syncSofttakeover();
    }
}
