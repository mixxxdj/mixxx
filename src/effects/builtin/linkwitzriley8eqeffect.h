#ifndef LINKWITZRILEYEQEFFECT_H
#define LINKWITZRILEYEQEFFECT_H

#include <QMap>

#include "control/controlproxy.h"
#include "effects/effect.h"
#include "effects/effectprocessor.h"
#include "engine/effects/engineeffect.h"
#include "engine/effects/engineeffectparameter.h"
#include "engine/filters/enginefilterlinkwitzriley8.h"
#include "util/class.h"
#include "util/defs.h"
#include "util/sample.h"
#include "util/types.h"

class LinkwitzRiley8EQEffectGroupState : public EffectState {
  public:
    LinkwitzRiley8EQEffectGroupState(const mixxx::EngineParameters& bufferParameters);
    virtual ~LinkwitzRiley8EQEffectGroupState();

    void setFilters(int sampleRate, int lowFreq, int highFreq);

    EngineFilterLinkwitzRiley8Low* m_low1;
    EngineFilterLinkwitzRiley8High* m_high1;
    EngineFilterLinkwitzRiley8Low* m_low2;
    EngineFilterLinkwitzRiley8High* m_high2;

    double old_low;
    double old_mid;
    double old_high;

    CSAMPLE* m_pLowBuf;
    CSAMPLE* m_pMidBuf;
    CSAMPLE* m_pHighBuf;

    unsigned int m_oldSampleRate;
    int m_loFreq;
    int m_hiFreq;
};

class LinkwitzRiley8EQEffect : public EffectProcessorImpl<LinkwitzRiley8EQEffectGroupState> {
  public:
    LinkwitzRiley8EQEffect(EngineEffect* pEffect);
    virtual ~LinkwitzRiley8EQEffect();

    static QString getId();
    static EffectManifestPointer getManifest();

    // See effectprocessor.h
    void processChannel(const ChannelHandle& handle,
                        LinkwitzRiley8EQEffectGroupState* pState,
                        const CSAMPLE* pInput, CSAMPLE *pOutput,
                        const mixxx::EngineParameters& bufferParameters,
                        const EffectEnableState enableState,
                        const GroupFeatureState& groupFeatureState);

  private:
    QString debugString() const {
        return getId();
    }

    EngineEffectParameter* m_pPotLow;
    EngineEffectParameter* m_pPotMid;
    EngineEffectParameter* m_pPotHigh;

    EngineEffectParameter* m_pKillLow;
    EngineEffectParameter* m_pKillMid;
    EngineEffectParameter* m_pKillHigh;

    ControlProxy* m_pLoFreqCorner;
    ControlProxy* m_pHiFreqCorner;

    DISALLOW_COPY_AND_ASSIGN(LinkwitzRiley8EQEffect);
};

#endif /* LINKWITZRILEYEQEFFECT_H */
