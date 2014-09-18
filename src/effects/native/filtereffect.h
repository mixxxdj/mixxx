#ifndef FILTEREFFECT_H
#define FILTEREFFECT_H

#include "effects/effect.h"
#include "effects/effectprocessor.h"
#include "engine/effects/engineeffect.h"
#include "engine/effects/engineeffectparameter.h"
#include "engine/enginefilterbiquad1.h"
#include "sampleutil.h"
#include "util.h"
#include "util/defs.h"
#include "util/types.h"



struct FilterGroupState {
    FilterGroupState();
    ~FilterGroupState();
    void setFilters(int sampleRate, double lowFreq, double highFreq);

    CSAMPLE* m_pBuf;
    EngineFilterBiquad1High* m_pHighFilter;
    EngineFilterBiquad1Low* m_pLowFilter;

    double m_hiFreq;
    double m_q;
    double m_loFreq;

};

class FilterEffect : public GroupEffectProcessor<FilterGroupState> {
  public:
    FilterEffect(EngineEffect* pEffect, const EffectManifest& manifest);
    virtual ~FilterEffect();

    static QString getId();
    static EffectManifest getManifest();

    // See effectprocessor.h
    void processGroup(const QString& group,
                      FilterGroupState* pState,
                      const CSAMPLE* pInput, CSAMPLE *pOutput,
                      const unsigned int numSamples,
                      const unsigned int sampleRate,
                      const GroupFeatureState& groupFeatures);

  private:
    QString debugString() const {
        return getId();
    }

    EngineEffectParameter* m_pHPF;
    EngineEffectParameter* m_pQ;
    EngineEffectParameter* m_pLPF;

    DISALLOW_COPY_AND_ASSIGN(FilterEffect);
};

#endif /* FILTEREFFECT_H */
