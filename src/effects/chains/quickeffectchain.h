#pragma once

#include "effects/chains/pergroupeffectchain.h"

/// QuickEffectChain is a simplified interface for effect chains.
/// It only presents the superknob and chain enable switch to the user.
/// The user can design complex EffectChainPresets with a StandardEffectChain
/// then load it into QuickEffectChain. QuickEffectChain is hardwired to one
/// input channel with the mix knob fully enabled.
class QuickEffectChain : public PerGroupEffectChain {
    Q_OBJECT
  public:
    QuickEffectChain(
            const ChannelHandleAndGroup& handleAndGroup,
            EffectsManager* pEffectsManager,
            EffectsMessengerPointer pEffectsMessenger);

    static QString formatEffectChainGroup(const QString& group);
    static QString formatEffectSlotGroup(const QString& group,
            const int iEffectSlotNumber = 0);

    int presetIndex() const override;
    EffectChainPresetPointer presetAtIndex(int index) const override;

    void loadChainPreset(EffectChainPresetPointer pPreset) override;
    int numPresets() const override;

  signals:
    void presetIndexChanged(int presetIndex);
};
