#ifndef LOFIEQ_H
#define LOFIEQ_H

#include "controlobjectslave.h"
#include "effects/effect.h"
#include "effects/effectprocessor.h"
#include "engine/enginefilteriir.h"
#include "engine/effects/engineeffect.h"
#include "engine/effects/engineeffectparameter.h"
#include "util.h"
#include "util/types.h"
#include "util/defs.h"
#include "sampleutil.h"

class LoFiEQGroupState {
public:
    LoFiEQGroupState();
    ~LoFiEQGroupState();


    EngineFilterIIR* low;
    EngineFilterIIR* band;
    EngineFilterIIR* high;

    double old_low;
    double old_mid;
    double old_high;
    double old_dry;

    CSAMPLE* m_pLowBuf;
    CSAMPLE* m_pBandBuf;
    CSAMPLE* m_pHighBuf;
};

class LoFiEQ : public GroupEffectProcessor<LoFiEQGroupState> {
  public:
    LoFiEQ(EngineEffect* pEffect, const EffectManifest& manifest);
    virtual ~LoFiEQ();

    static QString getId();
    static EffectManifest getManifest();

    // See effectprocessor.h
    void processGroup(const QString& group,
                      LoFiEQGroupState* pState,
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

    DISALLOW_COPY_AND_ASSIGN(LoFiEQ);
};

#endif // LOFIEQ_H
