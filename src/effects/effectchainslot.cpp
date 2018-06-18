#include "effects/effectchainslot.h"

#include "control/controlencoder.h"
#include "control/controlpotmeter.h"
#include "control/controlpushbutton.h"
#include "effects/effectprocessor.h"
#include "effects/effectrack.h"
#include "effects/effectslot.h"
#include "effects/effectsmanager.h"
#include "effects/effectxmlelements.h"
#include "engine/effects/engineeffectchain.h"
#include "engine/effects/message.h"
#include "engine/engine.h"
#include "mixer/playermanager.h"
#include "util/defs.h"
#include "util/math.h"
#include "util/sample.h"
#include "util/xml.h"


EffectChainSlot::EffectChainSlot(EffectRack* pRack, const QString& group,
                                 unsigned int iChainNumber,
                                 EffectsManager* pEffectsManager,
                                 const QString& id)
        : m_iChainSlotNumber(iChainNumber),
          // The control group names are 1-indexed while internally everything
          // is 0-indexed.
          m_group(group),
          m_pEffectRack(pRack),
          m_pEffectsManager(pEffectsManager),
          m_id(id),
          m_name(""),
          m_description(""),
          m_pEngineEffectChain(nullptr) {
    // qDebug() << "EffectChainSlot::EffectChainSlot " << pRack << ' ' << group << ' ' << iChainNumber;

    m_pControlClear = new ControlPushButton(ConfigKey(m_group, "clear"));
    connect(m_pControlClear, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlClear(double)));

    m_pControlNumEffects = new ControlObject(ConfigKey(m_group, "num_effects"));
    m_pControlNumEffects->setReadOnly();

    m_pControlNumEffectSlots = new ControlObject(ConfigKey(m_group, "num_effectslots"));
    m_pControlNumEffectSlots->setReadOnly();

    m_pControlChainLoaded = new ControlObject(ConfigKey(m_group, "loaded"));
    m_pControlChainLoaded->setReadOnly();
    if (id != QString()) {
        m_pControlChainLoaded->forceSet(1.0);
    }

    m_pControlChainEnabled = new ControlPushButton(ConfigKey(m_group, "enabled"));
    m_pControlChainEnabled->setButtonMode(ControlPushButton::POWERWINDOW);
    // Default to enabled. The skin might not show these buttons.
    m_pControlChainEnabled->setDefaultValue(true);
    m_pControlChainEnabled->set(true);
    connect(m_pControlChainEnabled, SIGNAL(valueChanged(double)),
            this, SLOT(sendParameterUpdate()));

    m_pControlChainMix = new ControlPotmeter(ConfigKey(m_group, "mix"), 0.0, 1.0,
                                             false, true, false, true, 1.0);
    connect(m_pControlChainMix, SIGNAL(valueChanged(double)),
            this, SLOT(sendParameterUpdate()));

    m_pControlChainSuperParameter = new ControlPotmeter(ConfigKey(m_group, "super1"), 0.0, 1.0);
    connect(m_pControlChainSuperParameter, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlChainSuperParameter(double)));
    m_pControlChainSuperParameter->set(0.0);
    m_pControlChainSuperParameter->setDefaultValue(0.0);

    m_pControlChainMixMode = new ControlPushButton(ConfigKey(m_group, "mix_mode"));
    m_pControlChainMixMode->setButtonMode(ControlPushButton::TOGGLE);
    m_pControlChainMixMode->setStates(static_cast<int>(EffectChainMixMode::NumMixModes));
    connect(m_pControlChainMixMode, SIGNAL(valueChanged(double)),
            this, SLOT(sendParameterUpdate()));

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

    connect(&m_channelStatusMapper, SIGNAL(mapped(const QString&)),
            this, SLOT(slotChannelStatusChanged(const QString&)));

    // ControlObjects for skin <-> controller mapping interaction.
    // Refer to comment in header for full explanation.
    m_pControlChainShowFocus = new ControlPushButton(
                                   ConfigKey(m_group, "show_focus"));
    m_pControlChainShowFocus->setButtonMode(ControlPushButton::TOGGLE);

    m_pControlChainShowParameters = new ControlPushButton(
                                        ConfigKey(m_group, "show_parameters"),
                                        true);
    m_pControlChainShowParameters->setButtonMode(ControlPushButton::TOGGLE);

    m_pControlChainFocusedEffect = new ControlPushButton(
                                       ConfigKey(m_group, "focused_effect"),
                                       true);
    m_pControlChainFocusedEffect->setButtonMode(ControlPushButton::TOGGLE);

    addToEngine(m_iChainSlotNumber);
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
    delete m_pControlChainShowParameters;
    delete m_pControlChainFocusedEffect;

    for (QMap<QString, ChannelInfo*>::iterator it = m_channelInfoByName.begin();
         it != m_channelInfoByName.end();) {
        delete it.value();
        it = m_channelInfoByName.erase(it);
    }

    m_slots.clear();
    removeFromEngine(m_iChainSlotNumber);
}

