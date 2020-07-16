#include "effects/effectslot.h"
#include "effects/effectxmlelements.h"

#include <QDebug>

#include "control/controlencoder.h"
#include "control/controlproxy.h"
#include "control/controlpushbutton.h"
#include "effects/defs.h"
#include "effects/effectsmessenger.h"
#include "effects/presets/effectpresetmanager.h"
#include "effects/visibleeffectslist.h"
#include "util/defs.h"
#include "util/math.h"

// The maximum number of effect parameters we're going to support.
const unsigned int kDefaultMaxParameters = 16;

EffectSlot::EffectSlot(const QString& group,
        EffectsManager* pEffectsManager,
        EffectsMessengerPointer pEffectsMessenger,
        const unsigned int iEffectnumber,
        EffectChainSlot* pChainSlot,
        EngineEffectChain* pEngineEffectChain)
        : m_iEffectNumber(iEffectnumber),
          m_group(group),
          m_pEffectsManager(pEffectsManager),
          m_pPresetManager(pEffectsManager->getEffectPresetManager()),
          m_pBackendManager(pEffectsManager->getBackendManager()),
          m_pMessenger(pEffectsMessenger),
          m_pVisibleEffects(m_pEffectsManager->getVisibleEffectsList()),
          m_pChainSlot(pChainSlot),
          m_pEngineEffectChain(pEngineEffectChain),
          m_pEngineEffect(nullptr) {
    VERIFY_OR_DEBUG_ASSERT(m_pEngineEffectChain != nullptr) {
        return;
    }

    m_pControlLoaded = std::make_unique<ControlObject>(ConfigKey(m_group, "loaded"));
    m_pControlLoaded->setReadOnly();

    m_pControlNumParameters.insert(EffectParameterType::KNOB,
            QSharedPointer<ControlObject>(
                    new ControlObject(ConfigKey(m_group, "num_parameters"))));
    m_pControlNumParameters.insert(EffectParameterType::BUTTON,
            QSharedPointer<ControlObject>(
                    new ControlObject(ConfigKey(m_group, "num_button_parameters"))));
    for (const auto& pControlNumParameters : m_pControlNumParameters) {
        pControlNumParameters->setReadOnly();
    }

    m_pControlNumParameterSlots.insert(EffectParameterType::KNOB,
            QSharedPointer<ControlObject>(
                    new ControlObject(ConfigKey(m_group, "num_parameterslots"))));
    m_pControlNumParameterSlots.insert(EffectParameterType::BUTTON,
            QSharedPointer<ControlObject>(
                    new ControlObject(ConfigKey(m_group, "num_button_parameterslots"))));
    for (const auto& pControlNumParameterSlots : m_pControlNumParameterSlots) {
        pControlNumParameterSlots->setReadOnly();
    }

    // Default to disabled to prevent accidental activation of effects
    // at the beginning of a set.
    m_pControlEnabled = std::make_unique<ControlPushButton>(ConfigKey(m_group, "enabled"));
    m_pControlEnabled->setButtonMode(ControlPushButton::POWERWINDOW);
    connect(m_pControlEnabled.get(),
            &ControlObject::valueChanged,
            this,
            &EffectSlot::updateEngineState);

    m_pControlNextEffect = std::make_unique<ControlPushButton>(
            ConfigKey(m_group, "next_effect"));
    connect(m_pControlNextEffect.get(),
            &ControlObject::valueChanged,
            this,
            &EffectSlot::slotNextEffect);

    m_pControlPrevEffect = std::make_unique<ControlPushButton>(
            ConfigKey(m_group, "prev_effect"));
    connect(m_pControlPrevEffect.get(),
            &ControlObject::valueChanged,
            this,
            &EffectSlot::slotPrevEffect);

    m_pControlLoadEffectAtListIndex = std::make_unique<ControlObject>(
            ConfigKey(m_group, "load_effect"));
    connect(m_pControlLoadEffectAtListIndex.get(),
            &ControlObject::valueChanged,
            this,
            &EffectSlot::slotLoadEffectAtListIndex);

    // Ignoring no-ops is important since this is for +/- tickers.
    m_pControlEffectSelector = std::make_unique<ControlEncoder>(
            ConfigKey(m_group, "effect_selector"), false);
    connect(m_pControlEffectSelector.get(),
            &ControlObject::valueChanged,
            this,
            &EffectSlot::slotEffectSelector);

    m_pControlClear =
            std::make_unique<ControlPushButton>(ConfigKey(m_group, "clear"));
    connect(m_pControlClear.get(),
            &ControlObject::valueChanged,
            this,
            &EffectSlot::slotClear);

    for (unsigned int i = 0; i < kDefaultMaxParameters; ++i) {
        addEffectParameterSlot(EffectParameterType::KNOB);
        addEffectParameterSlot(EffectParameterType::BUTTON);
    }

    m_pControlMetaParameter = std::make_unique<ControlPotmeter>(
            ConfigKey(m_group, "meta"), 0.0, 1.0);
    // QObject::connect cannot connect to slots with optional parameters using function
    // pointer syntax if the slot has more parameters than the signal, so use a lambda
    // to hack around this limitation.
    connect(m_pControlMetaParameter.get(),
            &ControlObject::valueChanged,
            this,
            [=](double value) { slotEffectMetaParameter(value); });
    m_pControlMetaParameter->set(0.0);
    m_pControlMetaParameter->setDefaultValue(0.0);

    m_pControlLoaded->forceSet(0.0);
}

