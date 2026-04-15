#include "effects/effectchain.h"

#include "control/controlencoder.h"
#include "control/controlpotmeter.h"
#include "control/controlpushbutton.h"
#include "effects/effectslot.h"
#include "effects/effectsmanager.h"
#include "effects/effectsmessenger.h"
#include "effects/presets/effectchainpreset.h"
#include "effects/presets/effectchainpresetmanager.h"
#include "engine/effects/engineeffectchain.h"
#include "moc_effectchain.cpp"
#include "util/sample.h"

EffectChain::EffectChain(const QString& group,
        EffectsManager* pEffectsManager,
        EffectsMessengerPointer pEffectsMessenger,
        SignalProcessingStage stage)
        : // The control group names are 1-indexed while internally everything
          // is 0-indexed.
          m_presetName(""),
          m_pEffectsManager(pEffectsManager),
          m_pChainPresetManager(pEffectsManager->getChainPresetManager()),
          m_pMessenger(pEffectsMessenger),
          m_group(group),
          m_signalProcessingStage(stage),
          m_pEngineEffectChain(nullptr) {
    // qDebug() << "EffectChain::EffectChain " << group << ' ' << iChainNumber;

    m_pControlClear = std::make_unique<ControlPushButton>(ConfigKey(m_group, "clear"));
    connect(m_pControlClear.get(),
            &ControlObject::valueChanged,
            this,
            &EffectChain::slotControlClear);

    m_pControlNumEffectSlots = std::make_unique<ControlObject>(
            ConfigKey(m_group, "num_effectslots"));
    m_pControlNumEffectSlots->setReadOnly();

    m_pControlNumChainPresets = std::make_unique<ControlObject>(
            ConfigKey(m_group, "num_chain_presets"));
    m_pControlNumChainPresets->set(m_pChainPresetManager->numPresets());
    m_pControlNumChainPresets->setReadOnly();
    connect(m_pChainPresetManager.data(),
            &EffectChainPresetManager::effectChainPresetRenamed,
            this,
            &EffectChain::slotEffectChainPresetRenamed);
    connect(m_pChainPresetManager.data(),
            &EffectChainPresetManager::effectChainPresetListUpdated,
            this,
            &EffectChain::slotPresetListUpdated);

    m_pControlChainEnabled =
            std::make_unique<ControlPushButton>(ConfigKey(m_group, "enabled"));
    m_pControlChainEnabled->setButtonMode(mixxx::control::ButtonMode::PowerWindow);
    // Default to enabled. The skin might not show these buttons.
    m_pControlChainEnabled->setDefaultValue(true);
    m_pControlChainEnabled->set(true);
    connect(m_pControlChainEnabled.get(),
            &ControlObject::valueChanged,
            this,
            &EffectChain::sendParameterUpdate);

    m_pControlChainMix = std::make_unique<ControlPotmeter>(
            ConfigKey(m_group, "mix"), 0.0, 1.0, false, true, false, true, 1.0);
    m_pControlChainMix->setDefaultValue(0.0);
    connect(m_pControlChainMix.get(),
            &ControlObject::valueChanged,
            this,
            &EffectChain::sendParameterUpdate);

    m_pControlChainSuperParameter = std::make_unique<ControlPotmeter>(
            ConfigKey(m_group, "super1"), 0.0, 1.0);
    // QObject::connect cannot connect to slots with optional parameters using function
    // pointer syntax if the slot has more parameters than the signal, so use a lambda
    // to hack around this limitation.
    connect(m_pControlChainSuperParameter.get(),
            &ControlObject::valueChanged,
            this,
            [=, this](double value) { slotControlChainSuperParameter(value, false); });
    m_pControlChainSuperParameter->setDefaultValue(0.0);

    m_pControlChainMixMode =
            std::make_unique<ControlPushButton>(ConfigKey(m_group, "mix_mode"));
    m_pControlChainMixMode->setBehavior(
            mixxx::control::ButtonMode::Toggle, EffectChainMixMode::kNumModes);
    double mixModeCODefault = static_cast<double>(EffectChainMixMode::DrySlashWet);
    m_pControlChainMixMode->setDefaultValue(mixModeCODefault);
    m_pControlChainMixMode->set(mixModeCODefault);
    connect(m_pControlChainMixMode.get(),
            &ControlObject::valueChanged,
            this,
            &EffectChain::sendParameterUpdate);

    m_pControlLoadedChainPreset = std::make_unique<ControlObject>(
            ConfigKey(m_group, "loaded_chain_preset"), false);
    m_pControlLoadedChainPreset->connectValueChangeRequest(
            this,
            &EffectChain::slotControlLoadedChainPresetRequest);

    m_pControlNextChainPreset = std::make_unique<ControlPushButton>(
            ConfigKey(m_group, "next_chain_preset"));
    connect(m_pControlNextChainPreset.get(),
            &ControlObject::valueChanged,
            this,
            &EffectChain::slotControlNextChainPreset);
    m_pControlNextChainPreset->addAlias(ConfigKey(m_group, QStringLiteral("next_chain")));

    m_pControlPrevChainPreset = std::make_unique<ControlPushButton>(
            ConfigKey(m_group, "prev_chain_preset"));
    connect(m_pControlPrevChainPreset.get(),
            &ControlObject::valueChanged,
            this,
            &EffectChain::slotControlPrevChainPreset);
    m_pControlPrevChainPreset->addAlias(ConfigKey(m_group, QStringLiteral("prev_chain")));

    // Ignoring no-ops is important since this is for +/- tickers.
    m_pControlChainPresetSelector = std::make_unique<ControlEncoder>(
            ConfigKey(m_group, "chain_preset_selector"), false);
    connect(m_pControlChainPresetSelector.get(),
            &ControlObject::valueChanged,
            this,
            &EffectChain::slotControlChainPresetSelector);
    m_pControlChainPresetSelector->addAlias(ConfigKey(m_group, QStringLiteral("chain_selector")));

    // ControlObjects for skin <-> controller mapping interaction.
    // Refer to comment in header for full explanation.
    m_pControlChainShowFocus = std::make_unique<ControlPushButton>(
            ConfigKey(m_group, "show_focus"));
    m_pControlChainShowFocus->setButtonMode(mixxx::control::ButtonMode::Toggle);

    m_pControlChainHasControllerFocus = std::make_unique<ControlPushButton>(
            ConfigKey(m_group, "controller_input_active"));
    m_pControlChainHasControllerFocus->setButtonMode(mixxx::control::ButtonMode::Toggle);

    m_pControlChainShowParameters = std::make_unique<ControlPushButton>(
            ConfigKey(m_group, "show_parameters"),
            true);
    m_pControlChainShowParameters->setButtonMode(mixxx::control::ButtonMode::Toggle);

    m_pControlChainFocusedEffect = std::make_unique<ControlPushButton>(
            ConfigKey(m_group, "focused_effect"),
            true);
    m_pControlChainFocusedEffect->setButtonMode(mixxx::control::ButtonMode::Toggle);

    addToEngine();
}