void EffectChainSlot::addToEngine(int iIndex) {
    m_pEngineEffectChain = new EngineEffectChain(m_id,
        m_pEffectsManager->registeredInputChannels(),
        m_pEffectsManager->registeredOutputChannels());
    EffectsRequest* pRequest = new EffectsRequest();
    pRequest->type = EffectsRequest::ADD_EFFECT_CHAIN;
    pRequest->AddEffectChain.signalProcessingStage = m_pEffectRack->getSignalProcessingStage();
    pRequest->AddEffectChain.pChain = m_pEngineEffectChain;
    pRequest->AddEffectChain.iIndex = iIndex;
    m_pEffectsManager->writeRequest(pRequest);

    // Add all effects.
    for (int i = 0; i < m_effects.size(); ++i) {
        // Add the effect to the engine.
        EffectPointer pEffect = m_effects[i];
        if (pEffect) {
            pEffect->addToEngine(m_pEngineEffectChain, i, m_enabledInputChannels);
        }
    }
    sendParameterUpdate();
}

void EffectChainSlot::removeFromEngine(int iIndex) {
    // Order doesn't matter when removing.
    for (int i = 0; i < m_effects.size(); ++i) {
        EffectPointer pEffect = m_effects[i];
        if (pEffect) {
            pEffect->removeFromEngine(m_pEngineEffectChain, i);
        }
    }

    EffectsRequest* pRequest = new EffectsRequest();
    pRequest->type = EffectsRequest::REMOVE_EFFECT_CHAIN;
    pRequest->RemoveEffectChain.signalProcessingStage = m_pEffectRack->getSignalProcessingStage();
    pRequest->RemoveEffectChain.pChain = m_pEngineEffectChain;
    pRequest->RemoveEffectChain.iIndex = iIndex;
    m_pEffectsManager->writeRequest(pRequest);

    m_pEngineEffectChain = nullptr;
}

const QString& EffectChainSlot::name() const {
    return m_name;
}

void EffectChainSlot::setName(const QString& name) {
    m_name = name;
    emit(updated());
}

QString EffectChainSlot::description() const {
    return m_description;
}

void EffectChainSlot::setDescription(const QString& description) {
    m_description = description;
    emit(updated());
}

void EffectChainSlot::setMix(const double& dMix) {
    m_pControlChainMix->set(dMix);
    sendParameterUpdate();
}

void EffectChainSlot::addEffect(EffectPointer pEffect) {
    // qDebug() << debugString() << "addEffect" << pEffect;
    if (!pEffect) {
        // Insert empty effects to preserve chain order
        // when loading chains with empty effects
        m_effects.append(pEffect);
        return;
    }

    VERIFY_OR_DEBUG_ASSERT(!m_effects.contains(pEffect)) {
        return;
    }

    m_effects.append(pEffect);
    pEffect->addToEngine(m_pEngineEffectChain, m_effects.size() - 1, m_enabledInputChannels);
    slotChainEffectChanged(m_effects.size() - 1);
}

