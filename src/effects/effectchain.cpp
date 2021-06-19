#include "effects/effectchain.h"

#include "effects/effectchainmanager.h"
#include "effects/effectprocessor.h"
#include "effects/effectsmanager.h"
#include "effects/effectxmlelements.h"
#include "engine/effects/engineeffectchain.h"
#include "engine/effects/engineeffectrack.h"
#include "engine/effects/message.h"
#include "engine/engine.h"
#include "moc_effectchain.cpp"
#include "util/defs.h"
#include "util/sample.h"
#include "util/xml.h"

EffectChain::EffectChain(EffectsManager* pEffectsManager, const QString& id,
                         EffectChainPointer pPrototype)
        : QObject(pEffectsManager),
          m_pEffectsManager(pEffectsManager),
          m_pPrototype(pPrototype),
          m_bEnabled(true),
          m_id(id),
          m_name(""),
          m_mixMode(EffectChainMixMode::DrySlashWet),
          m_dMix(0),
          m_pEngineEffectChain(nullptr),
          m_bAddedToEngine(false) {
}

EffectChain::~EffectChain() {
    // Remove all effects.
    for (int i = 0; i < m_effects.size(); ++i) {
        removeEffect(i);
    }
}

void EffectChain::addToEngine(EngineEffectRack* pRack, int iIndex) {
    m_pEngineEffectChain = new EngineEffectChain(m_id,
        m_pEffectsManager->registeredInputChannels(),
        m_pEffectsManager->registeredOutputChannels());
    EffectsRequest* pRequest = new EffectsRequest();
    pRequest->type = EffectsRequest::ADD_CHAIN_TO_RACK;
    pRequest->pTargetRack = pRack;
    pRequest->AddChainToRack.pChain = m_pEngineEffectChain;
    pRequest->AddChainToRack.iIndex = iIndex;
    m_pEffectsManager->writeRequest(pRequest);
    m_bAddedToEngine = true;

    // Add all effects.
    for (int i = 0; i < m_effects.size(); ++i) {
        // Add the effect to the engine.
        EffectPointer pEffect = m_effects[i];
        if (pEffect) {
            pEffect->addToEngine(m_pEngineEffectChain, i, m_enabledInputChannels);
        }
    }
}

void EffectChain::removeFromEngine(EngineEffectRack* pRack, int iIndex) {
    if (!m_bAddedToEngine) {
        return;
    }

    // Order doesn't matter when removing.
    for (int i = 0; i < m_effects.size(); ++i) {
        EffectPointer pEffect = m_effects[i];
        if (pEffect) {
            pEffect->removeFromEngine(m_pEngineEffectChain, i);
        }
    }

    EffectsRequest* pRequest = new EffectsRequest();
    pRequest->type = EffectsRequest::REMOVE_CHAIN_FROM_RACK;
    pRequest->pTargetRack = pRack;
    pRequest->RemoveChainFromRack.pChain = m_pEngineEffectChain;
    pRequest->RemoveChainFromRack.iIndex = iIndex;
    m_pEffectsManager->writeRequest(pRequest);
    m_bAddedToEngine = false;

    m_pEngineEffectChain = nullptr;
}

void EffectChain::updateEngineState() {
    if (!m_bAddedToEngine) {
        return;
    }
    // Update chain parameters in the engine.
    sendParameterUpdate();
    for (int i = 0; i < m_effects.size(); ++i) {
        EffectPointer pEffect = m_effects[i];
        if (pEffect) {
            // Update effect parameters in the engine.
            pEffect->updateEngineState();
        }
    }
}

// static
EffectChainPointer EffectChain::clone(EffectChainPointer pChain) {
    if (!pChain) {
        return EffectChainPointer();
    }

    EffectChain* pClone = new EffectChain(
        pChain->m_pEffectsManager, pChain->id(), pChain);
    pClone->setName(pChain->name());
    // Do not set the state of the chain because that information belongs
    // to the EffectChainSlot. Leave that to EffectChainSlot::loadEffectChain.
    for (const auto& pEffect : pChain->effects()) {
        EffectPointer pClonedEffect;
        if (pEffect == nullptr) {
            // Insert empty effect to preserve chain order
            pClonedEffect = EffectPointer();
        } else {
            pClonedEffect = pChain->m_pEffectsManager->instantiateEffect(
                    pEffect->getManifest()->id());
        }
        pClone->addEffect(pClonedEffect);
    }
    return EffectChainPointer(pClone);
}

