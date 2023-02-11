#pragma once

#include "effects/effectchain.h"

/// OutputEffectChain is hardwired to only one of Mixxx's outputs.
/// This is used for the main mix equalizer.
class OutputEffectChain : public EffectChain {
    Q_OBJECT
  public:
    OutputEffectChain(EffectsManager* pEffectsManager,
            EffectsMessengerPointer pEffectsMessenger);

  private:
    static QString formatEffectChainGroup(const QString& group);
};
