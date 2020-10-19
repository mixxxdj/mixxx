#pragma once

#include "effects/defs.h"
#include "effects/effectchain.h"
#include "effects/effectslot.h"
#include "effects/effectsmanager.h"
#include "util/memory.h"

class StandardEffectChain : public EffectChain {
  public:
    StandardEffectChain(unsigned int iChainNumber,
            EffectsManager* pEffectsManager,
            EffectsMessengerPointer pEffectsMessenger);
    static QString formatEffectChainGroup(const int iChainNumber);
    static QString formatEffectSlotGroup(const int iChainSlotNumber,
            const int iEffectSlotNumber);
};

class OutputEffectChain : public EffectChain {
  public:
    OutputEffectChain(EffectsManager* pEffectsManager,
            EffectsMessengerPointer pEffectsMessenger);

  private:
    static QString formatEffectChainGroup(const QString& group);
};

class PerGroupEffectChain : public EffectChain {
  public:
    PerGroupEffectChain(const QString& group,
            const QString& chainSlotGroup,
            SignalProcessingStage stage,
            EffectsManager* pEffectsManager,
            EffectsMessengerPointer pEffectsMessenger);
};

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
