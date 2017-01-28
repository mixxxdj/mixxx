// Ported from CAPS Reverb.
// This effect is GPL code.

#ifndef REVERBEFFECT_H
#define REVERBEFFECT_H

#include <QMap>

#include <Reverb.h>

#include "effects/effectprocessor.h"
#include "engine/effects/engineeffect.h"
#include "engine/effects/engineeffectparameter.h"
#include "util/class.h"
#include "util/defs.h"
#include "util/sample.h"
#include "util/types.h"

struct ReverbGroupState {
    float sampleRate  = 0;
    MixxxPlateX2 reverb{};
};

class ReverbEffect : public PerChannelEffectProcessor<ReverbGroupState> {
  public:
    ReverbEffect(EngineEffect* pEffect, const EffectManifest& manifest);
    virtual ~ReverbEffect();

    static QString getId();
    static EffectManifest getManifest();

    // See effectprocessor.h
    void processChannel(const ChannelHandle& handle,
                        ReverbGroupState* pState,
                        const CSAMPLE* pInput, CSAMPLE* pOutput,
                        const unsigned int numSamples,
                        const unsigned int sampleRate,
                        const EffectProcessor::EnableState enableState,
                        const GroupFeatureState& groupFeatures);

  private:
    QString debugString() const {
        return getId();
    }

    EngineEffectParameter* m_pDecayParameter;
    EngineEffectParameter* m_pBandWidthParameter;
    EngineEffectParameter* m_pDampingParameter;
    EngineEffectParameter* m_pSendParameter;

    DISALLOW_COPY_AND_ASSIGN(ReverbEffect);
};

#endif /* REVERBEFFECT_H */
