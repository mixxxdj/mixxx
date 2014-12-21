#include "effects/effectchainslot.h"

#include "effects/effectrack.h"
#include "sampleutil.h"
#include "controlpotmeter.h"
#include "controlpushbutton.h"
#include "util/math.h"

EffectChainSlot::EffectChainSlot(EffectRack* pRack, const QString& group,
                                 unsigned int iChainNumber)
        : m_iChainSlotNumber(iChainNumber),
          // The control group names are 1-indexed while internally everything
          // is 0-indexed.
          m_group(group),
          m_pEffectRack(pRack) {
    m_pControlClear = new ControlPushButton(ConfigKey(m_group, "clear"));
    connect(m_pControlClear, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlClear(double)));

    m_pControlNumEffects = new ControlObject(ConfigKey(m_group, "num_effects"));
    m_pControlNumEffects->connectValueChangeRequest(
        this, SLOT(slotControlNumEffects(double)));

    m_pControlNumEffectSlots = new ControlObject(ConfigKey(m_group, "num_effectslots"));
    m_pControlNumEffectSlots->connectValueChangeRequest(
        this, SLOT(slotControlNumEffectSlots(double)));

    m_pControlChainLoaded = new ControlObject(ConfigKey(m_group, "loaded"));
    m_pControlChainLoaded->connectValueChangeRequest(
        this, SLOT(slotControlChainLoaded(double)));

    m_pControlChainEnabled = new ControlPushButton(ConfigKey(m_group, "enabled"));
    m_pControlChainEnabled->setButtonMode(ControlPushButton::POWERWINDOW);
    // Default to enabled. The skin might not show these buttons.
    m_pControlChainEnabled->setDefaultValue(true);
    m_pControlChainEnabled->set(true);
    connect(m_pControlChainEnabled, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlChainEnabled(double)));

    m_pControlChainMix = new ControlPotmeter(ConfigKey(m_group, "mix"), 0.0, 1.0);
    connect(m_pControlChainMix, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlChainMix(double)));
    m_pControlChainMix->set(1.0);

    m_pControlChainSuperParameter = new ControlPotmeter(ConfigKey(m_group, "super1"), 0.0, 1.0);
    connect(m_pControlChainSuperParameter, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlChainSuperParameter(double)));
    m_pControlChainSuperParameter->set(0.0);
    m_pControlChainSuperParameter->setDefaultValue(0.0);

    m_pControlChainInsertionType = new ControlPushButton(ConfigKey(m_group, "insertion_type"));
    m_pControlChainInsertionType->setButtonMode(ControlPushButton::TOGGLE);
    m_pControlChainInsertionType->setStates(EffectChain::NUM_INSERTION_TYPES);
    connect(m_pControlChainInsertionType, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlChainInsertionType(double)));

    m_pControlChainNextPreset = new ControlPushButton(ConfigKey(m_group, "next_chain"));
    connect(m_pControlChainNextPreset, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlChainNextPreset(double)));

    m_pControlChainPrevPreset = new ControlPushButton(ConfigKey(m_group, "prev_chain"));
    connect(m_pControlChainPrevPreset, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlChainPrevPreset(double)));

    // Ignoring no-ops is important since this is for +/- tickers.
    m_pControlChainSelector = new ControlObject(ConfigKey(m_group, "chain_selector"), false);
    connect(m_pControlChainSelector, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlChainSelector(double)));

    connect(&m_groupStatusMapper, SIGNAL(mapped(const QString&)),
            this, SLOT(slotGroupStatusChanged(const QString&)));
}

EffectChainSlot::~EffectChainSlot() {
    //qDebug() << debugString() << "destroyed";
    clear();
    delete m_pControlClear;
    delete m_pControlNumEffects;
    delete m_pControlNumEffectSlots;
    delete m_pControlChainLoaded;
    delete m_pControlChainEnabled;
    delete m_pControlChainMix;
    delete m_pControlChainSuperParameter;
    delete m_pControlChainInsertionType;
    delete m_pControlChainPrevPreset;
    delete m_pControlChainNextPreset;
    delete m_pControlChainSelector;

    for (QMap<QString, ControlObject*>::iterator it = m_groupEnableControls.begin();
         it != m_groupEnableControls.end();) {
        delete it.value();
        it = m_groupEnableControls.erase(it);
    }

    m_slots.clear();
    m_pEffectChain.clear();
}