EffectSlot::~EffectSlot() {
    //qDebug() << debugString() << "destroyed";
    unloadEffect();
}

void EffectSlot::addToEngine() {
    VERIFY_OR_DEBUG_ASSERT(!isLoaded()) {
        return;
    }

    VERIFY_OR_DEBUG_ASSERT(m_pEngineEffect == nullptr) {
        return;
    }

    m_pEngineEffect = new EngineEffect(m_pManifest,
            m_pBackendManager,
            m_pChainSlot->getActiveChannels(),
            m_pEffectsManager->registeredInputChannels(),
            m_pEffectsManager->registeredOutputChannels());

    EffectsRequest* request = new EffectsRequest();
    request->type = EffectsRequest::ADD_EFFECT_TO_CHAIN;
    request->pTargetChain = m_pEngineEffectChain;
    request->AddEffectToChain.pEffect = m_pEngineEffect;
    request->AddEffectToChain.iIndex = m_iEffectNumber;
    m_pMessenger->writeRequest(request);
}

void EffectSlot::removeFromEngine() {
    VERIFY_OR_DEBUG_ASSERT(isLoaded()) {
        return;
    }

    EffectsRequest* request = new EffectsRequest();
    request->type = EffectsRequest::REMOVE_EFFECT_FROM_CHAIN;
    request->pTargetChain = m_pEngineEffectChain;
    request->RemoveEffectFromChain.pEffect = m_pEngineEffect;
    request->RemoveEffectFromChain.iIndex = m_iEffectNumber;
    m_pMessenger->writeRequest(request);

    m_pEngineEffect = nullptr;
}

void EffectSlot::updateEngineState() {
    if (!m_pEngineEffect) {
        return;
    }

    EffectsRequest* pRequest = new EffectsRequest();
    pRequest->type = EffectsRequest::SET_EFFECT_PARAMETERS;
    pRequest->pTargetEffect = m_pEngineEffect;
    pRequest->SetEffectParameters.enabled = m_pControlEnabled->get();
    m_pMessenger->writeRequest(pRequest);

    for (const auto& parameterList : m_allParameters) {
        for (auto const& pParameter : parameterList) {
            pParameter->updateEngineState();
        }
    }
}

void EffectSlot::fillEffectStatesMap(EffectStatesMap* pStatesMap) const {
    //TODO: get actual configuration of engine
    const mixxx::EngineParameters bufferParameters(
            mixxx::AudioSignal::SampleRate(96000),
            MAX_BUFFER_LEN / mixxx::kEngineChannelCount);

    if (isLoaded()) {
        for (const auto& outputChannel :
                m_pEffectsManager->registeredOutputChannels()) {
            pStatesMap->insert(outputChannel.handle(),
                    m_pEngineEffect->createState(bufferParameters));
        }
    } else {
        for (EffectState* pState : *pStatesMap) {
            if (pState != nullptr) {
                delete pState;
            }
        }
        pStatesMap->clear();
    }
};

