#pragma once

#include "effects/effectchainslot.h"
#include "effects/effectsmanager.h"
#include "effects/effectslot.h"
#include "effects/defs.h"
#include "util/memory.h"

class StandardEffectChainSlot : public EffectChainSlot {
  public:
    StandardEffectChainSlot(unsigned int iChainNumber,
            EffectsManager* pEffectsManager,
            EffectsMessengerPointer pEffectsMessenger);
    static QString formatEffectChainSlotGroup(const int iChainNumber);
    static QString formatEffectSlotGroup(const int iChainSlotNumber,
                                         const int iEffectSlotNumber);
};

class OutputEffectChainSlot : public EffectChainSlot {
  public:
    OutputEffectChainSlot(EffectsManager* pEffectsManager,
            EffectsMessengerPointer pEffectsMessenger);

  private:
    static QString formatEffectChainSlotGroup(const QString& group);
};

class PerGroupEffectChainSlot : public EffectChainSlot {
  public:
    PerGroupEffectChainSlot(const QString& group,
            const QString& chainSlotGroup,
            SignalProcessingStage stage,
            EffectsManager* pEffectsManager,
            EffectsMessengerPointer pEffectsMessenger);
};

class QuickEffectChainSlot : public PerGroupEffectChainSlot {
  public:
    QuickEffectChainSlot(const QString& group,
            EffectsManager* pEffectsManager,
            EffectsMessengerPointer pEffectsMessenger);

    static QString formatEffectChainSlotGroup(const QString& group);
    static QString formatEffectSlotGroup(const QString& group,
                                         const int iEffectSlotNumber = 0);

    int presetIndex() const override;
    EffectChainPresetPointer presetAtIndex(int index) const override;

    void loadChainPreset(EffectChainPresetPointer pPreset) override;
    int numPresets() const override;
};

class EqualizerEffectChainSlot : public PerGroupEffectChainSlot {
  public:
    EqualizerEffectChainSlot(const QString& group,
            EffectsManager* pEffectsManager,
            EffectsMessengerPointer pEffectsMessenger);

    void setFilterWaveform(bool state);

    static QString formatEffectChainSlotGroup(const QString& group);
    static QString formatEffectSlotGroup(const QString& group);

  private:
    void setupLegacyAliasesForGroup(const QString& group);
    std::unique_ptr<ControlObject> m_pCOFilterWaveform;
};