QString EffectChainSlot::id() const {
    if (m_pEffectChain)
        return m_pEffectChain->id();
    return "";
}

double EffectChainSlot::getSuperParameter() const {
    return m_pControlChainSuperParameter->get();
}

void EffectChainSlot::setSuperParameter(double value) {
    m_pControlChainSuperParameter->set(value);
}

void EffectChainSlot::setSuperParameterDefaultValue(double value) {
    m_pControlChainSuperParameter->setDefaultValue(value);
}

void EffectChainSlot::slotChainNameChanged(const QString&) {
    emit(updated());
}

void EffectChainSlot::slotChainEnabledChanged(bool bEnabled) {
    m_pControlChainEnabled->set(bEnabled);
    emit(updated());
}

void EffectChainSlot::slotChainMixChanged(double mix) {
    m_pControlChainMix->set(mix);
    emit(updated());
}

void EffectChainSlot::slotChainSuperParameterChanged(double parameter) {
    m_pControlChainSuperParameter->set(parameter);
    emit(updated());
}

void EffectChainSlot::slotChainInsertionTypeChanged(EffectChain::InsertionType type) {
    m_pControlChainInsertionType->set(static_cast<double>(type));
    emit(updated());
}

void EffectChainSlot::slotChainGroupStatusChanged(const QString& group,
                                                  bool enabled) {
    ControlObject* pGroupControl = m_groupEnableControls.value(group, NULL);
    if (pGroupControl != NULL) {
        pGroupControl->set(enabled);
        emit(updated());
    }
}

void EffectChainSlot::slotChainEffectsChanged(bool shouldEmit) {
    //qDebug() << debugString() << "slotChainEffectsChanged";
    if (m_pEffectChain) {
        QList<EffectPointer> effects = m_pEffectChain->effects();
        if (effects.size() > m_slots.size()) {
            qWarning() << debugString() << "has too few slots for effect";
        }
        for (int i = 0; i < m_slots.size(); ++i) {
            EffectSlotPointer pSlot = m_slots[i];
            EffectPointer pEffect;
            if (i < effects.size()) {
                pEffect = effects[i];
            }
            if (pSlot)
                pSlot->loadEffect(pEffect);
        }
        m_pControlNumEffects->setAndConfirm(math_min(
            static_cast<unsigned int>(m_slots.size()),
            m_pEffectChain->numEffects()));
        if (shouldEmit) {
            emit(updated());
        }
    }
}

void EffectChainSlot::loadEffectChain(EffectChainPointer pEffectChain) {
    //qDebug() << debugString() << "loadEffectChain" << (pEffectChain ? pEffectChain->id() : "(null)");
    clear();

    if (pEffectChain) {
        m_pEffectChain = pEffectChain;
        m_pEffectChain->addToEngine(m_pEffectRack->getEngineEffectRack(),
                                    m_iChainSlotNumber);
        m_pEffectChain->updateEngineState();

        connect(m_pEffectChain.data(), SIGNAL(effectsChanged()),
                this, SLOT(slotChainEffectsChanged()));
        connect(m_pEffectChain.data(), SIGNAL(nameChanged(const QString&)),
                this, SLOT(slotChainNameChanged(const QString&)));
        connect(m_pEffectChain.data(), SIGNAL(enabledChanged(bool)),
                this, SLOT(slotChainEnabledChanged(bool)));
        connect(m_pEffectChain.data(), SIGNAL(mixChanged(double)),
                this, SLOT(slotChainMixChanged(double)));
        connect(m_pEffectChain.data(), SIGNAL(insertionTypeChanged(EffectChain::InsertionType)),
                this, SLOT(slotChainInsertionTypeChanged(EffectChain::InsertionType)));
        connect(m_pEffectChain.data(), SIGNAL(groupStatusChanged(const QString&, bool)),
                this, SLOT(slotChainGroupStatusChanged(const QString&, bool)));

        m_pControlChainLoaded->setAndConfirm(true);
        m_pControlChainInsertionType->set(m_pEffectChain->insertionType());

        // Mix and enabled channels are persistent properties of the chain slot,
        // not of the chain. Propagate the current settings to the chain.
        m_pEffectChain->setMix(m_pControlChainMix->get());
        m_pEffectChain->setEnabled(m_pControlChainEnabled->get() > 0.0);
        for (QMap<QString, ControlObject*>::iterator it = m_groupEnableControls.begin();
             it != m_groupEnableControls.end(); ++it) {
            if (it.value()->get() > 0.0) {
                m_pEffectChain->enableForGroup(it.key());
            } else {
                m_pEffectChain->disableForGroup(it.key());
            }
        }

        // Don't emit because we will below.
        slotChainEffectsChanged(false);
    }

    emit(effectChainLoaded(pEffectChain));
    emit(updated());
}