EffectChain::~EffectChain() {
    m_effectSlots.clear();
    removeFromEngine();
}

void EffectChain::addToEngine() {
    VERIFY_OR_DEBUG_ASSERT(!m_pEngineEffectChain) {
        return;
    }

    m_pEngineEffectChain = new EngineEffectChain(
            m_group,
            m_pEffectsManager->registeredInputChannels(),
            m_pEffectsManager->registeredOutputChannels());
    EffectsRequest* pRequest = new EffectsRequest();
    pRequest->type = EffectsRequest::ADD_EFFECT_CHAIN;
    pRequest->AddEffectChain.signalProcessingStage = m_signalProcessingStage;
    pRequest->AddEffectChain.pChain = m_pEngineEffectChain;
    m_pMessenger->writeRequest(pRequest);

    sendParameterUpdate();
}

void EffectChain::removeFromEngine() {
    VERIFY_OR_DEBUG_ASSERT(m_effectSlots.isEmpty()) {
        m_effectSlots.clear();
    }

    VERIFY_OR_DEBUG_ASSERT(m_pEngineEffectChain) {
        return;
    }

    EffectsRequest* pRequest = new EffectsRequest();
    pRequest->type = EffectsRequest::REMOVE_EFFECT_CHAIN;
    pRequest->RemoveEffectChain.signalProcessingStage = m_signalProcessingStage;
    pRequest->RemoveEffectChain.pChain = m_pEngineEffectChain;
    m_pMessenger->writeRequest(pRequest);

    m_pEngineEffectChain = nullptr;
}

const QString& EffectChain::presetName() const {
    return m_presetName;
}

