#ifndef CUSTOMDSPEFFECT_H
#define CUSTOMDSPEFFECT_H

#include <QMap>

#include "effects/effect.h"
#include "effects/effectprocessor.h"
#include "engine/effects/engineeffect.h"
#include "engine/effects/engineeffectparameter.h"
#include "util/class.h"
#include "util/types.h"

struct CustomDspGroupState {
    // Default accumulator to 1 so we immediately pick an input value.
    CustomDspGroupState()
            : hold_l(0),
              hold_r(0) {
    }
    CSAMPLE hold_l, hold_r;

};

class CustomDspEffect : public PerChannelEffectProcessor<CustomDspGroupState> {
  public:
    CustomDspEffect(EngineEffect* pEffect, const EffectManifest& manifest);
    virtual ~CustomDspEffect();

    static QString getId();
    static EffectManifest getManifest();

    // See effectprocessor.h
    void processChannel(const ChannelHandle& handle,
                        CustomDspGroupState* pState,
                        const CSAMPLE* pInput, CSAMPLE *pOutput,
                        const unsigned int numSamples,
                        const unsigned int sampleRate,
                        const EffectProcessor::EnableState enableState,
                        const GroupFeatureState& groupFeatureState);

  private:
    QString debugString() const {
        return getId();
    }

    EngineEffectParameter* m_pKnobA;
    EngineEffectParameter* m_pSwitchA;


    DISALLOW_COPY_AND_ASSIGN(CustomDspEffect);
};

#endif /* CUSTOMDSPEFFECT_H */