void EffectChainSlot::replaceEffect(unsigned int effectSlotNumber,
                                EffectPointer pEffect) {
    // qDebug() << debugString() << "replaceEffect" << effectSlotNumber << pEffect;
    while (effectSlotNumber >= static_cast<unsigned int>(m_effects.size())) {
        if (pEffect.isNull()) {
            return;
        }
        m_effects.append(EffectPointer());
    }

    EffectPointer pOldEffect = m_effects[effectSlotNumber];
    if (!pOldEffect.isNull()) {
        pOldEffect->removeFromEngine(m_pEngineEffectChain, effectSlotNumber);
    }

    m_effects.replace(effectSlotNumber, pEffect);
    if (!pEffect.isNull()) {
        pEffect->addToEngine(m_pEngineEffectChain, effectSlotNumber, m_enabledInputChannels);
    }

    slotChainEffectChanged(effectSlotNumber);
}

void EffectChainSlot::removeEffect(unsigned int effectSlotNumber) {
    replaceEffect(effectSlotNumber, EffectPointer());
}

void EffectChainSlot::refreshAllEffects() {
    for (int i = 0; i < m_effects.size(); ++i) {
        slotChainEffectChanged(i);
    }
}

const QList<EffectPointer>& EffectChainSlot::effects() const {
    return m_effects;
}

void EffectChainSlot::sendParameterUpdate() {
    EffectsRequest* pRequest = new EffectsRequest();
    pRequest->type = EffectsRequest::SET_EFFECT_CHAIN_PARAMETERS;
    pRequest->pTargetChain = m_pEngineEffectChain;
    pRequest->SetEffectChainParameters.enabled = m_pControlChainEnabled->get();
    pRequest->SetEffectChainParameters.mix_mode = static_cast<EffectChainMixMode>(
                                                    static_cast<int>(m_pControlChainMixMode->get()));
    pRequest->SetEffectChainParameters.mix = m_pControlChainMix->get();
    m_pEffectsManager->writeRequest(pRequest);
}

QString EffectChainSlot::id() const {
    return m_id;
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

void EffectChainSlot::slotChainEffectChanged(unsigned int effectSlotNumber) {
    qDebug() << debugString() << "slotChainEffectChanged" << effectSlotNumber;
    EffectSlotPointer pSlot;
    EffectPointer pEffect;

    if (m_effects.size() > m_slots.size()) {
        qWarning() << debugString() << "has too few slots for effect";
    }
    if (effectSlotNumber < (unsigned) m_slots.size()) {
        pSlot = m_slots.at(effectSlotNumber);
    }
    if (effectSlotNumber < (unsigned) m_effects.size()) {
        pEffect = m_effects.at(effectSlotNumber);
    }
    if (pSlot != nullptr) {
        pSlot->loadEffect(pEffect, m_pEffectRack->isAdoptMetaknobValueEnabled());
    }

    m_pControlNumEffects->forceSet(math_min(
            static_cast<unsigned int>(m_slots.size()),
            static_cast<unsigned int>(m_effects.size())));
}

void EffectChainSlot::clear() {
    m_pControlNumEffects->forceSet(0.0);
    m_pControlChainLoaded->forceSet(0.0);
    m_pControlChainMixMode->set(
            static_cast<double>(EffectChainMixMode::DrySlashWet));
}

EffectSlotPointer EffectChainSlot::addEffectSlot(const QString& group) {
    // qDebug() << debugString() << "addEffectSlot" << group;

    EffectSlot* pEffectSlot = new EffectSlot(group, m_iChainSlotNumber,
                                             m_slots.size());
    connect(pEffectSlot, SIGNAL(clearEffect(unsigned int)),
            this, SLOT(slotClearEffect(unsigned int)));
    connect(pEffectSlot, SIGNAL(nextEffect(unsigned int, unsigned int, EffectPointer)),
            this, SIGNAL(nextEffect(unsigned int, unsigned int, EffectPointer)));
    connect(pEffectSlot, SIGNAL(prevEffect(unsigned int, unsigned int, EffectPointer)),
            this, SIGNAL(prevEffect(unsigned int, unsigned int, EffectPointer)));

    EffectSlotPointer pSlot(pEffectSlot);
    m_slots.append(pSlot);
    int numEffectSlots = m_pControlNumEffectSlots->get() + 1;
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
    if (pEnableControl->toBool()) {
        enableForInputChannel(handle_group);
    }

    ChannelInfo* pInfo = new ChannelInfo(handle_group, pEnableControl);
    m_channelInfoByName[handle_group.name()] = pInfo;

    // m_channelStatusMapper will emit a mapped(handle_group.name()) signal whenever
    // the valueChanged(double) signal is emitted by pEnableControl 
    m_channelStatusMapper.setMapping(pEnableControl, handle_group.name());
    connect(pEnableControl, SIGNAL(valueChanged(double)),
            &m_channelStatusMapper, SLOT(map()));
}

void EffectChainSlot::slotClearEffect(unsigned int iEffectSlotNumber) {
    removeEffect(iEffectSlotNumber);
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
        for (EffectSlotPointer pSlot : m_slots) {
            pSlot->clear();
        }
        clear();
    }
}

