#include "effects/specialeffectchains.h"

#include "mixer/playermanager.h"

StandardEffectChain::StandardEffectChain(unsigned int iChainNumber,
        EffectsManager* pEffectsManager,
        EffectsMessengerPointer pEffectsMessenger)
        : EffectChain(formatEffectChainGroup(iChainNumber),
                  pEffectsManager,
                  pEffectsMessenger,
                  SignalProcessingStage::Postfader) {
    for (int i = 0; i < kNumEffectsPerUnit; ++i) {
        addEffectSlot(formatEffectSlotGroup(iChainNumber, i));
    }

    const QSet<ChannelHandleAndGroup>& registeredChannels =
            m_pEffectsManager->registeredInputChannels();
    for (const ChannelHandleAndGroup& handle_group : registeredChannels) {
        int deckNumber;
        if (PlayerManager::isDeckGroup(handle_group.name(), &deckNumber) &&
                (iChainNumber + 1) == (unsigned)deckNumber) {
            registerInputChannel(handle_group, 1.0);
        } else {
            registerInputChannel(handle_group, 0.0);
        }
    }
}

QString StandardEffectChain::formatEffectChainGroup(const int iChainNumber) {
    return QString("[EffectRack1_EffectUnit%1]")
            .arg(QString::number(iChainNumber + 1));
}

QString StandardEffectChain::formatEffectSlotGroup(const int iChainSlotNumber,
        const int iEffectSlotNumber) {
    return QString("[EffectRack1_EffectUnit%1_Effect%2]")
            .arg(QString::number(iChainSlotNumber + 1))
            .arg(QString::number(iEffectSlotNumber + 1));
}

OutputEffectChain::OutputEffectChain(EffectsManager* pEffectsManager,
        EffectsMessengerPointer pEffectsMessenger)
        : EffectChain(formatEffectChainGroup("[Master]"),
                  pEffectsManager,
                  pEffectsMessenger,
                  SignalProcessingStage::Postfader) {
    addEffectSlot("[OutputEffectRack_[Master]_Effect1]");
    m_effectSlots[0]->setEnabled(true);

    // Register the master channel
    const ChannelHandleAndGroup* masterHandleAndGroup = nullptr;

    // TODO(Be): Remove this hideous hack to get the ChannelHandleAndGroup
    const QSet<ChannelHandleAndGroup>& registeredChannels =
            m_pEffectsManager->registeredInputChannels();
    for (const ChannelHandleAndGroup& handle_group : registeredChannels) {
        if (handle_group.name() == "[MasterOutput]") {
            masterHandleAndGroup = &handle_group;
            break;
        }
    }
    DEBUG_ASSERT(masterHandleAndGroup != nullptr);

    registerInputChannel(*masterHandleAndGroup);
    enableForInputChannel(*masterHandleAndGroup);
    m_pControlChainMix->set(1.0);
    sendParameterUpdate();
}

QString OutputEffectChain::formatEffectChainGroup(
        const QString& group) {
    return QString("[OutputEffectRack_%1]").arg(group);
}

PerGroupEffectChain::PerGroupEffectChain(const QString& group,
        const QString& chainSlotGroup,
        SignalProcessingStage stage,
        EffectsManager* pEffectsManager,
        EffectsMessengerPointer pEffectsMessenger)
        : EffectChain(chainSlotGroup,
                  pEffectsManager,
                  pEffectsMessenger,
                  stage) {
    // Set the chain to be fully wet.
    m_pControlChainMix->set(1.0);
    sendParameterUpdate();

    // TODO(rryan): remove.
    const ChannelHandleAndGroup* handleAndGroup = nullptr;
    for (const ChannelHandleAndGroup& handle_group :
            m_pEffectsManager->registeredInputChannels()) {
        if (handle_group.name() == group) {
            handleAndGroup = &handle_group;
            break;
        }
    }
    DEBUG_ASSERT(handleAndGroup != nullptr);

    // Register this channel alone with the chain slot.
    registerInputChannel(*handleAndGroup);
    enableForInputChannel(*handleAndGroup);
}

QuickEffectChain::QuickEffectChain(const QString& group,
        EffectsManager* pEffectsManager,
        EffectsMessengerPointer pEffectsMessenger)
        : PerGroupEffectChain(group,
                  formatEffectChainGroup(group),
                  SignalProcessingStage::Postfader,
                  pEffectsManager,
                  pEffectsMessenger) {
    for (int i = 0; i < kNumEffectsPerUnit; ++i) {
        addEffectSlot(formatEffectSlotGroup(group, i));
        m_effectSlots.at(i)->setEnabled(true);
    }
    disconnect(m_pChainPresetManager.get(),
            &EffectChainPresetManager::effectChainPresetListUpdated,
            this,
            &QuickEffectChain::slotPresetListUpdated);
    m_pControlNumPresetsAvailable->forceSet(m_pChainPresetManager->numQuickEffectPresets());
    connect(m_pChainPresetManager.get(),
            &EffectChainPresetManager::quickEffectChainPresetListUpdated,
            this,
            &QuickEffectChain::slotPresetListUpdated);
}

