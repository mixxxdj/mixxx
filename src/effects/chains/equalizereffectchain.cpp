#include "effects/chains/equalizereffectchain.h"

#include "control/controlobject.h"
#include "effects/effectslot.h"
#include "moc_equalizereffectchain.cpp"

EqualizerEffectChain::EqualizerEffectChain(
        const ChannelHandleAndGroup& handleAndGroup,
        EffectsManager* pEffectsManager,
        EffectsMessengerPointer pEffectsMessenger)
        : PerGroupEffectChain(
                  handleAndGroup,
                  formatEffectChainGroup(handleAndGroup.name()),
                  SignalProcessingStage::Prefader,
                  pEffectsManager,
                  pEffectsMessenger),
          m_pCOFilterWaveform(
                  new ControlObject(ConfigKey(handleAndGroup.name(), "filterWaveformEnable"))) {
    // Add a single effect slot
    addEffectSlot(formatEffectSlotGroup(handleAndGroup.name()));
    enableForInputChannel(handleAndGroup);
    m_effectSlots[0]->setEnabled(true);

    QObject::connect(this,
            &EffectChain::chainPresetChanged,
            this,
            [this](const QString& presetname) {
                Q_UNUSED(presetname);
                setFilterWaveform(
                        m_effectSlots.at(0)->getManifest()->isMixingEQ());
            });

    setupLegacyAliasesForGroup(handleAndGroup.name());
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
