#ifndef PEAKINGFILTEREFFECT_H
#define PEAKINGFILTEREFFECT_H

#include <QMap>

#include "control/controlproxy.h"
#include "effects/effect.h"
#include "effects/effectprocessor.h"
#include "engine/effects/engineeffect.h"
#include "engine/effects/engineeffectparameter.h"
#include "engine/enginefilterbiquad1.h"
#include "util/types.h"
#include "util/defs.h"


class PeakingFilterEffectGroupState {
  public:
    PeakingFilterEffectGroupState();
    virtual ~PeakingFilterEffectGroupState();

    void setFilter(int sampleRate);

    EngineFilterBiquad1Peaking* m_filter;
    double m_oldQ;
    double m_oldGain;
    double m_oldCenter;
};

class PeakingFilterEffect
        : public PerChannelEffectProcessor<PeakingFilterEffectGroupState> {
  public:
    PeakingFilterEffect(EngineEffect* pEffect, const EffectManifest& manifest);
    virtual ~PeakingFilterEffect();

    static QString getId();
    static EffectManifest getManifest();

    // See effectprocessor.h
    void processChannel(
            const ChannelHandle& handle,
            PeakingFilterEffectGroupState* channelState,
            const CSAMPLE* pInput, CSAMPLE* pOutput,
            const unsigned int numSamples,
            const unsigned int sampleRate,
            const EffectProcessor::EnableState enableState,
            const GroupFeatureState& groupFeatures);

  private:
    QString debugString() const {
        return getId();
    }

    EngineEffectParameter* m_pPotQ;
    EngineEffectParameter* m_pPotGain;
    EngineEffectParameter* m_pPotCenter;
    unsigned int m_oldSampleRate;

    DISALLOW_COPY_AND_ASSIGN(PeakingFilterEffect);
};

#endif // PEAKINGFILTEREFFECT_H