EffectChainPointer EffectChainSlot::getEffectChain() const {
    return m_pEffectChain;
}

void EffectChainSlot::clear() {
    // Stop listening to signals from any loaded effect
    if (m_pEffectChain) {
        m_pEffectChain->removeFromEngine(m_pEffectRack->getEngineEffectRack(),
                                         m_iChainSlotNumber);
        foreach (EffectSlotPointer pSlot, m_slots) {
            pSlot->clear();
        }
        m_pEffectChain->disconnect(this);
        m_pEffectChain.clear();
    }
    m_pControlNumEffects->setAndConfirm(0.0);
    m_pControlChainLoaded->setAndConfirm(0.0);
    m_pControlChainInsertionType->set(EffectChain::INSERT);
    emit(updated());
}

unsigned int EffectChainSlot::numSlots() const {
    //qDebug() << debugString() << "numSlots";
    return m_slots.size();
}

EffectSlotPointer EffectChainSlot::addEffectSlot(const QString& group) {
    //qDebug() << debugString() << "addEffectSlot" << group;

    EffectSlot* pEffectSlot = new EffectSlot(group, m_iChainSlotNumber,
                                             m_slots.size());
    // Rebroadcast effectLoaded signals
    connect(pEffectSlot, SIGNAL(effectLoaded(EffectPointer, unsigned int)),
            this, SLOT(slotEffectLoaded(EffectPointer, unsigned int)));
    connect(pEffectSlot, SIGNAL(clearEffect(unsigned int)),
            this, SLOT(slotClearEffect(unsigned int)));
    connect(pEffectSlot, SIGNAL(nextEffect(unsigned int, unsigned int, EffectPointer)),
            this, SIGNAL(nextEffect(unsigned int, unsigned int, EffectPointer)));
    connect(pEffectSlot, SIGNAL(prevEffect(unsigned int, unsigned int, EffectPointer)),
            this, SIGNAL(prevEffect(unsigned int, unsigned int, EffectPointer)));

    EffectSlotPointer pSlot(pEffectSlot);
    m_slots.append(pSlot);
    m_pControlNumEffectSlots->setAndConfirm(m_pControlNumEffectSlots->get() + 1);
    return pSlot;
}

void EffectChainSlot::registerGroup(const QString& group) {
    if (m_groupEnableControls.contains(group)) {
        qWarning() << debugString()
                   << "WARNING: registerGroup already has group registered:"
                   << group;
        return;
    }
    ControlPushButton* pEnableControl = new ControlPushButton(
        ConfigKey(m_group, QString("group_%1_enable").arg(group)));
    pEnableControl->setButtonMode(ControlPushButton::POWERWINDOW);
    m_groupEnableControls[group] = pEnableControl;
    m_groupStatusMapper.setMapping(pEnableControl, group);
    connect(pEnableControl, SIGNAL(valueChanged(double)),
            &m_groupStatusMapper, SLOT(map()));
}

void EffectChainSlot::slotEffectLoaded(EffectPointer pEffect, unsigned int slotNumber) {
    // const int is a safe read... don't bother locking
    emit(effectLoaded(pEffect, m_iChainSlotNumber, slotNumber));
}

void EffectChainSlot::slotClearEffect(unsigned int iEffectSlotNumber) {
    if (m_pEffectChain) {
        m_pEffectChain->removeEffect(iEffectSlotNumber);
    }
}

