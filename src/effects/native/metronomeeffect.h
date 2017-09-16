#ifndef METRONOMEEFFECT_H
#define METRONOMEEFFECT_H

#include <QMap>

#include "effects/effectprocessor.h"
#include "engine/effects/engineeffect.h"
#include "engine/effects/engineeffectparameter.h"
#include "engine/enginefilterpansingle.h"
#include "util/class.h"
#include "util/defs.h"
#include "util/sample.h"
#include "util/types.h"


class MetronomeGroupState final {
  public:
    MetronomeGroupState()
      : m_framesSinceClickStart(0) {
    }
    ~MetronomeGroupState() {
    }
    unsigned int m_framesSinceClickStart;
};

class MetronomeEffect : public PerChannelEffectProcessor<MetronomeGroupState> {
  public:
    MetronomeEffect(EngineEffect* pEffect, const EffectManifest& manifest);
    virtual ~MetronomeEffect();

    static QString getId();
    static EffectManifest getManifest();

    // See effectprocessor.h
    void processChannel(const ChannelHandle& handle,
                        MetronomeGroupState* pState,
                        const CSAMPLE* pInput, CSAMPLE* pOutput,
                        const unsigned int numSamples,
                        const unsigned int sampleRate,
                        const EffectProcessor::EnableState enableState,
                        const GroupFeatureState& groupFeatures);
  private:
    EngineEffectParameter* m_pBpmParameter;
    EngineEffectParameter* m_pSyncParameter;

    DISALLOW_COPY_AND_ASSIGN(MetronomeEffect);
};

#endif /* METRONOMEEFFECT_H */
