#ifndef LINKWITZRILEYEQEFFECT_H
#define LINKWITZRILEYEQEFFECT_H

#include <QMap>

#include "controlobjectslave.h"
#include "effects/effect.h"
#include "effects/effectprocessor.h"
#include "engine/effects/engineeffect.h"
#include "engine/effects/engineeffectparameter.h"
#include "engine/enginefilterlinkwitzriley8.h"
#include "util.h"
#include "util/types.h"
#include "util/defs.h"
#include "sampleutil.h"

class LinkwitzRiley8EQEffectGroupState {
  public:
    LinkwitzRiley8EQEffectGroupState();
    virtual ~LinkwitzRiley8EQEffectGroupState();

    void setFilters(int sampleRate, int lowFreq, int highFreq);

    EngineFilterLinkwtzRiley8Low* m_low1;
    EngineFilterLinkwtzRiley8High* m_high1;
    EngineFilterLinkwtzRiley8Low* m_low2;
    EngineFilterLinkwtzRiley8High* m_high2;

    double old_low;
    double old_mid;
    double old_high;

    CSAMPLE* m_pLowBuf;
    CSAMPLE* m_pBandBuf;
    CSAMPLE* m_pHighBuf;

    unsigned int m_oldSampleRate;
    int m_loFreq;
    int m_hiFreq;
};

class LinkwitzRiley8EQEffect : public GroupEffectProcessor<LinkwitzRiley8EQEffectGroupState> {
  public:
    LinkwitzRiley8EQEffect(EngineEffect* pEffect, const EffectManifest& manifest);
    virtual ~LinkwitzRiley8EQEffect();

    static QString getId();
    static EffectManifest getManifest();

    // See effectprocessor.h
    void processGroup(const QString& group,
                      LinkwitzRiley8EQEffectGroupState* pState,
                      const CSAMPLE* pInput, CSAMPLE *pOutput,
                      const unsigned int numSamples,
                      const unsigned int sampleRate,
                      const EffectProcessor::EnableState enableState,
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

    ControlObjectSlave* m_pLoFreqCorner;
    ControlObjectSlave* m_pHiFreqCorner;

    DISALLOW_COPY_AND_ASSIGN(LinkwitzRiley8EQEffect);
};

#endif /* LINKWITZRILEYEQEFFECT_H */