QString QuickEffectChain::formatEffectChainGroup(const QString& group) {
    return QString("[QuickEffectRack1_%1]").arg(group);
}

QString QuickEffectChain::formatEffectSlotGroup(
        const QString& group, const int iEffectSlotNumber) {
    return QString("[QuickEffectRack1_%1_Effect%2]")
            .arg(group)
            .arg(QString::number(iEffectSlotNumber + 1));
}

int QuickEffectChain::presetIndex() const {
    return m_pChainPresetManager->quickEffectPresetIndex(m_presetName);
}

EffectChainPresetPointer QuickEffectChain::presetAtIndex(int index) const {
    return m_pChainPresetManager->quickEffectPresetAtIndex(index);
}

void QuickEffectChain::loadChainPreset(EffectChainPresetPointer pPreset) {
    EffectChain::loadChainPreset(pPreset);
    setSuperParameter(pPreset->superKnob(), true);
}

int QuickEffectChain::numPresets() const {
    VERIFY_OR_DEBUG_ASSERT(m_pChainPresetManager) {
        return 0;
    }
    return m_pChainPresetManager->numQuickEffectPresets();
}

EqualizerEffectChain::EqualizerEffectChain(const QString& group,
        EffectsManager* pEffectsManager,
        EffectsMessengerPointer pEffectsMessenger)
        : PerGroupEffectChain(group,
                  formatEffectChainGroup(group),
                  SignalProcessingStage::Prefader,
                  pEffectsManager,
                  pEffectsMessenger),
          m_pCOFilterWaveform(
                  new ControlObject(ConfigKey(group, "filterWaveformEnable"))) {
    // Add a single effect slot
    addEffectSlot(formatEffectSlotGroup(group));
    m_effectSlots[0]->setEnabled(true);
    // DlgPrefEq loads the Effect with loadEffectToGroup

    setupLegacyAliasesForGroup(group);
}

void EqualizerEffectChain::setFilterWaveform(bool state) {
    m_pCOFilterWaveform->set(state);
}

QString EqualizerEffectChain::formatEffectChainGroup(const QString& group) {
    return QString("[EqualizerRack1_%1]").arg(group);
}

QString EqualizerEffectChain::formatEffectSlotGroup(const QString& group) {
    return QString("[EqualizerRack1_%1_Effect1]")
            .arg(group);
}

void EqualizerEffectChain::setupLegacyAliasesForGroup(const QString& group) {
    // Create aliases for legacy EQ controls.
    EffectSlotPointer pEffectSlot = getEffectSlot(0);
    if (pEffectSlot) {
        const QString& effectSlotGroup = pEffectSlot->getGroup();
        ControlDoublePrivate::insertAlias(ConfigKey(group, "filterLow"),
                ConfigKey(effectSlotGroup, "parameter1"));

        ControlDoublePrivate::insertAlias(ConfigKey(group, "filterMid"),
                ConfigKey(effectSlotGroup, "parameter2"));

        ControlDoublePrivate::insertAlias(ConfigKey(group, "filterHigh"),
                ConfigKey(effectSlotGroup, "parameter3"));

        ControlDoublePrivate::insertAlias(ConfigKey(group, "filterLowKill"),
                ConfigKey(effectSlotGroup, "button_parameter1"));

        ControlDoublePrivate::insertAlias(ConfigKey(group, "filterMidKill"),
                ConfigKey(effectSlotGroup, "button_parameter2"));

        ControlDoublePrivate::insertAlias(ConfigKey(group, "filterHighKill"),
                ConfigKey(effectSlotGroup, "button_parameter3"));

        ControlDoublePrivate::insertAlias(ConfigKey(group, "filterLow_loaded"),
                ConfigKey(effectSlotGroup, "parameter1_loaded"));

        ControlDoublePrivate::insertAlias(ConfigKey(group, "filterMid_loaded"),
                ConfigKey(effectSlotGroup, "parameter2_loaded"));

        ControlDoublePrivate::insertAlias(ConfigKey(group, "filterHigh_loaded"),
                ConfigKey(effectSlotGroup, "parameter3_loaded"));

        ControlDoublePrivate::insertAlias(ConfigKey(group, "filterLowKill_loaded"),
                ConfigKey(effectSlotGroup, "button_parameter1_loaded"));

        ControlDoublePrivate::insertAlias(ConfigKey(group, "filterMidKill_loaded"),
                ConfigKey(effectSlotGroup, "button_parameter2_loaded"));

        ControlDoublePrivate::insertAlias(ConfigKey(group, "filterHighKill_loaded"),
                ConfigKey(effectSlotGroup, "button_parameter3_loaded"));
    }
}
