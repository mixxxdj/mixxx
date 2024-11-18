#include "effects/effectslot.h"

#include <QDebug>
#include <QPoint>
#include <optional>

#include "control/controlencoder.h"
#include "control/controlpushbutton.h"
#include "effects/backends/effectmanifest.h"
#include "effects/defs.h"
#include "effects/effectbuttonparameterslot.h"
#include "effects/effectchain.h"
#include "effects/effectknobparameterslot.h"
#include "effects/effectparameter.h"
#include "effects/effectsmanager.h"
#include "effects/effectsmessenger.h"
#include "effects/presets/effectpreset.h"
#include "effects/presets/effectpresetmanager.h"
#include "effects/visibleeffectslist.h"
#include "engine/effects/engineeffect.h"
#include "moc_effectslot.cpp"
#include "util/math.h"

// The maximum number of effect parameters we're going to support.
constexpr unsigned int kDefaultMaxParameters = 16;

EffectSlot::EffectSlot(const QString& group,
        EffectsManager* pEffectsManager,
        EffectsMessengerPointer pEffectsMessenger,
        const unsigned int iEffectnumber,
        EffectChain* pChainSlot,
        EngineEffectChain* pEngineEffectChain)
        : m_iEffectNumber(iEffectnumber),
          m_group(group),
          m_pEffectsManager(pEffectsManager),
          m_pPresetManager(pEffectsManager->getEffectPresetManager()),
          m_pBackendManager(pEffectsManager->getBackendManager()),
          m_pMessenger(pEffectsMessenger),
          m_pVisibleEffects(m_pEffectsManager->getVisibleEffectsList()),
          m_pChain(pChainSlot),
          m_pEngineEffectChain(pEngineEffectChain),
          m_pEngineEffect(nullptr) {
    VERIFY_OR_DEBUG_ASSERT(m_pEngineEffectChain) {
        return;
    }

    m_pControlLoaded = std::make_unique<ControlObject>(ConfigKey(m_group, "loaded"));
    m_pControlLoaded->setReadOnly();

    m_pControlNumParameters.insert(EffectParameterType::Knob,
            QSharedPointer<ControlObject>(
                    new ControlObject(ConfigKey(m_group, "num_parameters"))));
    m_pControlNumParameters.insert(EffectParameterType::Button,
            QSharedPointer<ControlObject>(
                    new ControlObject(ConfigKey(m_group, "num_button_parameters"))));
    for (const auto& pControlNumParameters : std::as_const(m_pControlNumParameters)) {
        pControlNumParameters->setReadOnly();
    }

    m_pControlNumParameterSlots.insert(EffectParameterType::Knob,
            QSharedPointer<ControlObject>(
                    new ControlObject(ConfigKey(m_group, "num_parameterslots"))));
    m_pControlNumParameterSlots.insert(EffectParameterType::Button,
            QSharedPointer<ControlObject>(
                    new ControlObject(ConfigKey(m_group, "num_button_parameterslots"))));
    for (const auto& pControlNumParameterSlots : std::as_const(m_pControlNumParameterSlots)) {
        pControlNumParameterSlots->setReadOnly();
    }

    // Default to disabled to prevent accidental activation of effects
    // at the beginning of a set.
    m_pControlEnabled = std::make_unique<ControlPushButton>(ConfigKey(m_group, "enabled"));
    m_pControlEnabled->setButtonMode(mixxx::control::ButtonMode::PowerWindow);
    connect(m_pControlEnabled.get(),
            &ControlObject::valueChanged,
            this,
            &EffectSlot::updateEngineState);

    m_pControlUIShown = std::make_unique<ControlPushButton>(ConfigKey(m_group, "ui_shown"));
    m_pControlUIShown->setButtonMode(mixxx::control::ButtonMode::PowerWindow);
    connect(m_pControlUIShown.get(),
            &ControlObject::valueChanged,
            this,
            &EffectSlot::updateEffectUI);

    connect(this,
            &EffectSlot::effectChanged,
            this,
            &EffectSlot::updateEffectUI);

    m_pControlUIButtonShown = std::make_unique<ControlObject>(ConfigKey(m_group, "uibutton_shown"));
    m_pControlUIButtonShown->set(false);

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

    m_pControlLoadedEffect = std::make_unique<ControlObject>(
            ConfigKey(m_group, "loaded_effect"));
    m_pControlLoadedEffect->connectValueChangeRequest(
            this,
            &EffectSlot::slotLoadedEffectRequest);

    connect(m_pVisibleEffects.get(),
            &VisibleEffectsList::visibleEffectsListChanged,
            this,
            &EffectSlot::visibleEffectsListChanged);

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
        addEffectParameterSlot(EffectParameterType::Knob);
        addEffectParameterSlot(EffectParameterType::Button);
    }

    m_pControlMetaParameter = std::make_unique<ControlPotmeter>(
            ConfigKey(m_group, "meta"), 0.0, 1.0);
    // QObject::connect cannot connect to slots with optional parameters using function
    // pointer syntax if the slot has more parameters than the signal, so use a lambda
    // to hack around this limitation.
    connect(m_pControlMetaParameter.get(),
            &ControlObject::valueChanged,
            this,
            [=, this](double value) { slotEffectMetaParameter(value); });
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

    VERIFY_OR_DEBUG_ASSERT(!m_pEngineEffect) {
        return;
    }

    m_pEngineEffect = new EngineEffect(
            m_pManifest,
            m_pBackendManager,
            m_pChain->getActiveChannels(),
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
    pRequest->SetEffectParameters.enabled = m_pControlEnabled->toBool();
    m_pMessenger->writeRequest(pRequest);

    for (const auto& parameterList : std::as_const(m_allParameters)) {
        for (auto const& pParameter : parameterList) {
            pParameter->updateEngineState();
        }
    }
}

