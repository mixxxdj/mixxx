#pragma once

#include "effects/effectchain.h"

/// StandardEffectChain is a chain shown in the GUI with the
/// detail of the input routing switches, mix knob, superknob,
/// all effects, their parameters, enable switches, and metaknob
/// linkings. However, the chain enable switch is hidden because it
/// is redundant with the input routing switches and effect enable switches.
class StandardEffectChain : public EffectChain {
    Q_OBJECT
  public:
    StandardEffectChain(unsigned int iChainNumber,
            EffectsManager* pEffectsManager,
            EffectsMessengerPointer pEffectsMessenger);
    static QString formatEffectChainGroup(const int iChainNumber);
    static QString formatEffectSlotGroup(const int iChainSlotNumber,
            const int iEffectSlotNumber);
};