void EffectChain::loadChainPreset(EffectChainPresetPointer pChainPreset) {
    slotControlClear(1);
    VERIFY_OR_DEBUG_ASSERT(pChainPreset) {
        return;
    }

    const QList effectPresets = pChainPreset->effectPresets();

    // TODO: use C++23 std::ranges::views::zip instead
    // `EffectChain`s can create arbitrary amounts of effectslots and chain presets
    // can contain an arbitrary number of effects. This ensures we only load
    // as many effects as we have slots available.
    const int validPresetSlotCount = std::min(effectPresets.count(), m_effectSlots.count());
    for (int presetSlotIndex = 0;
            presetSlotIndex < validPresetSlotCount;
            presetSlotIndex++) {
        m_effectSlots[presetSlotIndex]->loadEffectFromPreset(effectPresets[presetSlotIndex]);
    }

    setMixMode(pChainPreset->mixMode());
    m_pControlChainSuperParameter->setDefaultValue(pChainPreset->superKnob());

    m_presetName = pChainPreset->name();
    emit chainPresetChanged(m_presetName);

    setControlLoadedPresetIndex(presetIndex());
}

void EffectChain::resetToDefault() {
    m_pControlChainEnabled->set(true);
    if (m_presetName.isEmpty() || !presetIndex()) {
        // If no preset is selected, reset the super knob to a mid position, as it is by default.
        setSuperParameter(0.5, true);
        return;
    }
    setSuperParameter(m_pControlChainSuperParameter->defaultValue(), true);
}

bool EffectChain::isEmpty() {
    for (const auto& pEffectSlot : std::as_const(m_effectSlots)) {
        if (pEffectSlot->isLoaded()) {
            return false;
        }
    }
    return true;
}

bool EffectChain::isEmptyPlaceholderPresetLoaded() {
    return isEmpty() && presetName() == kNoEffectString;
}

void EffectChain::loadEmptyNamelessPreset() {
    loadChainPreset(m_pChainPresetManager->createEmptyNamelessChainPreset());
}

void EffectChain::sendParameterUpdate() {
    EffectsRequest* pRequest = new EffectsRequest();
    pRequest->type = EffectsRequest::SET_EFFECT_CHAIN_PARAMETERS;
    pRequest->pTargetChain = m_pEngineEffectChain;
    pRequest->SetEffectChainParameters.enabled = m_pControlChainEnabled->toBool();
    pRequest->SetEffectChainParameters.mix_mode = mixMode();
    pRequest->SetEffectChainParameters.mix = m_pControlChainMix->get();
    m_pMessenger->writeRequest(pRequest);
}

QString EffectChain::group() const {
    return m_group;
}

double EffectChain::getSuperParameter() const {
    return m_pControlChainSuperParameter->get();
}

void EffectChain::setSuperParameter(double value, bool force) {
    m_pControlChainSuperParameter->set(value);
    slotControlChainSuperParameter(value, force);
}

EffectChainMixMode::Type EffectChain::mixMode() const {
    return static_cast<EffectChainMixMode::Type>(
            static_cast<int>(
                    m_pControlChainMixMode->get()));
}

void EffectChain::setMixMode(EffectChainMixMode::Type mixMode) {
    m_pControlChainMixMode->set(static_cast<double>(mixMode));
    sendParameterUpdate();
}

EffectSlotPointer EffectChain::addEffectSlot(const QString& group) {
    if (kEffectDebugOutput) {
        qDebug() << debugString() << "addEffectSlot" << group;
    }
    EffectSlotPointer pEffectSlot = EffectSlotPointer(new EffectSlot(group,
            m_pEffectsManager,
            m_pMessenger,
            m_effectSlots.size(),
            this,
            m_pEngineEffectChain));

    m_effectSlots.append(pEffectSlot);
    int numEffectSlots = static_cast<int>(m_pControlNumEffectSlots->get()) + 1;
    m_pControlNumEffectSlots->forceSet(numEffectSlots);
    m_pControlChainFocusedEffect->setStates(numEffectSlots);
    return pEffectSlot;
}

int EffectChain::numPresets() const {
    VERIFY_OR_DEBUG_ASSERT(m_pChainPresetManager) {
        return 0;
    }
    return m_pChainPresetManager->numPresets();
}

void EffectChain::registerInputChannel(const ChannelHandleAndGroup& handleGroup,
        const double initialValue) {
    VERIFY_OR_DEBUG_ASSERT(!m_channelEnableButtons.contains(handleGroup)) {
        return;
    }

    auto pEnableControl = std::make_shared<ControlPushButton>(
            ConfigKey(m_group, QString("group_%1_enable").arg(handleGroup.name())),
            true,
            initialValue);
    m_channelEnableButtons.insert(handleGroup, pEnableControl);
    pEnableControl->setButtonMode(mixxx::control::ButtonMode::PowerWindow);
    if (pEnableControl->toBool()) {
        enableForInputChannel(handleGroup);
    }

    connect(pEnableControl.get(),
            &ControlObject::valueChanged,
            this,
            [this, handleGroup](double value) { slotChannelStatusChanged(value, handleGroup); });
}

