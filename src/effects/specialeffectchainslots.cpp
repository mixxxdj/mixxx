#include "effects/specialeffectchainslots.h"

#include "effects/effectchainslot.h"
#include "mixer/playermanager.h"

StandardEffectChainSlot::StandardEffectChainSlot(unsigned int iChainNumber,
                                                 EffectsManager* pEffectsManager,
                                                 const QString& id)
        : EffectChainSlot(formatEffectChainSlotGroup(iChainNumber),
                          pEffectsManager,
                          SignalProcessingStage::Postfader, false,
                          formatEffectChainSlotGroup(iChainNumber)) {
    for (int i = 0; i < kNumEffectsPerUnit; ++i) {
        addEffectSlot(formatEffectSlotGroup(iChainNumber, i));
    }

    const QSet<ChannelHandleAndGroup>& registeredChannels =
            m_pEffectsManager->registeredInputChannels();
    for (const ChannelHandleAndGroup& handle_group : registeredChannels) {
        int deckNumber;
        if (PlayerManager::isDeckGroup(handle_group.name(), &deckNumber) &&
            (iChainNumber + 1) == (unsigned) deckNumber) {
            registerInputChannel(handle_group, 1.0);
        } else {
            registerInputChannel(handle_group, 0.0);
        }
    }
}

QString StandardEffectChainSlot::formatEffectChainSlotGroup(const int iChainNumber) {
    return QString("[EffectRack1_EffectUnit%1]")
                   .arg(QString::number(iChainNumber + 1));
}

QString StandardEffectChainSlot::formatEffectSlotGroup(const int iChainSlotNumber,
                                                       const int iEffectSlotNumber) {
    return QString("[EffectRack1_EffectUnit%1_Effect%2]")
                   .arg(QString::number(iChainSlotNumber + 1))
                   .arg(QString::number(iEffectSlotNumber + 1));
}


OutputEffectChainSlot::OutputEffectChainSlot(EffectsManager* pEffectsManager)
        : EffectChainSlot(formatEffectChainSlotGroup("[Master]"),
                          pEffectsManager,
                          SignalProcessingStage::Postfader, true,
                          formatEffectChainSlotGroup("[Master]")) {
    addEffectSlot("[OutputEffectRack_[Master]_Effect1]");

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

QString OutputEffectChainSlot::formatEffectChainSlotGroup(const QString& group) {
    return QString("[OutputEffectRack_%1]").arg(group);
}


PerGroupEffectChainSlot::PerGroupEffectChainSlot(const QString& group,
                                                 const QString& chainSlotGroup,
                                                 EffectsManager* pEffectsManager)
        : EffectChainSlot(chainSlotGroup, pEffectsManager,
                          SignalProcessingStage::Prefader, false,
                          chainSlotGroup) {
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

QuickEffectChainSlot::QuickEffectChainSlot(const QString& group,
                                           EffectsManager* pEffectsManager)
        : PerGroupEffectChainSlot(group, formatEffectChainSlotGroup(group),
                                  pEffectsManager) {
    // Add a single effect slot
    addEffectSlot(formatEffectSlotGroup(group));
    // DlgPrefEq loads the Effect with loadEffectToGroup

    setSuperParameter(0.5);
    setSuperParameterDefaultValue(0.5);
}

QString QuickEffectChainSlot::formatEffectChainSlotGroup(const QString& group) {
    return QString("[QuickEffectRack1_%1]").arg(group);
}

QString QuickEffectChainSlot::formatEffectSlotGroup(const QString& group,
                                                    const int iEffectSlotNumber) {
        return QString("[QuickEffectRack1_%1_Effect%2]")
                .arg(group)
                .arg(QString::number(iEffectSlotNumber + 1));
}


EqualizerEffectChainSlot::EqualizerEffectChainSlot(const QString& group,
                                                   EffectsManager* pEffectsManager)
        : PerGroupEffectChainSlot(group, formatEffectChainSlotGroup(group),
                                  pEffectsManager) {
    // Add a single effect slot
    addEffectSlot(formatEffectSlotGroup(group));
    // DlgPrefEq loads the Effect with loadEffectToGroup

    setupLegacyAliasesForGroup(group);
}

QString EqualizerEffectChainSlot::formatEffectChainSlotGroup(const QString& group) {
    return QString("[EqualizerRack1_%1]").arg(group);
}

QString EqualizerEffectChainSlot::formatEffectSlotGroup(const QString& group) {
    return QString("[EqualizerRack1_%1_Effect1]")
            .arg(group);
}

void EqualizerEffectChainSlot::setupLegacyAliasesForGroup(const QString& group) {
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
