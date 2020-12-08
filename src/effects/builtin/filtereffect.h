#pragma once

#include "effects/effect.h"
#include "effects/effectprocessor.h"
#include "engine/effects/engineeffect.h"
#include "engine/effects/engineeffectparameter.h"
#include "engine/filters/enginefilterbiquad1.h"
#include "util/class.h"
#include "util/defs.h"
#include "util/sample.h"
#include "util/samplebuffer.h"
#include "util/types.h"

struct FilterGroupState : public EffectState {
    FilterGroupState(const mixxx::EngineParameters& bufferParameters);
    ~FilterGroupState();

    void setFilters(int sampleRate, double lowFreq, double highFreq);

    mixxx::SampleBuffer m_buffer;
    EngineFilterBiquad1Low* m_pLowFilter;
    EngineFilterBiquad1High* m_pHighFilter;

    double m_loFreq;
    double m_q;
    double m_hiFreq;

};

class FilterEffect : public EffectProcessorImpl<FilterGroupState> {
  public:
    FilterEffect(EngineEffect* pEffect);
    virtual ~FilterEffect();

    static QString getId();
    static EffectManifestPointer getManifest();

    void processChannel(const ChannelHandle& handle,
                        FilterGroupState* pState,
                        const CSAMPLE* pInput, CSAMPLE *pOutput,
                        const mixxx::EngineParameters& bufferParameters,
                        const EffectEnableState enableState,
                        const GroupFeatureState& groupFeatures);

  private:
    QString debugString() const {
        return getId();
    }

    EngineEffectParameter* m_pLPF;
    EngineEffectParameter* m_pQ;
    EngineEffectParameter* m_pHPF;

    DISALLOW_COPY_AND_ASSIGN(FilterEffect);
};
