#ifndef EQEFFECT_H
#define EQEFFECT_H

#include <QMap>

#include "effects/effect.h"
#include "effects/effectprocessor.h"
#include "engine/effects/engineeffect.h"
#include "engine/effects/engineeffectparameter.h"
#include "engine/enginefilterbutterworth8.h"
#include "util.h"
#include "util/types.h"
#include "util/defs.h"
#include "sampleutil.h"

class EqEffectGroupState {
public:
    EqEffectGroupState();
    ~EqEffectGroupState();
    CSAMPLE *m_pLowBuf, *m_pBandBuf, *m_pHighBuf;
    double old_low, old_mid, old_high, old_dry;
    EngineObjectConstIn *low, *band, *high;
};

class EqEffect : public GroupEffectProcessor<EqEffectGroupState> {
  public:
    EqEffect(EngineEffect* pEffect, const EffectManifest& manifest);
    virtual ~EqEffect();

    static QString getId();
    static EffectManifest getManifest();

    // See effectprocessor.h
    void processGroup(const QString& group,
                      EqEffectGroupState* pState,
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

    DISALLOW_COPY_AND_ASSIGN(EqEffect);
};

#endif /* EQEFFECT_H */