EffectManifestPointer EffectSlot::getManifest() const {
    return m_pManifest;
}

void EffectSlot::addEffectParameterSlot(EffectParameterType parameterType) {
    EffectParameterSlotBasePointer pParameterSlot =
            EffectParameterSlotBasePointer();
    if (parameterType == EffectParameterType::KNOB) {
        pParameterSlot = static_cast<EffectParameterSlotBasePointer>(
                new EffectKnobParameterSlot(
                        m_group, m_iNumParameterSlots[parameterType]));
    } else if (parameterType == EffectParameterType::BUTTON) {
        pParameterSlot = static_cast<EffectParameterSlotBasePointer>(
                new EffectButtonParameterSlot(
                        m_group, m_iNumParameterSlots[parameterType]));
    }
    ++m_iNumParameterSlots[parameterType];
    m_pControlNumParameterSlots[parameterType]->forceSet(
            m_pControlNumParameterSlots[parameterType]->get() + 1);
    VERIFY_OR_DEBUG_ASSERT(m_iNumParameterSlots[parameterType] ==
            m_pControlNumParameterSlots[parameterType]->get()) {
        return;
    }
    m_parameterSlots[parameterType].append(pParameterSlot);
}

unsigned int EffectSlot::numParameters(
        EffectParameterType parameterType) const {
    return m_allParameters.value(parameterType).size();
}

void EffectSlot::setEnabled(bool enabled) {
    m_pControlEnabled->set(enabled);
}

EffectParameterSlotBasePointer EffectSlot::getEffectParameterSlot(
        EffectParameterType parameterType, unsigned int slotNumber) {
    VERIFY_OR_DEBUG_ASSERT(slotNumber <=
            (unsigned)m_parameterSlots.value(parameterType).size()) {
        return nullptr;
    }
    return m_parameterSlots.value(parameterType).at(slotNumber);
}

void EffectSlot::loadEffectFromPreset(const EffectPresetPointer pPreset) {
    if (pPreset == nullptr) {
        loadEffectInner(nullptr, nullptr, true);
        return;
    }
    EffectManifestPointer pManifest = m_pBackendManager->getManifest(
            pPreset->id(), pPreset->backendType());
    loadEffectInner(pManifest, pPreset, true);
}

void EffectSlot::loadEffectWithDefaults(const EffectManifestPointer pManifest) {
    if (pManifest == nullptr) {
        loadEffectInner(nullptr, nullptr, false);
        return;
    }
    EffectPresetPointer pPreset = m_pPresetManager->getDefaultPreset(pManifest);
    loadEffectInner(pManifest, pPreset, false);
}