void EffectSlot::updateEffectUI() {
    bool uiShown = m_pControlUIShown->toBool();
    std::optional<QPoint> oldDlgPosition;

    if (m_pEffectUI) {
        // Carry over the screen position of the dialog to keep the user's
        // window layout when switching to another effect.
        oldDlgPosition = m_pEffectUI->pos();

        // Prevent close from retriggering updateControlOnEffectUIClose if we
        // want to keep showing a UI (e.g. because the effect was switched).
        m_pEffectUI->setClosesWithoutSignal(uiShown);
        m_pEffectUI->close();
    }

    if (uiShown && m_pEngineEffect) {
        m_pEffectUI = m_pEngineEffect->createUI();
        m_pEffectUI->show();
        connect(&*m_pEffectUI,
                &DlgEffect::closed,
                this,
                &EffectSlot::updateControlOnEffectUIClose);

        if (oldDlgPosition.has_value()) {
            m_pEffectUI->move(oldDlgPosition->x(), oldDlgPosition->y());
        }
    } else {
        m_pEffectUI = nullptr;
    }

    m_pControlUIButtonShown->set(m_pManifest ? m_pManifest->hasUI() : false);
}

void EffectSlot::updateControlOnEffectUIClose() {
    m_pControlUIShown->set(false);
}

void EffectSlot::initalizeInputChannel(ChannelHandle inputChannel) {
    if (!m_pEngineEffect) {
        return;
    }
    m_pEngineEffect->initalizeInputChannel(inputChannel);
};

EffectManifestPointer EffectSlot::getManifest() const {
    return m_pManifest;
}

