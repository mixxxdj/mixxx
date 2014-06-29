#ifndef LIGHTWEIGHT_H
#define LIGHTWEIGHT_H

#include "effects/effect.h"
#include "effects/effectprocessor.h"
#include "engine/enginefilteriir.h"
#include "engine/effects/engineeffect.h"
#include "engine/effects/engineeffectparameter.h"
#include "util.h"
#include "util/types.h"
#include "util/defs.h"
#include "sampleutil.h"

class LightweightEQGroupState {
public:
    LightweightEQGroupState();
    ~LightweightEQGroupState();

    EngineFilterIIR* low;
    EngineFilterIIR* band;
    EngineFilterIIR* high;

    double old_low;
    double old_mid;
    double old_high;

    CSAMPLE* m_pLowBuf;
    CSAMPLE* m_pBandBuf;
    CSAMPLE* m_pHighBuf;
};

class LightweightEQ : public GroupEffectProcessor<LightweightEQGroupState> {
  public:
    LightweightEQ(EngineEffect* pEffect, const EffectManifest& manifest);
    virtual ~LightweightEQ();

    static QString getId();
    static EffectManifest getManifest();

    // See effectprocessor.h
    void processGroup(const QString& group,
                      LightweightEQGroupState* pState,
                      const CSAMPLE* pInput, CSAMPLE *pOutput,
                      const unsigned int numSamples,
                      const GroupFeatureState& groupFeatureState);

  private:
    QString debugString() const {
        return getId();
    }

    EngineEffectParameter* m_pPotLow;
    EngineEffectParameter* m_pPotMid;
    EngineEffectParameter* m_pPotHigh;

    DISALLOW_COPY_AND_ASSIGN(LightweightEQ);
};

#endif // LIGHTWEIGHT_H