void EffectSlot::loadEffectInner(const EffectManifestPointer pManifest,
        EffectPresetPointer pEffectPreset,
        bool adoptMetaknobFromPreset) {
    if (kEffectDebugOutput) {
        if (pManifest != nullptr) {
            qDebug() << this << m_group << "loading effect" << pManifest->id();
        } else {
            qDebug() << this << m_group << "unloading effect";
        }
    }
    unloadEffect();

    m_pManifest = pManifest;

    if (pManifest == nullptr || pEffectPreset == nullptr) {
        // No new effect to load; just unload the old effect and return.
        emit effectChanged();
        return;
    }

    addToEngine();

    // Create EffectParameters. Every parameter listed in the manifest must have
    // an EffectParameter created, regardless of whether it is loaded in a slot.
    for (const auto& pManifestParameter : m_pManifest->parameters()) {
        // match the manifest parameter to the preset parameter
        EffectParameterPreset parameterPreset = EffectParameterPreset();
        if (pEffectPreset != nullptr) {
            for (const auto& p : pEffectPreset->getParameterPresets()) {
                if (p.id() == pManifestParameter->id()) {
                    parameterPreset = p;
                }
            }
        }
        EffectParameterPointer pParameter(new EffectParameter(m_pEngineEffect,
                m_pMessenger,
                pManifestParameter,
                parameterPreset));
        m_allParameters[pManifestParameter->parameterType()].append(pParameter);
    }

    // Map the parameter slots to the EffectParameters.
    // The slot order is determined by the order parameters are listed in the preset.
    int numTypes = static_cast<int>(EffectParameterType::NUM_TYPES);
    for (int parameterTypeId = 0; parameterTypeId < numTypes;
            ++parameterTypeId) {
        const EffectParameterType parameterType =
                static_cast<EffectParameterType>(parameterTypeId);

        if (pEffectPreset != nullptr && !pEffectPreset.isNull()) {
            m_loadedParameters[parameterType].clear();
            int slot = 0;
            for (const auto& parameterPreset :
                    pEffectPreset->getParameterPresets()) {
                if (parameterPreset.hidden() || parameterPreset.isNull()) {
                    continue;
                }

                for (const auto& pParameter :
                        m_allParameters.value(parameterType)) {
                    if (pParameter->manifest()->id() == parameterPreset.id()) {
                        m_loadedParameters[parameterType].insert(
                                slot, pParameter);
                        break;
                    }
                }
                slot++;
            }
        }
    }

    loadParameters();

    m_pControlLoaded->forceSet(1.0);

    if (m_pEffectsManager->isAdoptMetaknobValueEnabled()) {
        if (adoptMetaknobFromPreset) {
            // Update the ControlObject value, but do not sync the parameters
            // with slotEffectMetaParameter. This allows presets to intentionally
            // save parameters in a state inconsistent with the metaknob.
            m_pControlMetaParameter->set(pEffectPreset->metaParameter());
        } else {
            slotEffectMetaParameter(m_pControlMetaParameter->get(), true);
        }
    } else {
        m_pControlMetaParameter->set(pEffectPreset->metaParameter());
        slotEffectMetaParameter(pEffectPreset->metaParameter(), true);
    }

    emit effectChanged();
    updateEngineState();
}

void EffectSlot::unloadEffect() {
    if (!isLoaded()) {
        return;
    }

    m_pControlLoaded->forceSet(0.0);
    for (const auto& pControlNumParameters : m_pControlNumParameters) {
        pControlNumParameters->forceSet(0.0);
    }

    for (auto& slotList : m_parameterSlots) {
        // Do not delete the slots; clear the parameters from the slots
        // The parameter slots are used by the next effect, but the EffectParameters
        // are deleted below.
        for (auto pSlot : slotList) {
            pSlot->clear();
        }
    }
    for (auto& parameterList : m_allParameters) {
        parameterList.clear();
    }
    for (auto& parameterList : m_loadedParameters) {
        parameterList.clear();
    }
    for (auto& parameterList : m_hiddenParameters) {
        parameterList.clear();
    }

    m_pManifest.clear();

    removeFromEngine();
}

void EffectSlot::loadParameters() {
    //qDebug() << this << m_group << "loading parameters";
    int numTypes = static_cast<int>(EffectParameterType::NUM_TYPES);
    for (int parameterTypeId = 0; parameterTypeId < numTypes;
            ++parameterTypeId) {
        const EffectParameterType parameterType =
                static_cast<EffectParameterType>(parameterTypeId);

        m_pControlNumParameters[parameterType]->forceSet(
                numParameters(parameterType));

        int slot = 0;
        for (auto pParameter : m_loadedParameters.value(parameterType)) {
            // LV2 effects may have more parameters than there are slots available
            if ((unsigned)slot >= kDefaultMaxParameters) {
                return;
            }
            VERIFY_OR_DEBUG_ASSERT(
                    slot <= m_parameterSlots.value(parameterType).size()) {
                break;
            }
            m_parameterSlots.value(parameterType)
                    .at(slot)
                    ->loadParameter(pParameter);
            slot++;
        }

        // Clear any EffectParameterSlots that still have a loaded parameter from before
        // but the loop above did not load a new parameter into them.
        for (; slot < m_parameterSlots.value(parameterType).size(); slot++) {
            m_parameterSlots.value(parameterType).at(slot)->clear();
        }

        m_hiddenParameters[parameterType].clear();
        for (auto pParameter : m_allParameters.value(parameterType)) {
            if (!m_loadedParameters.value(parameterType).contains(pParameter)) {
                m_hiddenParameters[parameterType].append(pParameter);
            }
        }
    }
}