void EffectChainSlot::slotControlChainSuperParameter(double v, bool force) {
    // qDebug() << debugString() << "slotControlChainSuperParameter" << v;

    m_pControlChainSuperParameter->set(v);
    for (const auto& pSlot : m_slots) {
        pSlot->setMetaParameter(v, force);
    }
}

void EffectChainSlot::slotControlChainSelector(double v) {
//     qDebug() << debugString() << "slotControlChainSelector" << v;
//     if (v > 0) {
//         emit(nextChain(m_iChainSlotNumber, m_pEffectChain));
//     } else if (v < 0) {
//         emit(prevChain(m_iChainSlotNumber, m_pEffectChain));
//     }
}

void EffectChainSlot::slotControlChainNextPreset(double v) {
    // qDebug() << debugString() << "slotControlChainNextPreset" << v;
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
    ChannelInfo* pChannelInfo = m_channelInfoByName.value(group, NULL);
    if (pChannelInfo != NULL && pChannelInfo->pEnabled != NULL) {
        bool bEnable = pChannelInfo->pEnabled->toBool();
        if (bEnable) {
            enableForInputChannel(pChannelInfo->handle_group);
        } else {
            disableForInputChannel(pChannelInfo->handle_group);
        }
    }
}

void EffectChainSlot::enableForInputChannel(const ChannelHandleAndGroup& handle_group) {
    if (m_enabledInputChannels.contains(handle_group)) {
        return;
    }

    EffectsRequest* request = new EffectsRequest();
    request->type = EffectsRequest::ENABLE_EFFECT_CHAIN_FOR_INPUT_CHANNEL;
    request->pTargetChain = m_pEngineEffectChain;
    request->EnableInputChannelForChain.pChannelHandle = &handle_group.handle();

    // Allocate EffectStates here in the main thread to avoid allocating
    // memory in the realtime audio callback thread. Pointers to the
    // EffectStates are passed to the EffectRequest and the EffectProcessorImpls
    // store the pointers. The containers of EffectState* pointers get deleted
    // by ~EffectsRequest, but the EffectStates are managed by EffectProcessorImpl.
    auto pEffectStatesMapArray = new EffectStatesMapArray;

    //TODO: get actual configuration of engine
    const mixxx::EngineParameters bufferParameters(
          mixxx::AudioSignal::SampleRate(96000),
          MAX_BUFFER_LEN / mixxx::kEngineChannelCount);

    // TODO: Simplify by defining a method to create an EffectState for the input channel
    for (int i = 0; i < m_effects.size(); ++i) {
        auto& statesMap = (*pEffectStatesMapArray)[i];
        if (m_effects[i] != nullptr) {
            for (const auto& outputChannel : m_pEffectsManager->registeredOutputChannels()) {
                if (kEffectDebugOutput) {
                    qDebug() << debugString() << "EffectChain::enableForInputChannel creating EffectState for input" << handle_group << "output" << outputChannel;
                }
                statesMap.insert(outputChannel.handle(),
                        m_effects[i]->createState(bufferParameters));
            }
        } else {
            for (EffectState* pState : statesMap) {
                if (pState != nullptr) {
                    delete pState;
                }
            }
            statesMap.clear();
        }
    }
    request->EnableInputChannelForChain.pEffectStatesMapArray = pEffectStatesMapArray;

    m_pEffectsManager->writeRequest(request);

    m_enabledInputChannels.insert(handle_group);
}