EffectChainPointer EffectChain::prototype() const {
    return m_pPrototype;
}

const QString& EffectChain::id() const {
    return m_id;
}

const QString& EffectChain::name() const {
    return m_name;
}

void EffectChain::setName(const QString& name) {
    m_name = name;
    emit nameChanged(name);
}

QString EffectChain::description() const {
    return m_description;
}

void EffectChain::setDescription(const QString& description) {
    m_description = description;
    emit descriptionChanged(description);
}

bool EffectChain::enabled() const {
    return m_bEnabled;
}

void EffectChain::setEnabled(bool enabled) {
    m_bEnabled = enabled;
    sendParameterUpdate();
    emit enabledChanged(enabled);
}

void EffectChain::enableForInputChannel(const ChannelHandleAndGroup& handleGroup) {
    // TODO(Be): remove m_enabledChannels from this class and move this logic
    // to EffectChainSlot
    bool bWasAlreadyEnabled = m_enabledInputChannels.contains(handleGroup);
    if (!bWasAlreadyEnabled) {
        m_enabledInputChannels.insert(handleGroup);
    }

    // The allocation of EffectStates below may be expensive, so avoid it if
    // not needed.
    if (!m_bAddedToEngine || bWasAlreadyEnabled) {
        return;
    }

    EffectsRequest* request = new EffectsRequest();
    request->type = EffectsRequest::ENABLE_EFFECT_CHAIN_FOR_INPUT_CHANNEL;
    request->pTargetChain = m_pEngineEffectChain;
    request->EnableInputChannelForChain.pChannelHandle = &handleGroup.handle();

    // Allocate EffectStates here in the main thread to avoid allocating
    // memory in the realtime audio callback thread. Pointers to the
    // EffectStates are passed to the EffectRequest and the EffectProcessorImpls
    // store the pointers. The containers of EffectState* pointers get deleted
    // by ~EffectsRequest, but the EffectStates are managed by EffectProcessorImpl.
    auto* pEffectStatesMapArray = new EffectStatesMapArray;

    //TODO: get actual configuration of engine
    const mixxx::EngineParameters bufferParameters(
          mixxx::audio::SampleRate(96000),
          MAX_BUFFER_LEN / mixxx::kEngineChannelCount);

    for (int i = 0; i < m_effects.size(); ++i) {
        auto& statesMap = (*pEffectStatesMapArray)[i];
        if (m_effects[i] != nullptr) {
            for (const auto& outputChannel : m_pEffectsManager->registeredOutputChannels()) {
                if (kEffectDebugOutput) {
                    qDebug() << debugString() << "EffectChain::enableForInputChannel creating EffectState for input" << handleGroup << "output" << outputChannel;
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
    emit channelStatusChanged(handleGroup.name(), true);
}

bool EffectChain::enabledForChannel(const ChannelHandleAndGroup& handleGroup) const {
    return m_enabledInputChannels.contains(handleGroup);
}

void EffectChain::disableForInputChannel(const ChannelHandleAndGroup& handleGroup) {
    if (m_enabledInputChannels.remove(handleGroup)) {
        if (!m_bAddedToEngine) {
            return;
        }
        EffectsRequest* request = new EffectsRequest();
        request->type = EffectsRequest::DISABLE_EFFECT_CHAIN_FOR_INPUT_CHANNEL;
        request->pTargetChain = m_pEngineEffectChain;
        request->DisableInputChannelForChain.pChannelHandle = &handleGroup.handle();
        m_pEffectsManager->writeRequest(request);

        emit channelStatusChanged(handleGroup.name(), false);
    }
}

double EffectChain::mix() const {
    return m_dMix;
}

void EffectChain::setMix(const double& dMix) {
    m_dMix = dMix;
    sendParameterUpdate();
    emit mixChanged(dMix);
}

EffectChainMixMode EffectChain::mixMode() const {
    return m_mixMode;
}

void EffectChain::setMixMode(EffectChainMixMode mixMode) {
    m_mixMode = mixMode;
    sendParameterUpdate();
    emit mixModeChanged(mixMode);
}

void EffectChain::addEffect(EffectPointer pEffect) {
    //qDebug() << debugString() << "addEffect" << pEffect;
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
    if (m_bAddedToEngine) {
        pEffect->addToEngine(m_pEngineEffectChain, m_effects.size() - 1, m_enabledInputChannels);
    }
    emit effectChanged(m_effects.size() - 1);
}

void EffectChain::replaceEffect(unsigned int effectSlotNumber,
                                EffectPointer pEffect) {
    //qDebug() << debugString() << "replaceEffect" << effectSlotNumber << pEffect;
    while (effectSlotNumber >= static_cast<unsigned int>(m_effects.size())) {
        if (pEffect.isNull()) {
            return;
        }
        m_effects.append(EffectPointer());
    }

    EffectPointer pOldEffect = m_effects[effectSlotNumber];
    if (!pOldEffect.isNull()) {
        if (m_bAddedToEngine) {
            pOldEffect->removeFromEngine(m_pEngineEffectChain, effectSlotNumber);
        }
    }

    m_effects.replace(effectSlotNumber, pEffect);
    if (!pEffect.isNull()) {
        if (m_bAddedToEngine) {
            pEffect->addToEngine(m_pEngineEffectChain, effectSlotNumber, m_enabledInputChannels);
        }
    }

    emit effectChanged(effectSlotNumber);
}

void EffectChain::removeEffect(unsigned int effectSlotNumber) {
    replaceEffect(effectSlotNumber, EffectPointer());
}

void EffectChain::refreshAllEffects() {
    for (int i = 0; i < m_effects.size(); ++i) {
        emit effectChanged(i);
    }
}

unsigned int EffectChain::numEffects() const {
    return m_effects.size();
}

const QList<EffectPointer>& EffectChain::effects() const {
    return m_effects;
}

EngineEffectChain* EffectChain::getEngineEffectChain() {
    return m_pEngineEffectChain;
}

void EffectChain::sendParameterUpdate() {
    if (!m_bAddedToEngine) {
        return;
    }
    EffectsRequest* pRequest = new EffectsRequest();
    pRequest->type = EffectsRequest::SET_EFFECT_CHAIN_PARAMETERS;
    pRequest->pTargetChain = m_pEngineEffectChain;
    pRequest->SetEffectChainParameters.enabled = m_bEnabled;
    pRequest->SetEffectChainParameters.mix_mode = m_mixMode;
    pRequest->SetEffectChainParameters.mix = m_dMix;
    m_pEffectsManager->writeRequest(pRequest);
}

// static
EffectChainPointer EffectChain::createFromXml(EffectsManager* pEffectsManager,
                                        const QDomElement& element) {
    if (!element.hasChildNodes()) {
        // An empty element <EffectChain/> is treated as an ejected Chain (null)
        return EffectChainPointer();
    }

    QString id = XmlParse::selectNodeQString(element,
                                             EffectXml::ChainId);
    QString name = XmlParse::selectNodeQString(element,
                                               EffectXml::ChainName);
    QString description = XmlParse::selectNodeQString(element,
                                                      EffectXml::ChainDescription);
    QString mixModeStr = XmlParse::selectNodeQString(element,
                                                           EffectXml::ChainMixMode);

    EffectChainPointer pChain(new EffectChain(pEffectsManager, id));
    pChain->setName(name);
    pChain->setDescription(description);
    EffectChainMixMode mixMode = mixModeFromString(mixModeStr);
    if (mixMode < EffectChainMixMode::NumMixModes) {
        pChain->setMixMode(mixMode);
    }

    QDomElement effects = XmlParse::selectElement(element, EffectXml::EffectsRoot);
    QDomNodeList effectChildren = effects.childNodes();

    for (int i = 0; i < effectChildren.count(); ++i) {
        QDomNode effect = effectChildren.at(i);
        if (effect.isElement()) {
            EffectPointer pEffect = Effect::createFromXml(
                pEffectsManager, effect.toElement());
            pChain->addEffect(pEffect);
        }
    }

    return pChain;
}