void EffectSlot::hideParameter(EffectParameterPointer pParameter) {
    auto parameterType = pParameter->manifest()->parameterType();
    VERIFY_OR_DEBUG_ASSERT(
            m_allParameters.value(parameterType).contains(pParameter)) {
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(
            !m_hiddenParameters.value(parameterType).contains(pParameter)) {
        return;
    }
    m_loadedParameters[parameterType].removeAll(pParameter);
    loadParameters();
    emit parametersChanged();
}

void EffectSlot::showParameter(EffectParameterPointer pParameter) {
    auto parameterType = pParameter->manifest()->parameterType();
    VERIFY_OR_DEBUG_ASSERT(
            m_allParameters.value(parameterType).contains(pParameter)) {
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(
            !m_loadedParameters.value(parameterType).contains(pParameter)) {
        return;
    }
    m_loadedParameters[parameterType].append(pParameter);
    loadParameters();
    emit parametersChanged();
}

void EffectSlot::swapParameters(EffectParameterType type, int index1, int index2) {
    VERIFY_OR_DEBUG_ASSERT(m_loadedParameters[type].size() > index1) {
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(m_loadedParameters[type].size() > index2) {
        return;
    }
    m_loadedParameters[type].swapItemsAt(index1, index2);
    loadParameters();
    emit parametersChanged();
}

void EffectSlot::slotPrevEffect(double v) {
    if (v > 0) {
        loadEffectWithDefaults(m_pVisibleEffects->previous(m_pManifest));
    }
}

void EffectSlot::slotNextEffect(double v) {
    if (v > 0) {
        loadEffectWithDefaults(m_pVisibleEffects->next(m_pManifest));
    }
}

void EffectSlot::slotLoadEffectAtListIndex(double value) {
    // ControlObjects are 1-indexed
    loadEffectWithDefaults(m_pVisibleEffects->at(value - 1));
}

void EffectSlot::slotEffectSelector(double v) {
    if (v > 0) {
        loadEffectWithDefaults(m_pVisibleEffects->next(m_pManifest));
    } else if (v < 0) {
        loadEffectWithDefaults(m_pVisibleEffects->previous(m_pManifest));
    }
}

void EffectSlot::slotClear(double v) {
    if (v > 0) {
        unloadEffect();
        emit effectChanged();
    }
}

void EffectSlot::syncSofttakeover() {
    for (const auto& parameterSlotList : m_parameterSlots) {
        for (const auto& pParameterSlot : parameterSlotList) {
            if (pParameterSlot->parameterType() == EffectParameterType::KNOB) {
                pParameterSlot->syncSofttakeover();
            }
        }
    }
}

double EffectSlot::getMetaParameter() const {
    return m_pControlMetaParameter->get();
}

// This function is for the superknob to update individual effects' meta knobs
// slotEffectMetaParameter does not need to update m_pControlMetaParameter's value
void EffectSlot::setMetaParameter(double v, bool force) {
    if (!m_metaknobSoftTakeover.ignore(m_pControlMetaParameter.get(), v) ||
            !m_pControlEnabled->toBool() || force) {
        m_pControlMetaParameter->set(v);
        slotEffectMetaParameter(v, force);
    }
}

void EffectSlot::slotEffectMetaParameter(double v, bool force) {
    // Clamp to [0.0, 1.0]
    if (v < 0.0 || v > 1.0) {
        qWarning() << debugString() << "value out of limits";
        v = math_clamp(v, 0.0, 1.0);
        m_pControlMetaParameter->set(v);
    }
    if (!m_pControlEnabled->toBool()) {
        force = true;
    }

    // Only knobs are linked to the metaknob; not buttons
    const auto& knobParameters = m_parameterSlots.value(EffectParameterType::KNOB);
    for (const auto& pParameterSlot : knobParameters) {
        if (pParameterSlot->parameterType() == EffectParameterType::KNOB) {
            pParameterSlot->onEffectMetaParameterChanged(v, force);
        }
    }
}
