#include "effects/chains/quickeffectchain.h"

#include "control/controlobject.h"
#include "effects/effectslot.h"
#include "effects/presets/effectchainpreset.h"
#include "effects/presets/effectchainpresetmanager.h"
#include "moc_quickeffectchain.cpp"

QuickEffectChain::QuickEffectChain(
        const ChannelHandleAndGroup& handleAndGroup,
        EffectsManager* pEffectsManager,
        EffectsMessengerPointer pEffectsMessenger)
        : PerGroupEffectChain(
                  handleAndGroup,
                  formatEffectChainGroup(handleAndGroup.name()),
                  SignalProcessingStage::Postfader,
                  pEffectsManager,
                  pEffectsMessenger) {
    for (int i = 0; i < kNumEffectsPerUnit; ++i) {
        addEffectSlot(formatEffectSlotGroup(handleAndGroup.name(), i));
        m_effectSlots.at(i)->setEnabled(true);
    }
    enableForInputChannel(handleAndGroup);
    // The base EffectChain class constructor connects to the signal for the list of StandardEffectChain presets,
    // but QuickEffectChain has a separate list, so disconnect the signal which is inappropriate for this subclass.
    disconnect(m_pChainPresetManager.data(),
            &EffectChainPresetManager::effectChainPresetListUpdated,
            this,
            &QuickEffectChain::slotPresetListUpdated);
    m_pControlNumChainPresets->forceSet(m_pChainPresetManager->numQuickEffectPresets());
    connect(m_pChainPresetManager.data(),
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
            .arg(group,
                    QString::number(iEffectSlotNumber + 1));
}

int QuickEffectChain::presetIndex() const {
    return m_pChainPresetManager->quickEffectPresetIndex(m_presetName);
}

EffectChainPresetPointer QuickEffectChain::presetAtIndex(int index) const {
    return m_pChainPresetManager->quickEffectPresetAtIndex(index);
}

void QuickEffectChain::loadChainPreset(EffectChainPresetPointer pPreset) {
    EffectChain::loadChainPreset(pPreset);
    if (pPreset) {
        setSuperParameter(pPreset->superKnob(), true);
    }
}

int QuickEffectChain::numPresets() const {
    VERIFY_OR_DEBUG_ASSERT(m_pChainPresetManager) {
        return 0;
    }
    return m_pChainPresetManager->numQuickEffectPresets();
}
