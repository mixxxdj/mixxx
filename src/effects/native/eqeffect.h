#ifndef EQEFFECT_H
#define EQEFFECT_H

#include <QMap>

#include "controlobjectslave.h"
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

    void setFilters(int sampleRate, int lowFreq, int highFreq);

    EngineObjectConstIn* low;
    EngineObjectConstIn* band;
    EngineObjectConstIn* high;

    double old_low;
    double old_mid;
    double old_high;
    double old_dry;

    CSAMPLE* m_pLowBuf;
    CSAMPLE* m_pBandBuf;
    CSAMPLE* m_pHighBuf;
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

    ControlObjectSlave* m_pLoFreqCorner;
    ControlObjectSlave* m_pHiFreqCorner;

    int m_oldSampleRate;
    int m_loFreq;
    int m_hiFreq;

    DISALLOW_COPY_AND_ASSIGN(EqEffect);
};

#endif /* EQEFFECT_H */