void EffectSlot::addEffectParameterSlot(EffectParameterType parameterType) {
    EffectParameterSlotBasePointer pParameterSlot =
            EffectParameterSlotBasePointer();
    if (parameterType == EffectParameterType::Knob) {
        pParameterSlot = QSharedPointer<EffectKnobParameterSlot>::create(
                m_group, m_iNumParameterSlots[parameterType]);
    } else if (parameterType == EffectParameterType::Button) {
        pParameterSlot = QSharedPointer<EffectButtonParameterSlot>::create(
                m_group, m_iNumParameterSlots[parameterType]);
    }
    ++m_iNumParameterSlots[parameterType];
    const auto pCONumParameterSlots = m_pControlNumParameterSlots[parameterType];
    pCONumParameterSlots->forceSet(pCONumParameterSlots->get() + 1);
    VERIFY_OR_DEBUG_ASSERT(m_iNumParameterSlots[parameterType] == pCONumParameterSlots->get()) {
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
    EffectManifestPointer pManifest;
    if (pPreset && !pPreset->isEmpty()) {
        pManifest = m_pBackendManager->getManifest(pPreset);
    }
    if (!pManifest) {
        loadEffectInner(nullptr, nullptr, true);
        return;
    }
    loadEffectInner(pManifest, pPreset, true);
}

void EffectSlot::loadEffectWithDefaults(const EffectManifestPointer pManifest) {
    EffectPresetPointer pPreset = m_pPresetManager->getDefaultPreset(pManifest);
    loadEffectInner(pManifest, pPreset, false);
}

void EffectSlot::loadEffectInner(const EffectManifestPointer pManifest,
        EffectPresetPointer pEffectPreset,
        bool adoptMetaknobFromPreset) {
    if (kEffectDebugOutput) {
        if (pManifest) {
            qDebug() << this << m_group << "loading effect" << pManifest->id();
        } else {
            qDebug() << this << m_group << "unloading effect";
        }
    }
    unloadEffect();
    DEBUG_ASSERT(!m_pManifest);

    // The function shall be called only with both pointers set or both null.
    DEBUG_ASSERT(pManifest.isNull() == pEffectPreset.isNull());
    if (!pManifest || !pEffectPreset) {
        // No new effect to load; just unload the old effect and return.
        emit effectChanged();
        return;
    }

    // Don't load an effect into the '---' preset. The preset would remain
    // selected in WEffectChainPresetSelector and WEffectChainPresetButton and
    // therefore couldn't be used to clear the chain.
    // Instead, load an empty, nameless preset, then load the desired effect.
    if (m_pChain->isEmptyPlaceholderPresetLoaded()) {
        m_pChain->loadEmptyNamelessPreset();
    }

    m_pManifest = pManifest;
    addToEngine();

    // Create EffectParameters. Every parameter listed in the manifest must have
    // an EffectParameter created, regardless of whether it is loaded in a slot.
    for (const auto& pManifestParameter : m_pManifest->parameters()) {
        // match the manifest parameter to the preset parameter
        EffectParameterPreset parameterPreset;
        if (pEffectPreset) {
            for (const auto& p : pEffectPreset->getParameterPresets()) {
                if (p.id() == pManifestParameter->id()) {
                    parameterPreset = p;
                    break;
                }
            }
        }
        EffectParameterPointer pParameter(new EffectParameter(
                m_pEngineEffect,
                m_pMessenger,
                pManifestParameter,
                parameterPreset));
        m_allParameters[pManifestParameter->parameterType()].append(pParameter);
    }

    // Map the parameter slots to the EffectParameters.
    // The slot order is determined by the order parameters are listed in the preset.
    int numTypes = static_cast<int>(EffectParameterType::NumTypes);
    for (int parameterTypeId = 0; parameterTypeId < numTypes;
            ++parameterTypeId) {
        const EffectParameterType parameterType =
                static_cast<EffectParameterType>(parameterTypeId);

        if (pEffectPreset && !pEffectPreset.isNull()) {
            m_loadedParameters[parameterType].clear();
            for (const auto& parameterPreset :
                    pEffectPreset->getParameterPresets()) {
                if (parameterPreset.hidden() || parameterPreset.isNull()) {
                    continue;
                }

                const auto& allParameters = m_allParameters.value(parameterType);
                for (const auto& pParameter : allParameters) {
                    if (pParameter->manifest()->id() == parameterPreset.id()) {
                        m_loadedParameters[parameterType].append(pParameter);
                        break;
                    }
                }
            }
        }
    }

    loadParameters();

    m_pControlMetaParameter->setDefaultValue(pManifest->metaknobDefault());

    m_pControlLoaded->forceSet(1.0);

    if (m_pEffectsManager->isAdoptMetaknobSettingEnabled()) {
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

    // ControlObjects are 1-indexed
    m_pControlLoadedEffect->setAndConfirm(m_pVisibleEffects->indexOf(pManifest) + 1);

    emit effectChanged();
    updateEngineState();
}

void EffectSlot::unloadEffect() {
    if (!isLoaded()) {
        return;
    }

    m_pControlLoaded->forceSet(0.0);
    m_pControlLoadedEffect->setAndConfirm(0.0);
    for (const auto& pControlNumParameters : std::as_const(m_pControlNumParameters)) {
        pControlNumParameters->forceSet(0.0);
    }

    for (const auto& slotList : std::as_const(m_parameterSlots)) {
        // Do not delete the slots; clear the parameters from the slots
        // The parameter slots are used by the next effect, but the EffectParameters
        // are deleted below.
        for (auto pSlot : std::as_const(slotList)) {
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

    m_pControlMetaParameter->setDefaultValue(0.0);

    m_pManifest.clear();

    removeFromEngine();
}

void EffectSlot::loadParameters() {
    //qDebug() << this << m_group << "loading parameters";
    int numTypes = static_cast<int>(EffectParameterType::NumTypes);
    for (int parameterTypeId = 0; parameterTypeId < numTypes;
            ++parameterTypeId) {
        const EffectParameterType parameterType =
                static_cast<EffectParameterType>(parameterTypeId);

        m_pControlNumParameters[parameterType]->forceSet(
                numParameters(parameterType));

        int slot = 0;
        const auto& loadedParameters = m_loadedParameters.value(parameterType);
        for (const auto& pParameter : loadedParameters) {
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
        const auto& allParameters = m_allParameters.value(parameterType);
        for (const auto& pParameter : allParameters) {
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
    if (index1 == index2) {
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(m_loadedParameters[type].size() > index1) {
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(m_loadedParameters[type].size() > index2) {
        return;
    }
#if QT_VERSION >= QT_VERSION_CHECK(5, 13, 0)
    m_loadedParameters[type].swapItemsAt(index1, index2);
#else
    m_loadedParameters[type].swap(index1, index2);
#endif
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

void EffectSlot::slotLoadedEffectRequest(double value) {
    // ControlObjects are 1-indexed
    int index = static_cast<int>(value) - 1;
    if (index < 0 || index >= m_pVisibleEffects->getList().size()) {
        return;
    }
    // loadEffectInner calls setAndConfirm
    loadEffectWithDefaults(m_pVisibleEffects->at(index));
}

void EffectSlot::visibleEffectsListChanged() {
    if (isLoaded()) {
        // ControlObjects are 1-indexed
        m_pControlLoadedEffect->setAndConfirm(
                m_pVisibleEffects->indexOf(m_pManifest) + 1);
    }
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
    for (const auto& parameterSlotList : std::as_const(m_parameterSlots)) {
        for (const auto& pParameterSlot : std::as_const(parameterSlotList)) {
            if (pParameterSlot->parameterType() == EffectParameterType::Knob) {
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
    if (!m_metaknobSoftTakeover.ignore(*m_pControlMetaParameter, v) ||
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
    const auto& knobParameters = m_parameterSlots.value(EffectParameterType::Knob);
    for (const auto& pParameterSlot : std::as_const(knobParameters)) {
        if (pParameterSlot->parameterType() == EffectParameterType::Knob) {
            pParameterSlot->onEffectMetaParameterChanged(v, force);
        }
    }
}
