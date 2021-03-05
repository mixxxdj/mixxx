#pragma once

#include "effects/defs.h"
#include "effects/effectchain.h"
#include "effects/effectslot.h"
#include "effects/effectsmanager.h"
#include "util/memory.h"

/// StandardEffectChain is a chain shown in the GUI with the
/// detail of the input routing switches, mix knob, superknob,
/// all effects, their parameters, enable switches, and metaknob
/// linkings. However, the chain enable switch is hidden because it
/// is redundant with the input routing switches and effect enable switches.
class StandardEffectChain : public EffectChain {
  public:
    StandardEffectChain(unsigned int iChainNumber,
            EffectsManager* pEffectsManager,
            EffectsMessengerPointer pEffectsMessenger);
    static QString formatEffectChainGroup(const int iChainNumber);
    static QString formatEffectSlotGroup(const int iChainSlotNumber,
            const int iEffectSlotNumber);
};

/// OutputEffectChain is hardwired to only one of Mixxx's outputs.
/// This is used for the main mix equalizer.
class OutputEffectChain : public EffectChain {
  public:
    OutputEffectChain(EffectsManager* pEffectsManager,
            EffectsMessengerPointer pEffectsMessenger);

  private:
    static QString formatEffectChainGroup(const QString& group);
};

/// PerGroupEffectChain is a base class hardwired for one input channel.
/// The routing switches are not presented to the user.
class PerGroupEffectChain : public EffectChain {
  public:
    PerGroupEffectChain(const QString& group,
            const QString& chainSlotGroup,
            SignalProcessingStage stage,
            EffectsManager* pEffectsManager,
            EffectsMessengerPointer pEffectsMessenger);
};

/// QuickEffectChain is a simplified interface for effect chains.
/// It only presents the superknob and chain enable switch to the user.
/// The user can design complex EffectChainPresets with a StandardEffectChain
/// then load it into QuickEffectChain. QuickEffectChain is hardwired to one
/// input channel with the mix knob fully enabled.
class QuickEffectChain : public PerGroupEffectChain {
  public:
    QuickEffectChain(const QString& group,
            EffectsManager* pEffectsManager,
            EffectsMessengerPointer pEffectsMessenger);

    static QString formatEffectChainGroup(const QString& group);
    static QString formatEffectSlotGroup(const QString& group,
            const int iEffectSlotNumber = 0);

    int presetIndex() const override;
    EffectChainPresetPointer presetAtIndex(int index) const override;

    void loadChainPreset(EffectChainPresetPointer pPreset) override;
    int numPresets() const override;
};

/// EqualizerEffectChain is specifically for the equalizers only.
/// It only has a single effect in the chain. The user can pick the
/// effect in the preferences. In the main GUI, only the parameters of
/// that single effect are presented, not the metaknob nor superknob.
/// EqualizerEffectChain is hardwired to one input channel, always
/// enabled, and has the mix knob fully enabled.
class EqualizerEffectChain : public PerGroupEffectChain {
  public:
    EqualizerEffectChain(const QString& group,
            EffectsManager* pEffectsManager,
            EffectsMessengerPointer pEffectsMessenger);

    void setFilterWaveform(bool state);

    static QString formatEffectChainGroup(const QString& group);
    static QString formatEffectSlotGroup(const QString& group);

  private:
    void setupLegacyAliasesForGroup(const QString& group);
    std::unique_ptr<ControlObject> m_pCOFilterWaveform;
};