EffectSlotPointer EffectChainSlot::getEffectSlot(unsigned int slotNumber) {
    //qDebug() << debugString() << "getEffectSlot" << slotNumber;
    if (slotNumber >= static_cast<unsigned int>(m_slots.size())) {
        qWarning() << "WARNING: slotNumber out of range";
        return EffectSlotPointer();
    }
    return m_slots[slotNumber];
}

void EffectChainSlot::slotControlClear(double v) {
    if (v > 0) {
        clear();
    }
}

void EffectChainSlot::slotControlNumEffects(double v) {
    // Ignore sets to num_effects.
    Q_UNUSED(v);
    //qDebug() << debugString() << "slotControlNumEffects" << v;
    qWarning() << "WARNING: num_effects is a read-only control.";
}

void EffectChainSlot::slotControlNumEffectSlots(double v) {
    // Ignore sets to num_effectslots.
    Q_UNUSED(v);
    //qDebug() << debugString() << "slotControlNumEffectSlots" << v;
    qWarning() << "WARNING: num_effectslots is a read-only control.";
}

void EffectChainSlot::slotControlChainLoaded(double v) {
    // Ignore sets to loaded.
    Q_UNUSED(v);
    //qDebug() << debugString() << "slotControlChainLoaded" << v;
    qWarning() << "WARNING: loaded is a read-only control.";
}

void EffectChainSlot::slotControlChainEnabled(double v) {
    //qDebug() << debugString() << "slotControlChainEnabled" << v;
    if (m_pEffectChain) {
        m_pEffectChain->setEnabled(v > 0);
    }
}

void EffectChainSlot::slotControlChainMix(double v) {
    //qDebug() << debugString() << "slotControlChainMix" << v;

    // Clamp to [0.0, 1.0]
    if (v < 0.0 || v > 1.0) {
        qWarning() << debugString() << "value out of limits";
        v = math_clamp(v, 0.0, 1.0);
        m_pControlChainMix->set(v);
    }
    if (m_pEffectChain) {
        m_pEffectChain->setMix(v);
    }
}

void EffectChainSlot::slotControlChainSuperParameter(double v) {
    //qDebug() << debugString() << "slotControlChainSuperParameter" << v;

    // Clamp to [0.0, 1.0]
    if (v < 0.0 || v > 1.0) {
        qWarning() << debugString() << "value out of limits";
        v = math_clamp(v, 0.0, 1.0);
        m_pControlChainSuperParameter->set(v);
    }
    for (int i = 0; i < m_slots.size(); ++i) {
        m_slots[i]->onChainSuperParameterChanged(v);
    }
}

void EffectChainSlot::slotControlChainInsertionType(double v) {
    // Intermediate cast to integer is needed for VC++.
    EffectChain::InsertionType type = static_cast<EffectChain::InsertionType>(int(v));
    (void)v; // this avoids a false warning with g++ 4.8.1
    if (m_pEffectChain && type >= 0 &&
            type < EffectChain::NUM_INSERTION_TYPES) {
        m_pEffectChain->setInsertionType(type);
    }
}

void EffectChainSlot::slotControlChainSelector(double v) {
    //qDebug() << debugString() << "slotControlChainSelector" << v;
    if (v > 0) {
        emit(nextChain(m_iChainSlotNumber, m_pEffectChain));
    } else if (v < 0) {
        emit(prevChain(m_iChainSlotNumber, m_pEffectChain));
    }
}

void EffectChainSlot::slotControlChainNextPreset(double v) {
    //qDebug() << debugString() << "slotControlChainNextPreset" << v;
    if (v > 0) {
        slotControlChainSelector(1);
    }
}

void EffectChainSlot::slotControlChainPrevPreset(double v) {
    //qDebug() << debugString() << "slotControlChainPrevPreset" << v;
    if (v > 0) {
        slotControlChainSelector(-1);
    }
}

void EffectChainSlot::slotGroupStatusChanged(const QString& group) {
    if (m_pEffectChain) {
        ControlObject* pGroupControl = m_groupEnableControls.value(group, NULL);
        if (pGroupControl != NULL) {
            bool bEnable = pGroupControl->get() > 0;
            if (bEnable) {
                m_pEffectChain->enableForGroup(group);
            } else {
                m_pEffectChain->disableForGroup(group);
            }
        }
    }
}

unsigned int EffectChainSlot::getChainSlotNumber() const {
    return m_iChainSlotNumber;
}
