#ifndef BITCRUSHEREFFECT_H
#define BITCRUSHEREFFECT_H

#include <QMap>

#include "effects/effect.h"
#include "effects/effectprocessor.h"
#include "engine/effects/engineeffect.h"
#include "engine/effects/engineeffectparameter.h"
#include "util.h"
#include "util/types.h"

struct BitCrusherGroupState {
    // Default accumulator to 1 so we immediately pick an input value.
    BitCrusherGroupState()
            : hold_l(0),
              hold_r(0),
              accumulator(1) {
    }
    CSAMPLE hold_l, hold_r;
    // Accumulated fractions of a samplerate period.
    CSAMPLE accumulator;
};

class BitCrusherEffect : public GroupEffectProcessor<BitCrusherGroupState> {
  public:
    BitCrusherEffect(EngineEffect* pEffect, const EffectManifest& manifest);
    virtual ~BitCrusherEffect();

    static QString getId();
    static EffectManifest getManifest();

    // See effectprocessor.h
    void processGroup(const QString& group,
                      BitCrusherGroupState* pState,
                      const CSAMPLE* pInput, CSAMPLE *pOutput,
                      const unsigned int numSamples,
                      const unsigned int sampleRate,
                      const EffectProcessor::EnableState enableState,
                      const GroupFeatureState& groupFeatureState);

  private:
    QString debugString() const {
        return getId();
    }

    EngineEffectParameter* m_pBitDepthParameter;
    EngineEffectParameter* m_pDownsampleParameter;

    DISALLOW_COPY_AND_ASSIGN(BitCrusherEffect);
};

#endif /* BITCRUSHEREFFECT_H */