EffectSlotPointer EffectChain::getEffectSlot(unsigned int slotNumber) {
    //qDebug() << debugString() << "getEffectSlot" << slotNumber;
    VERIFY_OR_DEBUG_ASSERT(slotNumber <= static_cast<unsigned int>(m_effectSlots.size())) {
        return EffectSlotPointer();
    }
    return m_effectSlots[slotNumber];
}

void EffectChain::slotControlClear(double v) {
    for (const auto& pEffectSlot : std::as_const(m_effectSlots)) {
        pEffectSlot->slotClear(v);
    }
}

void EffectChain::slotControlChainSuperParameter(double v, bool force) {
    // qDebug() << debugString() << "slotControlChainSuperParameter" << v;

    m_pControlChainSuperParameter->set(v);
    for (const auto& pEffectSlot : std::as_const(m_effectSlots)) {
        pEffectSlot->setMetaParameter(v, force);
    }
}

void EffectChain::slotControlChainPresetSelector(double value) {
    int index = presetIndex();
    if (value > 0) {
        index++;
    } else if (value < 0) {
        index--;
    } else {
        // Do not reload the current preset when set to 0.
        return;
    }
    loadChainPreset(presetAtIndex(index));
}

void EffectChain::slotControlLoadedChainPresetRequest(double value) {
    int index = static_cast<int>(value);
    if (index < 0 || index >= numPresets()) {
        return;
    }
    // loadChainPreset calls setAndConfirm
    loadChainPreset(presetAtIndex(index));
}

void EffectChain::setControlLoadedPresetIndex(int index) {
    m_pControlLoadedChainPreset->setAndConfirm(index);
}

void EffectChain::slotControlNextChainPreset(double value) {
    if (value > 0) {
        slotControlChainPresetSelector(1);
    }
}

void EffectChain::slotControlPrevChainPreset(double value) {
    if (value > 0) {
        slotControlChainPresetSelector(-1);
    }
}

void EffectChain::slotChannelStatusChanged(
        double value, const ChannelHandleAndGroup& handleGroup) {
    if (value > 0) {
        enableForInputChannel(handleGroup);
    } else {
        disableForInputChannel(handleGroup);
    }
}

void EffectChain::slotEffectChainPresetRenamed(const QString& oldName, const QString& newName) {
    if (m_presetName == oldName) {
        m_presetName = newName;
    }
}

void EffectChain::slotPresetListUpdated() {
    setControlLoadedPresetIndex(presetIndex());
    m_pControlNumChainPresets->forceSet(numPresets());
}

void EffectChain::enableForInputChannel(const ChannelHandleAndGroup& handleGroup) {
    if (m_enabledInputChannels.contains(handleGroup)) {
        return;
    }

    EffectsRequest* request = new EffectsRequest();
    request->type = EffectsRequest::ENABLE_EFFECT_CHAIN_FOR_INPUT_CHANNEL;
    request->pTargetChain = m_pEngineEffectChain;
    request->EnableInputChannelForChain.channelHandle = handleGroup.handle();

    // Initialize EffectStates for the input channel here in the main thread to
    // avoid allocating memory in the realtime audio callback thread.

    for (int i = 0; i < m_effectSlots.size(); ++i) {
        m_effectSlots[i]->initalizeInputChannel(handleGroup.handle());
    }

    m_pMessenger->writeRequest(request);

    m_enabledInputChannels.insert(handleGroup);
}

void EffectChain::disableForInputChannel(const ChannelHandleAndGroup& handleGroup) {
    if (!m_enabledInputChannels.remove(handleGroup)) {
        return;
    }

    EffectsRequest* request = new EffectsRequest();
    request->type = EffectsRequest::DISABLE_EFFECT_CHAIN_FOR_INPUT_CHANNEL;
    request->pTargetChain = m_pEngineEffectChain;
    request->DisableInputChannelForChain.channelHandle = handleGroup.handle();
    m_pMessenger->writeRequest(request);
}

int EffectChain::presetIndex() const {
    // 0-indexed, 0 is the empty '---' preset.
    // This can be -1 if the name is not found in the presets list,
    // which is default state of standard effect chains.
    return m_pChainPresetManager->presetIndex(m_presetName);
}

EffectChainPresetPointer EffectChain::presetAtIndex(int index) const {
    return m_pChainPresetManager->presetAtIndex(index);
}