void EffectChainSlot::disableForInputChannel(const ChannelHandleAndGroup& handle_group) {
    if (!m_enabledInputChannels.remove(handle_group)) {
        return;
    }

    EffectsRequest* request = new EffectsRequest();
    request->type = EffectsRequest::DISABLE_EFFECT_CHAIN_FOR_INPUT_CHANNEL;
    request->pTargetChain = m_pEngineEffectChain;
    request->DisableInputChannelForChain.pChannelHandle = &handle_group.handle();
    m_pEffectsManager->writeRequest(request);
}

unsigned int EffectChainSlot::getChainSlotNumber() const {
    return m_iChainSlotNumber;
}

QDomElement EffectChainSlot::toXml(QDomDocument* doc) const {
    QDomElement chainElement = doc->createElement(EffectXml::Chain);

    // XmlParse::addElement(*doc, chainElement, EffectXml::ChainId,
    //         m_pEffectChain->id());
    // XmlParse::addElement(*doc, chainElement, EffectXml::ChainName,
    //         m_pEffectChain->name());
    // XmlParse::addElement(*doc, chainElement, EffectXml::ChainDescription,
    //         m_pEffectChain->description());
    // XmlParse::addElement(*doc, chainElement, EffectXml::ChainMixMode,
    //         EffectChain::mixModeToString(
    //                 static_cast<EffectChainMixMode>(
    //                         static_cast<int>(m_pControlChainMixMode->get()))));
    // XmlParse::addElement(*doc, chainElement, EffectXml::ChainSuperParameter,
    //         QString::number(m_pControlChainSuperParameter->get()));

    // QDomElement effectsElement = doc->createElement(EffectXml::EffectsRoot);
    // for (const auto& pEffectSlot : m_slots) {
    //     QDomElement effectNode;
    //     if (pEffectSlot->getEffect()) {
    //         effectNode = pEffectSlot->toXml(doc);
    //     } else {
    //         // Create empty element to ensure effects stay in order
    //         // if there are empty slots before loaded slots.
    //         effectNode = doc->createElement(EffectXml::Effect);
    //     }
    //     effectsElement.appendChild(effectNode);
    // }
    // chainElement.appendChild(effectsElement);

    return chainElement;
}

void EffectChainSlot::loadChainSlotFromXml(const QDomElement& effectChainElement) {
    // if (!effectChainElement.hasChildNodes()) {
    //     return;
    // }

    // // FIXME: mix mode is set in EffectChain::createFromXml

    // m_pControlChainSuperParameter->set(XmlParse::selectNodeDouble(
    //                                       effectChainElement,
    //                                       EffectXml::ChainSuperParameter));

    // QDomElement effectsElement = XmlParse::selectElement(effectChainElement,
    //                                                      EffectXml::EffectsRoot);
    // QDomNodeList effectsNodeList = effectsElement.childNodes();
    // for (int i = 0; i < m_slots.size(); ++i) {
    //     if (m_slots[i] != nullptr) {
    //         QDomNode effectNode = effectsNodeList.at(i);
    //         if (effectNode.isElement()) {
    //             QDomElement effectElement = effectNode.toElement();
    //             m_slots[i]->loadEffectSlotFromXml(effectElement);
    //         }
    //     }
    // }
}
