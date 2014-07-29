#ifndef BUTTERWORTHSTATICEQEFFECT_H
#define BUTTERWORTHSTATICEQEFFECT_H

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

class ButterworthStaticEQEffectGroupState {
  public:
    ButterworthStaticEQEffectGroupState();
    virtual ~ButterworthStaticEQEffectGroupState();

    void setFilters(int sampleRate, int lowFreq, int highFreq);

    EngineFilterButterworth8Low* low;
    QList<EngineFilterButterworth8Band*> band;
    EngineFilterButterworth8High* high;

    double old_low;
    QList<double> old_mid;
    double old_high;

    CSAMPLE* m_pLowBuf;
    QList<CSAMPLE*> m_pBandBuf;
    CSAMPLE* m_pHighBuf;
};

class ButterworthStaticEQEffect : public GroupEffectProcessor<ButterworthStaticEQEffectGroupState> {
  public:
    ButterworthStaticEQEffect(EngineEffect* pEffect, const EffectManifest& manifest);
    virtual ~ButterworthStaticEQEffect();

    static QString getId();
    static EffectManifest getManifest();

    // See effectprocessor.h
    void processGroup(const QString& group,
                      ButterworthStaticEQEffectGroupState* pState,
                      const CSAMPLE* pInput, CSAMPLE *pOutput,
                      const unsigned int numSamples,
                      const GroupFeatureState& groupFeatureState);

  private:
    QString debugString() const {
        return getId();
    }

    EngineEffectParameter* m_pPotLow;
    QList<EngineEffectParameter*> m_pPotMid;
    EngineEffectParameter* m_pPotHigh;

    DISALLOW_COPY_AND_ASSIGN(ButterworthStaticEQEffect);
};

#endif /* BUTTERWORTHSTATICEQEFFECT_H */
