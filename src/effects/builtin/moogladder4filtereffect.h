#ifndef MOOGLADDER4FILTEREFFECT_H
#define MOOGLADDER4FILTEREFFECT_H

#include "effects/effect.h"
#include "effects/effectprocessor.h"
#include "engine/effects/engineeffect.h"
#include "engine/effects/engineeffectparameter.h"
#include "engine/filters/enginefiltermoogladder4.h"
#include "util/class.h"
#include "util/defs.h"
#include "util/sample.h"
#include "util/types.h"

class MoogLadder4FilterGroupState : public EffectState {
  public:
    MoogLadder4FilterGroupState(const mixxx::EngineParameters& bufferParameters);
    ~MoogLadder4FilterGroupState();
    void setFilters(int sampleRate, double lowFreq, double highFreq);

    CSAMPLE* m_pBuf;
    EngineFilterMoogLadder4Low* m_pLowFilter;
    EngineFilterMoogLadder4High* m_pHighFilter;

    double m_loFreq;
    double m_resonance;
    double m_hiFreq;
    double m_samplerate;
};

class MoogLadder4FilterEffect : public EffectProcessorImpl<MoogLadder4FilterGroupState> {
  public:
    MoogLadder4FilterEffect(EngineEffect* pEffect);
    virtual ~MoogLadder4FilterEffect();

    static QString getId();
    static EffectManifestPointer getManifest();

    // See effectprocessor.h
    void processChannel(const ChannelHandle& handle,
                        MoogLadder4FilterGroupState* pState,
                        const CSAMPLE* pInput, CSAMPLE *pOutput,
                        const mixxx::EngineParameters& bufferParameters,
                        const EffectEnableState enableState,
                        const GroupFeatureState& groupFeatures);

  private:
    QString debugString() const {
        return getId();
    }

    EngineEffectParameter* m_pLPF;
    EngineEffectParameter* m_pResonance;
    EngineEffectParameter* m_pHPF;

    DISALLOW_COPY_AND_ASSIGN(MoogLadder4FilterEffect);
};

#endif /* MOOGLADDER4FILTEREFFECT_H */
