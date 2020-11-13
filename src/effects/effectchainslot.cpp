#include "effects/effectchainslot.h"

#include "effects/effectrack.h"
#include "effects/effectxmlelements.h"
#include "effects/effectslot.h"
#include "control/controlpotmeter.h"
#include "control/controlpushbutton.h"
#include "control/controlencoder.h"
#include "mixer/playermanager.h"
#include "util/math.h"
#include "util/xml.h"

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
    m_pControlNumEffects->setReadOnly();

    m_pControlNumEffectSlots = new ControlObject(ConfigKey(m_group, "num_effectslots"));
    m_pControlNumEffectSlots->setReadOnly();

    m_pControlChainLoaded = new ControlObject(ConfigKey(m_group, "loaded"));
    m_pControlChainLoaded->setReadOnly();

    m_pControlChainEnabled = new ControlPushButton(ConfigKey(m_group, "enabled"));
    m_pControlChainEnabled->setButtonMode(ControlPushButton::POWERWINDOW);
    // Default to enabled. The skin might not show these buttons.
    m_pControlChainEnabled->setDefaultValue(true);
    m_pControlChainEnabled->set(true);
    connect(m_pControlChainEnabled, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlChainEnabled(double)));

    m_pControlChainMix = new ControlPotmeter(ConfigKey(m_group, "mix"), 0.0, 1.0,
                                             false, true, false, true, 1.0);
    connect(m_pControlChainMix, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlChainMix(double)));

    m_pControlChainSuperParameter = new ControlPotmeter(ConfigKey(m_group, "super1"), 0.0, 1.0);
    connect(m_pControlChainSuperParameter, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlChainSuperParameter(double)));
    m_pControlChainSuperParameter->set(0.0);
    m_pControlChainSuperParameter->setDefaultValue(0.0);

    m_pControlChainMixMode = new ControlPushButton(ConfigKey(m_group, "mix_mode"));
    m_pControlChainMixMode->setButtonMode(ControlPushButton::TOGGLE);
    m_pControlChainMixMode->setStates(static_cast<int>(EffectChainMixMode::NumMixModes));
    connect(m_pControlChainMixMode, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlChainMixMode(double)));

    m_pControlChainNextPreset = new ControlPushButton(ConfigKey(m_group, "next_chain"));
    connect(m_pControlChainNextPreset, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlChainNextPreset(double)));

    m_pControlChainPrevPreset = new ControlPushButton(ConfigKey(m_group, "prev_chain"));
    connect(m_pControlChainPrevPreset, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlChainPrevPreset(double)));

    // Ignoring no-ops is important since this is for +/- tickers.
    m_pControlChainSelector = new ControlEncoder(ConfigKey(m_group, "chain_selector"), false);
    connect(m_pControlChainSelector, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlChainSelector(double)));

    // ControlObjects for skin <-> controller mapping interaction.
    // Refer to comment in header for full explanation.
    m_pControlChainShowFocus = new ControlPushButton(
                                   ConfigKey(m_group, "show_focus"));
    m_pControlChainShowFocus->setButtonMode(ControlPushButton::TOGGLE);

    m_pControlChainHasControllerFocus = new ControlPushButton(
                                   ConfigKey(m_group, "controller_input_active"));
    m_pControlChainHasControllerFocus->setButtonMode(ControlPushButton::TOGGLE);

    m_pControlChainShowParameters = new ControlPushButton(
                                        ConfigKey(m_group, "show_parameters"),
                                        true);
    m_pControlChainShowParameters->setButtonMode(ControlPushButton::TOGGLE);

    m_pControlChainFocusedEffect = new ControlPushButton(
                                       ConfigKey(m_group, "focused_effect"),
                                       true);
    m_pControlChainFocusedEffect->setButtonMode(ControlPushButton::TOGGLE);
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
    delete m_pControlChainMixMode;
    delete m_pControlChainPrevPreset;
    delete m_pControlChainNextPreset;
    delete m_pControlChainSelector;
    delete m_pControlChainShowFocus;
    delete m_pControlChainHasControllerFocus;
    delete m_pControlChainShowParameters;
    delete m_pControlChainFocusedEffect;

    for (QMap<QString, ChannelInfo*>::iterator it = m_channelInfoByName.begin();
         it != m_channelInfoByName.end();) {
        delete it.value();
        it = m_channelInfoByName.erase(it);
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

void EffectChainSlot::setSuperParameter(double value, bool force) {
    m_pControlChainSuperParameter->set(value);
    slotControlChainSuperParameter(value, force);
}

void EffectChainSlot::setSuperParameterDefaultValue(double value) {
    m_pControlChainSuperParameter->setDefaultValue(value);
}

void EffectChainSlot::slotChainNameChanged(const QString&) {
    emit updated();
}

void EffectChainSlot::slotChainEnabledChanged(bool bEnabled) {
    m_pControlChainEnabled->set(bEnabled);
    emit updated();
}

void EffectChainSlot::slotChainMixChanged(double mix) {
    m_pControlChainMix->set(mix);
    emit updated();
}

void EffectChainSlot::slotChainMixModeChanged(EffectChainMixMode mixMode) {
    m_pControlChainMixMode->set(static_cast<double>(mixMode));
    emit updated();
}

void EffectChainSlot::slotChainChannelStatusChanged(const QString& group,
                                                    bool enabled) {
    ChannelInfo* pInfo = m_channelInfoByName.value(group, NULL);
    if (pInfo != NULL && pInfo->pEnabled != NULL) {
        pInfo->pEnabled->set(enabled);
        emit updated();
    }
}

void EffectChainSlot::slotChainEffectChanged(unsigned int effectSlotNumber,
                                             bool shouldEmit) {
    //qDebug() << debugString() << "slotChainEffectChanged" << effectSlotNumber;
    if (m_pEffectChain) {
        const QList<EffectPointer> effects = m_pEffectChain->effects();
        EffectSlotPointer pSlot;
        EffectPointer pEffect;

        if (effects.size() > m_slots.size()) {
            qWarning() << debugString() << "has too few slots for effect";
        }

        if (effectSlotNumber < (unsigned) m_slots.size()) {
            pSlot = m_slots.at(effectSlotNumber);
        }
        if (effectSlotNumber < (unsigned) effects.size()) {
            pEffect = effects.at(effectSlotNumber);
        }
        if (pSlot != nullptr) {
            pSlot->loadEffect(pEffect, m_pEffectRack->isAdoptMetaknobValueEnabled());
        }

        m_pControlNumEffects->forceSet(math_min(
                static_cast<unsigned int>(m_slots.size()),
                m_pEffectChain->numEffects()));

        if (shouldEmit) {
            emit updated();
        }
    }
}

void EffectChainSlot::loadEffectChainToSlot(EffectChainPointer pEffectChain) {
    //qDebug() << debugString() << "loadEffectChainToSlot" << (pEffectChain ? pEffectChain->id() : "(null)");
    clear();

    if (pEffectChain) {
        m_pEffectChain = pEffectChain;

        connect(m_pEffectChain.data(), SIGNAL(effectChanged(unsigned int)),
                this, SLOT(slotChainEffectChanged(unsigned int)));
        connect(m_pEffectChain.data(), SIGNAL(nameChanged(const QString&)),
                this, SLOT(slotChainNameChanged(const QString&)));
        connect(m_pEffectChain.data(), SIGNAL(enabledChanged(bool)),
                this, SLOT(slotChainEnabledChanged(bool)));
        connect(m_pEffectChain.data(), SIGNAL(mixChanged(double)),
                this, SLOT(slotChainMixChanged(double)));
        connect(m_pEffectChain.data(), SIGNAL(mixModeChanged(EffectChainMixMode)),
                this, SLOT(slotChainMixModeChanged(EffectChainMixMode)));
        connect(m_pEffectChain.data(), SIGNAL(channelStatusChanged(const QString&, bool)),
                this, SLOT(slotChainChannelStatusChanged(const QString&, bool)));

        m_pControlChainLoaded->forceSet(true);
        m_pControlChainMixMode->set(
                static_cast<double>(m_pEffectChain->mixMode()));

        // Mix and enabled channels are persistent properties of the chain slot,
        // not of the chain. Propagate the current settings to the chain.
        m_pEffectChain->setMix(m_pControlChainMix->get());
        m_pEffectChain->setEnabled(m_pControlChainEnabled->get() > 0.0);

        // Don't emit because we will below.
        for (int i = 0; i < m_slots.size(); ++i) {
            slotChainEffectChanged(i, false);
        }
    }

    emit effectChainLoaded(pEffectChain);
    emit updated();
}

void EffectChainSlot::updateRoutingSwitches() {
    VERIFY_OR_DEBUG_ASSERT(m_pEffectChain) {
        return;
    }
    for (const ChannelInfo* pChannelInfo : qAsConst(m_channelInfoByName)) {
        if (pChannelInfo->pEnabled->toBool()) {
            m_pEffectChain->enableForInputChannel(pChannelInfo->handle_group);
        } else {
            m_pEffectChain->disableForInputChannel(pChannelInfo->handle_group);
        }
    }
}

EffectChainPointer EffectChainSlot::getEffectChain() const {
    return m_pEffectChain;
}

EffectChainPointer EffectChainSlot::getOrCreateEffectChain(
        EffectsManager* pEffectsManager) {
    if (!m_pEffectChain) {
        EffectChainPointer pEffectChain(
                new EffectChain(pEffectsManager, QString()));
        //: Name for an empty effect chain, that is created after eject
        pEffectChain->setName(tr("Empty Chain"));
        loadEffectChainToSlot(pEffectChain);
        pEffectChain->addToEngine(m_pEffectRack->getEngineEffectRack(), m_iChainSlotNumber);
        pEffectChain->updateEngineState();
        updateRoutingSwitches();
    }
    return m_pEffectChain;
}

void EffectChainSlot::clear() {
    // Stop listening to signals from any loaded effect
    if (m_pEffectChain) {
        m_pEffectChain->removeFromEngine(m_pEffectRack->getEngineEffectRack(),
                                         m_iChainSlotNumber);
        for (EffectSlotPointer pSlot : qAsConst(m_slots)) {
            pSlot->clear();
        }
        m_pEffectChain->disconnect(this);
        m_pEffectChain.clear();
    }
    m_pControlNumEffects->forceSet(0.0);
    m_pControlChainLoaded->forceSet(0.0);
    m_pControlChainMixMode->set(
            static_cast<double>(EffectChainMixMode::DrySlashWet));
    emit updated();
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
    int numEffectSlots = static_cast<int>(m_pControlNumEffectSlots->get()) + 1;
    m_pControlNumEffectSlots->forceSet(numEffectSlots);
    m_pControlChainFocusedEffect->setStates(numEffectSlots);
    return pSlot;
}

void EffectChainSlot::registerInputChannel(const ChannelHandleAndGroup& handle_group) {
    VERIFY_OR_DEBUG_ASSERT(!m_channelInfoByName.contains(handle_group.name())) {
        return;
    }

    double initialValue = 0.0;
    int deckNumber;
    if (PlayerManager::isDeckGroup(handle_group.name(), &deckNumber) &&
        (m_iChainSlotNumber + 1) == (unsigned) deckNumber) {
        initialValue = 1.0;
    }
    ControlPushButton* pEnableControl = new ControlPushButton(
            ConfigKey(m_group, QString("group_%1_enable").arg(handle_group.name())),
            true, initialValue);
    pEnableControl->setButtonMode(ControlPushButton::POWERWINDOW);

    ChannelInfo* pInfo = new ChannelInfo(handle_group, pEnableControl);
    m_channelInfoByName[handle_group.name()] = pInfo;
    connect(pEnableControl, &ControlPushButton::valueChanged,
            this, [this, handle_group] { slotChannelStatusChanged(handle_group.name()); });

}

void EffectChainSlot::slotEffectLoaded(EffectPointer pEffect, unsigned int slotNumber) {
    // const int is a safe read... don't bother locking
    emit effectLoaded(pEffect, m_iChainSlotNumber, slotNumber);
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

void EffectChainSlot::slotControlChainSuperParameter(double v, bool force) {
    //qDebug() << debugString() << "slotControlChainSuperParameter" << v;

    // Clamp to [0.0, 1.0]
    if (v < 0.0 || v > 1.0) {
        qWarning() << debugString() << "value out of limits";
        v = math_clamp(v, 0.0, 1.0);
        m_pControlChainSuperParameter->set(v);
    }
    for (const auto& pSlot : qAsConst(m_slots)) {
        pSlot->setMetaParameter(v, force);
    }
}

void EffectChainSlot::slotControlChainMixMode(double v) {
    // Intermediate cast to integer is needed for VC++.
    EffectChainMixMode type = static_cast<EffectChainMixMode>(int(v));
    (void)v; // this avoids a false warning with g++ 4.8.1
    if (m_pEffectChain && type < EffectChainMixMode::NumMixModes) {
        m_pEffectChain->setMixMode(type);
    }
}

void EffectChainSlot::slotControlChainSelector(double v) {
    //qDebug() << debugString() << "slotControlChainSelector" << v;
    if (v > 0) {
        emit nextChain(m_iChainSlotNumber, m_pEffectChain);
    } else if (v < 0) {
        emit prevChain(m_iChainSlotNumber, m_pEffectChain);
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

void EffectChainSlot::slotChannelStatusChanged(const QString& group) {
    if (m_pEffectChain) {
        ChannelInfo* pChannelInfo = m_channelInfoByName.value(group, NULL);
        if (pChannelInfo != NULL && pChannelInfo->pEnabled != NULL) {
            bool bEnable = pChannelInfo->pEnabled->toBool();
            if (bEnable) {
                m_pEffectChain->enableForInputChannel(pChannelInfo->handle_group);
            } else {
                m_pEffectChain->disableForInputChannel(pChannelInfo->handle_group);
            }
        }
    }
}

unsigned int EffectChainSlot::getChainSlotNumber() const {
    return m_iChainSlotNumber;
}

QDomElement EffectChainSlot::toXml(QDomDocument* doc) const {
    QDomElement chainElement = doc->createElement(EffectXml::Chain);
    if (m_pEffectChain == nullptr) {
        // ejected chains are stored empty <EffectChain/>
        return chainElement;
    }

    XmlParse::addElement(*doc, chainElement, EffectXml::ChainId,
            m_pEffectChain->id());
    XmlParse::addElement(*doc, chainElement, EffectXml::ChainName,
            m_pEffectChain->name());
    XmlParse::addElement(*doc, chainElement, EffectXml::ChainDescription,
            m_pEffectChain->description());
    XmlParse::addElement(*doc, chainElement, EffectXml::ChainMixMode,
            EffectChain::mixModeToString(
                    static_cast<EffectChainMixMode>(
                            static_cast<int>(m_pControlChainMixMode->get()))));
    XmlParse::addElement(*doc, chainElement, EffectXml::ChainSuperParameter,
            QString::number(m_pControlChainSuperParameter->get()));

    QDomElement effectsElement = doc->createElement(EffectXml::EffectsRoot);
    for (const auto& pEffectSlot : m_slots) {
        QDomElement effectNode;
        if (pEffectSlot->getEffect()) {
            effectNode = pEffectSlot->toXml(doc);
        } else {
            // Create empty element to ensure effects stay in order
            // if there are empty slots before loaded slots.
            effectNode = doc->createElement(EffectXml::Effect);
        }
        effectsElement.appendChild(effectNode);
    }
    chainElement.appendChild(effectsElement);

    return chainElement;
}

void EffectChainSlot::loadChainSlotFromXml(const QDomElement& effectChainElement) {
    if (!effectChainElement.hasChildNodes()) {
        return;
    }

    // FIXME: mix mode is set in EffectChain::createFromXml

    m_pControlChainSuperParameter->set(XmlParse::selectNodeDouble(
                                          effectChainElement,
                                          EffectXml::ChainSuperParameter));

    QDomElement effectsElement = XmlParse::selectElement(effectChainElement,
                                                         EffectXml::EffectsRoot);
    QDomNodeList effectsNodeList = effectsElement.childNodes();
    for (int i = 0; i < m_slots.size(); ++i) {
        if (m_slots[i] != nullptr) {
            QDomNode effectNode = effectsNodeList.at(i);
            if (effectNode.isElement()) {
                QDomElement effectElement = effectNode.toElement();
                m_slots[i]->loadEffectSlotFromXml(effectElement);
            }
        }
    }
}
